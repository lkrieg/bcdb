#include "common.h"
#include "socket.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

static int numclients;
static net_cln_t clients[MAX_CLIENTS];
// static int sockets[MAX_CLIENTS];
static net_func_t handler;
static fd_set masterset;
static int maxfd;

static int telport, telsock = -1;
static int webport, websock = -1;
static char teladdr[MAX_IPADDR];
static char webaddr[MAX_IPADDR];

enum socktype
{
	T_NET_TEL,
	T_NET_WEB
};

static int Bind(int type, int port);
static int Listen(int type);
static int AddClient(int socket, int type);
static int GetClient(int socket, net_cln_t **out);
static void DeleteClient(int socket);
static int SetOpt(int fd, int opt, int val);
static const char *GetAddrStr(struct sockaddr *addr);

int NET_Init(int tel, int web)
{
	Info("Initializing networking module...");

	FD_ZERO(&masterset);
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

void NET_SetHandler(net_func_t func)
{
	handler = func;
}

int NET_Update(void)
{
	byte buf[MAX_LINEBUF];
	static fd_set set;
	net_evt_t evt;
	int fd, n;
	int type;

	Assert(telsock >= 0);
	Assert(websock >= 0);
	Assert(handler != NULL);

	set = masterset;
	if (select(maxfd + 1, &set, NULL, NULL, NULL) < 0) {
		Warning(E_SELECT ": %s", strerror(errno));
		return -1;
	}

	for (fd = 0; fd <= maxfd; fd++) {
		if (FD_ISSET(fd, &set)) {
			memset(&evt, 0, sizeof(evt));
			if (fd == telsock || fd == websock) {
				type = (fd == telsock) ? T_CLN_TEL : T_CLN_WEB;

				// Accept new client
				if ((AddClient(fd, type) < 0)
				|| ((GetClient(fd, &evt.client) < 0))) {
					Warning(E_ADDCLN);
					continue;
				}

				evt.type = T_EVT_CONNECTED;
				handler(&evt);

			} else {
				// Receive data from client
				n = recv(fd, buf, sizeof(buf), 0);
				if (n > 0) {
					if (GetClient(fd, &evt.client) < 0) {
						Warning(E_GETCLN);
						continue;
					}

					evt.type = T_EVT_RECEIVED;
					evt.length = n;
					evt.data = buf;
					handler(&evt);

				// Closed
				} else {
					if (n < 0)
						Warning(E_RXDATA);

					DeleteClient(fd);
					evt.type = T_EVT_CLOSED;
					handler(&evt);
				}
			}
		}
	}

	return 0;
}

void NET_Shutdown(void)
{
	Info("Shutting down networking module...");
}

static int Bind(int type, int port)
{
	char portstr[16];
	struct addrinfo hints;
	struct addrinfo *addr, *it;
	const char *addrstr;
	int rc, fd;

	memset(&hints, 0, sizeof(hints));

	Assert(port >= 0 && port < 65536);
	Assert(type == T_NET_TEL || type == T_NET_WEB);

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

		// The type argument is a kind of convoluted hack
		// for giving better debug output in Listen()
		// Not sure if this is worth it...

		snprintf((type == T_NET_TEL) ? teladdr : webaddr,
		         MAX_IPADDR, "%s:%d", addrstr, port);

		if (type == T_NET_TEL) {
			telsock = fd;
			telport = port;
		} else {
			websock = fd;
			webport = port;
		}

		if (fd > maxfd)
			maxfd = fd;

		break;
	}

	freeaddrinfo(addr);
	if (it == NULL)
		return -1;

	return fd;
}

static int Listen(int type)
{
	int fd;
	const char *addr;
	const char *name;

	Assert(telsock >= 0 && websock >= 0);
	Assert(type == T_NET_TEL || type == T_NET_WEB);

	fd    = (type == T_NET_TEL) ? telsock : websock;
	addr  = (type == T_NET_TEL) ? teladdr : webaddr;
	name  = (type == T_NET_TEL) ? "telnet" : "http";

	Info("Listening for %s from %s...", name, addr);
	if (listen(fd, MAX_BACKLOG) < 0) {
		Warning(E_LISTEN ": %s", strerror(errno));
		return -1;
	}

	FD_SET(fd, &masterset);
	return 0;
}

static int AddClient(int socket, int type)
{
	struct sockaddr_storage remote;
	struct sockaddr *addr;
	socklen_t socklen;
	net_cln_t *cln;
	int fd;

	if (numclients > MAX_CLIENTS) {
		Warning(E_MAXCLN);
		return -1;
	}

	socklen  = sizeof(remote);
	addr     = (struct sockaddr *) &remote;

	if ((fd = accept(socket, addr, &socklen)) < 0) {
		Warning(E_ACCEPT ": %s", strerror(errno));
		return -1;
	}

	// TODO: Add new client to client list
	// TODO: Constant time find by socket descriptor

	UNUSED(clients);
	UNUSED(type);
	UNUSED(cln);

	FD_SET(fd, &masterset);
	if (fd > maxfd)
		maxfd = fd;

	return 0;
}

static int GetClient(int socket, net_cln_t **out)
{
	// TODO: Get client struct from socket descriptor

	Warning("Not implemented");
	UNUSED(socket);
	UNUSED(out);
	return -1;
}

static void DeleteClient(int socket)
{
	// TODO: Add client struct to free list
	// TODO: Perhaps update maxfd if necessary?

	FD_CLR(socket, &masterset);
	close(socket);
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
