#include "common.h"
#include "socket.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

static int Bind(int port);
static int Listen(int fd);
static int SetOpt(int fd, int opt, int val);
static const char *GetAddrStr(struct sockaddr *addr);

int NET_Init(int tel, int web)
{
	int telsock;
	int websock;

	telsock = Bind(tel);
	websock = Bind(web);

	if (telsock < 0 || websock < 0) {
		if (telsock >= 0)
			close(telsock);
		if (websock >= 0)
			close(websock);

		return -1;
	}

	if ((Listen(telsock) < 0)
	|| ((Listen(websock) < 0))) {
		close(telsock);
		close(websock);
		return -2;
	}

	return 0;
}

void NET_Shutdown(void)
{
	Info("Shutting down networking...");
}

static int Bind(int port)
{
	char portstr[16];
	struct addrinfo hints;
	struct addrinfo *addr, *it;
	const char *addrstr;
	int rc, fd;

	memset(&hints, 0, sizeof(hints));

	hints.ai_family    = AF_UNSPEC;    // IPv4, IPv6
	hints.ai_socktype  = SOCK_STREAM;  // TCP stream
	hints.ai_flags     = AI_PASSIVE;   // Bindable

	snprintf(portstr, 16, "%d", port);
	if ((rc = getaddrinfo(NULL, portstr, &hints, &addr)) != 0)
		Warning(E_GETADR ": %s", gai_strerror(rc));

	for (it = addr; it; it = it->ai_next) {
		addrstr = GetAddrStr(it->ai_addr);
		Info("Getting socket descriptor %s:%d...", addrstr, port);
		fd = socket(it->ai_family, it->ai_socktype,
		            it->ai_protocol);

		if (fd < 0) {
			Warning(E_NOSOCK " %s:%d", addrstr, port);
			continue;
		}

		Verbose("Enabling socket option SO_REUSEADDR...");
		if (SetOpt(fd, SO_REUSEADDR, 1) < 0)
			Warning(E_SETOPT, ": %s", strerror(errno));

		Verbose("Binding to socket descriptor...");
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

	return fd;
}

static int Listen(int fd)
{
	Info("Listening on socket...");
	if (listen(fd, MAX_BACKLOG) < 0) {
		Warning(E_LISTEN ": %s", strerror(errno));
		return -1;
	}

	return 0;
}

static int SetOpt(int fd, int opt, int val)
{
	return setsockopt(fd, SOL_SOCKET, opt, &val, sizeof(val));
}

static const char *GetAddrStr(struct sockaddr *addr)
{
	static char buf[MAX_IPADDR];
	const void *src;

	Assert(addr != NULL);

	src = (addr->sa_family == AF_INET) // Support both IPv4 and IPv6
	? (void*) &(((struct sockaddr_in  *) addr)->sin_addr)
	: (void*) &(((struct sockaddr_in6 *) addr)->sin6_addr);

	// Writing to static buffer, therefore not thread-safe!
	if (inet_ntop(addr->sa_family, src, buf, sizeof(buf)) == NULL)
		return "INVALID";

	return buf;
}
