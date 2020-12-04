#include "common/common.h"
#include "common/external/zlib.h"
#include "common/external/telnet.h"

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

static int sockfd = -1;

static const telnet_telopt_t telopts[] = {
	{ TELNET_TELOPT_COMPRESS2, TELNET_WILL, TELNET_DONT },
	{ TELNET_TELOPT_NAWS,      TELNET_WONT, TELNET_DO   },
	{ -1, 0, 0 }
};

static const char *GetAddrStr(struct sockaddr *addr);

int NET_Init(void)
{
	struct addrinfo hints;
	struct addrinfo *addr, *it;
	int rc, fd, opt = 1;

	memset(&hints, 0, sizeof(hints));

	hints.ai_family   = AF_UNSPEC;   // IPv4, IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream
	hints.ai_flags    = AI_PASSIVE;  // Bindable

	if ((rc = getaddrinfo(NULL, "telnet", &hints, &addr)) != 0)
		Error(E_GETADR ": %s", gai_strerror(rc));

	for (it = addr; it; it = it->ai_next) {
		fd = socket(it->ai_family, it->ai_socktype,
		            it->ai_protocol);

		if (fd < 0) { // Keep trying each bindable address
			Warning(E_NOSOCK ": %s", strerror(errno));
			continue;
		}

		// Allow immediate address reuse after closing the socket descriptor
		if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
			Warning(E_SETOPT " SO_REUSEADDR: %s", strerror(errno));
			continue;
		}

		if (bind(fd, it->ai_addr, it->ai_addrlen) < 0) {
			Warning(E_NOBIND ": %s", strerror(errno));
			close(fd);
			continue;
		}

		break;
	}

	freeaddrinfo(addr);
	if (it == NULL)
		return -1;

	if (listen(fd, MAX_BACKLOG) < 0)
		Error(E_LISTEN);

	sockfd = fd;

	return 0;
}

static void HandleEvent(telnet_t *telnet, telnet_event_t *evt, void *client)
{
	net_cln_t *cln = (net_cln_t *) client; // FIXME
	unsigned int i;

	UNUSED(telnet);
	switch(evt->type) {
	case TELNET_EV_DATA:
		Print("Received data from %s:", cln->address);
		for (i = 0; i < evt->data.size; i++)
			Print(" %02X", evt->data.buffer[i]);
		Print("\n");
		break;
	case TELNET_EV_SEND:
		NET_Send(cln, evt->data.buffer, evt->data.size);
		break;
	case TELNET_EV_DO:
		if (evt->neg.telopt == TELNET_TELOPT_COMPRESS2) {
			Info("Starting compression");
			telnet_begin_compress2(telnet);
		}
		break;
	case TELNET_EV_ERROR:
	default:
		break;

	}
}

int NET_Accept(net_cln_t *out)
{
	socklen_t solen;
	struct sockaddr *sa;
	struct sockaddr_storage addr;
	const char *adstr;
	int fd;

	solen  = sizeof(addr);
	sa     = (struct sockaddr *) &addr;
	fd     = accept(sockfd, sa, &solen);

	if (fd < 0) {
		Warning(E_ACCEPT ": %s", strerror(errno));
		return -1;
	}

	adstr = GetAddrStr(sa);
	Info(M_ACCEPT ": %s", adstr);
	strcpy(out->address, adstr);

	out->handle = fd;
	out->parent = sockfd;
	out->telnet = telnet_init(telopts, HandleEvent, 0, &out); // FIXME
	telnet_negotiate(out->telnet, TELNET_WILL, TELNET_TELOPT_COMPRESS2);
	telnet_negotiate(out->telnet, TELNET_WILL, TELNET_TELOPT_ECHO);

	return out->handle;
}

void NET_Close(net_cln_t *cln)
{
	telnet_free(cln->telnet);
	close(cln->handle);
}

int NET_NextEvent(net_cln_t *cln, net_evt_t *out)
{
	char buf[MAX_MSG_LEN];
	int n;

	// TODO: Return enqueued events before receiving more
        if ((n = recv(cln->handle, buf, MAX_MSG_LEN, 0)) > 0)
		telnet_recv(cln->telnet, buf, n);

	if (n < 0 && errno != EINTR)
		Error(E_RXDATA " from '%s'", cln->address);

	out->type = T_EVT_NONE;
	return out->type;
}

void NET_Send(net_cln_t *cln, const char *buf, int size)
{
	int n;

	if (cln->handle < 0)
		return;

	while (size > 0) {
		if ((n = send(cln->handle, buf, size, 0)) <= 0) {
			if (errno != EINTR && errno != ECONNRESET)
				Error(E_TXDATA ": %s", strerror(errno));
			return;
		}

		buf  += n;
		size -= n;
	}
}

void NET_Shutdown(void)
{
	if (sockfd < 0)
		return;

	close(sockfd);
}

static const char *GetAddrStr(struct sockaddr *addr)
{       
	static char buf[MAX_ADR_LEN];
	const void *src;
        
	src = (addr->sa_family == AF_INET)
	    ? (void*) &(((struct sockaddr_in  *) addr)->sin_addr)
	    : (void*) &(((struct sockaddr_in6 *) addr)->sin6_addr);
        
	// Writing to static buffer, therefore not thread-safe!
	if (inet_ntop(addr->sa_family, src, buf, sizeof(buf)) == NULL)
		return "INVALID";
        
	return buf;
}
