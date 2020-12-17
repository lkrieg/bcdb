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

static int telport, telsock = -1;
static int webport, websock = -1;
static char teladdr[MAX_IPADDR];
static char webaddr[MAX_IPADDR];

enum handle
{
	T_NET_TEL,
	T_NET_WEB
};

static int Bind(int handle, int port);
static int Listen(int handle);
static int SetOpt(int fd, int opt, int val);
static const char *GetAddrStr(struct sockaddr *addr);

int NET_Init(int tel, int web)
{
	Info("Initializing networking module...");

	if (((Bind(T_NET_TEL, tel)) < 0)
	|| (((Bind(T_NET_WEB, web)) < 0))) {
		if (telsock >= 0)
			close(telsock);
		if (websock >= 0)
			close(websock);
		return -1;
	}

	if ((Listen(T_NET_TEL) < 0)
	|| ((Listen(T_NET_WEB) < 0))) {
		close(telsock);
		close(websock);
		return -2;
	}

	return 0;
}

int NET_Accept(net_cln_t *out)
{
	UNUSED(out);
	return 0;
}

void NET_Shutdown(void)
{
	Info("Shutting down networking module...");
}

static int Bind(int handle, int port)
{
	char portstr[16];
	struct addrinfo hints;
	struct addrinfo *addr, *it;
	const char *addrstr;
	int rc, fd;

	memset(&hints, 0, sizeof(hints));

	Assert(port >= 0 && port < 65536);
	Assert(handle == T_NET_TEL || handle == T_NET_WEB);

	hints.ai_family    = AF_UNSPEC;    // IPv4, IPv6
	hints.ai_socktype  = SOCK_STREAM;  // TCP stream
	hints.ai_flags     = AI_PASSIVE;   // Bindable

	snprintf(portstr, 16, "%d", port);
	if ((rc = getaddrinfo(NULL, portstr, &hints, &addr)) != 0)
		Warning(E_GETADR ": %s", gai_strerror(rc));

	for (it = addr; it; it = it->ai_next) {
		addrstr = GetAddrStr(it->ai_addr);
		Verbose("Requesting socket descriptor %s:%d...", addrstr, port);
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

		if (handle == T_NET_TEL) {
			telsock = fd;
			telport = port;
			snprintf(teladdr, MAX_IPADDR,
			         "%s:%d", addrstr, port);
		} else {
			websock = fd;
			webport = port;
			snprintf(webaddr, MAX_IPADDR,
			         "%s:%d", addrstr, port);
		}

		break;
	}

	freeaddrinfo(addr);
	if (it == NULL)
		return -1;

	return fd;
}

static int Listen(int handle)
{
	int fd;
	char *addr;
	const char *type;

	Assert(telsock >= 0 && websock >= 0);
	Assert(handle == T_NET_TEL || handle == T_NET_WEB);

	fd    = (handle == T_NET_TEL) ? telsock : websock;
	addr  = (handle == T_NET_TEL) ? teladdr : webaddr;
	type  = (handle == T_NET_TEL) ? "telnet" : "http";

	Info("Listening for %s from %s...", type, addr);
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
