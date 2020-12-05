#include "common/common.h"
#include "common/external/zlib.h"
#include "common/external/telnet.h"

#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

static const char *GetOptStr(int opt);
static const char *GetAddrStr(struct sockaddr *addr);
void SendClient(net_cln_t *cln, const char *buf, int size);

static int sockfd = -1;
static const telnet_telopt_t telopts[] = {
	{ TELNET_TELOPT_TTYPE,     TELNET_WONT, TELNET_DO   },
	{ TELNET_TELOPT_NAWS,      TELNET_WONT, TELNET_DO   },
	{ TELNET_TELOPT_LINEMODE,  TELNET_WONT, TELNET_DO   },
	{ TELNET_TELOPT_COMPRESS2, TELNET_WILL, TELNET_DONT },
	{ TELNET_TELOPT_ECHO,      TELNET_WILL, TELNET_DONT },
	{ -1, 0, 0 }
};

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
	net_cln_t *cln;
	short rows, cols;
	const char *ptr;
	char buf[64];

	UNUSED(GetOptStr);
	cln = (net_cln_t *) client;

	switch(evt->type) {
	case TELNET_EV_DATA:
		break;
	case TELNET_EV_IAC:
		break;
	case TELNET_EV_SEND:
		SendClient(cln, evt->data.buffer, evt->data.size);
		break;
	case TELNET_EV_DO:
		if (evt->neg.telopt == TELNET_TELOPT_COMPRESS2) {
			Info("DOING COMPRESSION");
			telnet_begin_compress2(telnet);
		}
		break;
	case TELNET_EV_WILL:
		if (evt->neg.telopt == TELNET_TELOPT_TTYPE)
			telnet_ttype_send(telnet);
		if (evt->neg.telopt == TELNET_TELOPT_LINEMODE) {
			buf[0] = TELNET_LINEMODE_MODE;
			buf[1] = TELNET_LINEMODE_TRAPSIG;
			telnet_begin_sb(telnet, TELNET_TELOPT_LINEMODE);
			telnet_send(telnet, buf, 2);
			telnet_finish_sb(telnet);
		}
		break;
	case TELNET_EV_SUBNEGOTIATION:
		if ((evt->sub.telopt == TELNET_TELOPT_NAWS)
		&& ((evt->sub.size == 4))) {
			ptr = evt->sub.buffer;
			rows = (((short) ptr[3]) << 8) | ptr[2];                        
			cols = (((short) ptr[1]) << 8) | ptr[0];                        
			rows = ntohs(rows);                                        
			cols = ntohs(cols);   
			Info("Setting SIZE to %dx%d", rows, cols);
			resizeterm(rows, cols);
		}
		break;
	case TELNET_EV_TTYPE:
		Info("Setting TERM to %s", evt->data.buffer);
		//setterm(evt->data.buffer);
		break;
	case TELNET_EV_ERROR:
		break;
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
	FILE *fh;
	int fd;

	solen  = sizeof(addr);
	sa     = (struct sockaddr *) &addr;
	fd     = accept(sockfd, sa, &solen);

	if (fd < 0) {
		if (errno != EINTR)
			Warning(E_ACCEPT ": %s", strerror(errno));

		return -1;
	}

	adstr = GetAddrStr(sa);
	Info(M_ACCEPT ": %s", adstr);
	strcpy(out->address, adstr);

	out->handle   = fd;
	out->parent   = sockfd;
	out->rows     = MIN_ROW_NUM;
	out->cols     = MIN_COL_NUM;
	out->telnet   = telnet_init(telopts, HandleEvent, 0, out);

	telnet_negotiate(out->telnet, TELNET_DO,   TELNET_TELOPT_NAWS);
	telnet_negotiate(out->telnet, TELNET_DO,   TELNET_TELOPT_TTYPE);
	telnet_negotiate(out->telnet, TELNET_DO,   TELNET_TELOPT_LINEMODE);
	telnet_negotiate(out->telnet, TELNET_WILL, TELNET_TELOPT_COMPRESS2);
	telnet_negotiate(out->telnet, TELNET_WILL, TELNET_TELOPT_ECHO);

	// Hide client terminal cursor
	NET_Send(out, "\033[?12l", 6);
	NET_Send(out, "\033[?25l", 6);

	NET_Send(out, "\033[1;36m", 7);
	NET_Send(out, "\033[1;40m", 7);

	fh  = fdopen(out->handle, "rw");
//	out->screen = newterm("vt100", fh, fh);
	out->screen = newterm("xterm-color", fh, fh);
	set_term(out->screen);
	resizeterm(out->rows, out->cols);

	return out->handle;
}

void NET_Close(net_cln_t *cln)
{
	telnet_free(cln->telnet);
	close(cln->handle);
}

int NET_NextEvent(net_cln_t *cln)
{
	char buf[MAX_MSG_LEN];
	int n;

        if ((n = recv(cln->handle, buf, MAX_MSG_LEN, 0)) > 0)
		telnet_recv(cln->telnet, buf, n);

	if (n < 0 && errno != EINTR)
		Error(E_RXDATA " from '%s'", cln->address);

	return 1;
}

void NET_SetHandler(net_cln_t *cln, net_fun_t *func)
{
	cln->func = func;
}

void NET_Send(net_cln_t *cln, const char *buf, int size)
{
	telnet_send(cln->telnet, buf, size);
}

void NET_Shutdown(void)
{
	if (sockfd < 0)
		return;

	close(sockfd);
}

static const char *GetOptStr(int opt)
{
	switch (opt) {
	case 0: return "BINARY";
	case 1: return "ECHO";
	case 2: return "RCP";
	case 3: return "SGA";
	case 4: return "NAMS";
	case 5: return "STATUS";
	case 6: return "TM";
	case 7: return "RCTE";
	case 8: return "NAOL";
	case 9: return "NAOP";
	case 10: return "NAOCRD";
	case 11: return "NAOHTS";
	case 12: return "NAOHTD";
	case 13: return "NAOFFD";
	case 14: return "NAOVTS";
	case 15: return "NAOVTD";
	case 16: return "NAOLFD";
	case 17: return "XASCII";
	case 18: return "LOGOUT";
	case 19: return "BM";
	case 20: return "DET";
	case 21: return "SUPDUP";
	case 22: return "SUPDUPOUTPUT";
	case 23: return "SNDLOC";
	case 24: return "TTYPE";
	case 25: return "EOR";
	case 26: return "TUID";
	case 27: return "OUTMRK";
	case 28: return "TTYLOC";
	case 29: return "3270REGIME";
	case 30: return "X3PAD";
	case 31: return "NAWS";
	case 32: return "TSPEED";
	case 33: return "LFLOW";
	case 34: return "LINEMODE";
	case 35: return "XDISPLOC";
	case 36: return "ENVIRON";
	case 37: return "AUTHENTICATION";
	case 38: return "ENCRYPT";
	case 39: return "NEW-ENVIRON";
	case 70: return "MSSP";
	case 85: return "COMPRESS";
	case 86: return "COMPRESS2";
	case 93: return "ZMP";
	case 255: return "EXOPL";
	default: return "UNKNOWN";
	}
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

void SendClient(net_cln_t *cln, const char *buf, int size)
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
