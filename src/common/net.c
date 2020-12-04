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

	out->handle = fd;
	out->parent = sockfd;
	adstr = GetAddrStr(sa);
	strcpy(out->address, adstr);
	Info(M_ACCEPT ": %s", adstr);

	return out->handle;
}

int NET_Read(net_cln_t *cln, byte *out)
{
	int n;

	// TODO: Now test libtelnet with zlib compression here
        if ((n = recv(cln->handle, out, MAX_MSG_LEN, 0)) < 0)
                Warning(E_RXDATA " from '%s'", cln->address);

	return n;
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
