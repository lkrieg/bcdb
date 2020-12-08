#include "common.h"
#include "net.h"

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

static int sockfd;

static void Parse(net_cln_t *cln, const byte *data, int size);
static int PushEvent(net_cln_t *cln, net_evt_t *evt);
static int PopEvent(net_cln_t *cln, net_evt_t *out);
static int SetSockOpt(int fd, int opt, int val);
static const char *GetAddrStr(struct sockaddr *addr);
static int SendTelOpt(const net_cln_t *cln, int opt);
static const char *GetOptStr(byte opt);

int NET_Init(void)
{
	struct addrinfo hints;
	struct addrinfo *addr, *it;
	int rc, fd;

	Info(M_INITIP);
	memset(&hints, 0, sizeof(hints));

	hints.ai_family    = AF_UNSPEC;    // IPv4, IPv6
	hints.ai_socktype  = SOCK_STREAM;  // TCP stream socket
	hints.ai_flags     = AI_PASSIVE;   // Bindable address

	if ((rc = getaddrinfo(NULL, "telnet", &hints, &addr)) != 0) {
		Warning(E_GETADR ": %s", gai_strerror(rc));
		return -1;
	}

	for (it = addr; it; it = it->ai_next) {
		Info(M_SOCKFD " for '%s'", GetAddrStr(it->ai_addr));
		fd = socket(it->ai_family, it->ai_socktype,
		            it->ai_protocol);

		if (fd < 0) {
			Warning(E_NOSOCK ": %s", strerror(errno));
			continue;
		}

		Info(M_SETOPT " 'REUSEADDR'");
		if (SetSockOpt(fd, SO_REUSEADDR, 1) < 0)
			Warning(E_SETOPT ": %s", strerror(errno));

		Info(M_BINDFD);
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

	Info(M_LISTEN);
	if (listen(fd, MAX_BACKLOG) < 0) {
		Warning(E_LISTEN);
		return -1;
	}

	sockfd = fd;

	return 0;
}

int NET_Accept(net_cln_t *out)
{
	socklen_t len;
	struct sockaddr *sa;
	struct sockaddr_storage addr;
	const char *name;
	int fd;

	AS_NEQ_NULL(out);
	AS_GEQ_ZERO(sockfd);

	len   = sizeof(addr);
	sa    = (struct sockaddr *) &addr;
	fd    = accept(sockfd, sa, &len);
	name  = GetAddrStr(sa);

	if (fd < 0)
		return -1;

	Info(M_ACCEPT " '%s'", name);
	strcpy(out->name, name);

	out->handle   = fd;
	out->parent   = sockfd;
	out->queue    = NULL;
	// out->rows     = MIN_ROW_NUM;
	// out->cols     = MIN_COL_NUM;
	// out->type[0]  = '\0';

	return fd;
}

void NET_Negotiate(net_cln_t *cln, int flags)
{
	SendTelOpt(cln, flags);
}

int NET_NextEvent(net_cln_t *cln, net_evt_t *out)
{
	int n;
	byte data[MAX_MSG_LEN];

	if (PopEvent(cln, out))
		return out->type;

	if ((n = recv(cln->handle, data, MAX_MSG_LEN - 1, 0)) < 0)
		Error(E_RXDATA " from '%s'", cln->name);

	if (n > 0) {
		Parse(cln, data, n);
		if (PopEvent(cln, out))
			return out->type;
	}

	// Non-zero value!
	return T_EVT_NONE;
}

void NET_Shutdown(void)
{
	if (sockfd >= 0)
		close(sockfd);
}

static void Parse(net_cln_t *cln, const byte *data, int size)
{
	static int value, n;
	static int state = T_STATE_NONE;
	static byte buf[MAX_MSG_LEN];
	const byte *head, *tail;
	net_evt_t evt;

	head = data;
	tail = data + size;

	for (; head < tail; head++) {
		switch (state) {
		case T_STATE_NONE:
			if (*head == T_CMD_IAC) {
				state = T_STATE_COMMAND;
				break;
			}

			switch (*head) {
			case 0x03: // CTRL-C
				evt.type = T_EVT_QUIT;
				PushEvent(cln, &evt);
				break;
			}
			break;

		case T_STATE_COMMAND:
			switch (*head) { // Negotiation
			case T_CMD_WONT: case T_CMD_WILL:
			case T_CMD_DONT: case T_CMD_DO:
				state = T_STATE_NEGOTIATE;
				value = *head;
				break;
			case T_CMD_SB:
				state = T_STATE_SUB;
				break;
			}
			break;

		case T_STATE_NEGOTIATE:
			state = T_STATE_NONE;

			// Print debug information to improve device support
			Info("Received negotiation %s %s", GetOptStr(*head),
			     (value == T_CMD_WONT) ? "WONT" :
			     (value == T_CMD_WILL) ? "WILL" :
			     (value == T_CMD_DONT) ? "DONT" :
			     (value == T_CMD_DO)   ? "DO"   : "");

			switch (*head) {
			case T_OPT_NAWS:
				if (value == T_CMD_WILL)
					SendTelOpt(cln, T_DO | T_OPT_NAWS);
				break;
			case T_OPT_TTYPE:
				if (value == T_CMD_WILL)
					SendTelOpt(cln, T_DO | T_OPT_TTYPE);
				break;
			case T_OPT_LINEMODE:
				if (value == T_CMD_WILL)
					SendTelOpt(cln, T_DONT | T_OPT_LINEMODE);
				break;
			}
			break;

		case T_STATE_SUB:
			if (*head == T_OPT_NAWS)
				state = T_STATE_NAWS;
			if (*head == T_OPT_TTYPE)
				state = T_STATE_TTYPE;
			if (*head == T_CMD_SE)
				state = T_STATE_NONE;
			n = 0;
			break;

		case T_STATE_NAWS:
			if (*head != T_CMD_SE && n < 5) {
				buf[n++] = *head;
				break;
			}

			if (*head != T_CMD_SE || n != 5)
				Error("INVALID NAWS");

			evt.type = T_EVT_RESIZE;
			evt.rows = (((short) buf[3]) << 8) | buf[2];
			evt.cols = (((short) buf[1]) << 8) | buf[0];
			evt.rows = ntohs(evt.rows);
			evt.cols = ntohs(evt.cols);

			PushEvent(cln, &evt);
			state = T_STATE_NONE;
			break;
		case T_STATE_TTYPE:
			// TODO: Read terminal type
			if (*head == T_CMD_SE) {
				state = T_STATE_NONE;
				evt.type = T_EVT_TTYPE;
				PushEvent(cln, &evt);
				break;
			}
			break;
		}
	}
}

static int PushEvent(net_cln_t *cln, net_evt_t *evt)
{
	// TODO: Enqueue events
	if (evt->type == T_EVT_RESIZE)
		Info("Window size changed to %dx%d",
			evt->rows, evt->cols);
	if (evt->type == T_EVT_QUIT) {
		Info("User interrupt received");
		close(cln->handle);
		exit(EXIT_SUCCESS);
	}

	return 0;
}

static int PopEvent(net_cln_t *cln, net_evt_t *out)
{
	// TODO: Dequeue events
	UNUSED(cln);
	UNUSED(out);
	return 0;
}

static int SetSockOpt(int fd, int opt, int val)
{
	AS_GEQ_ZERO(fd);
	AS_GEQ_ZERO(opt);
	AS_GEQ_ZERO(val);

	return setsockopt(fd, SOL_SOCKET, opt, &val, sizeof(val));
}

static const char *GetAddrStr(struct sockaddr *addr)
{
	static char buf[MAX_ADR_LEN];
	const void *src;

	AS_NEQ_NULL(addr);

	src = (addr->sa_family == AF_INET) // Support both IPv4 and IPv6
	    ? (void*) &(((struct sockaddr_in  *) addr)->sin_addr)
	    : (void*) &(((struct sockaddr_in6 *) addr)->sin6_addr);

	// Writing to static buffer, therefore not thread-safe!
	if (inet_ntop(addr->sa_family, src, buf, sizeof(buf)) == NULL)
		return "INVALID";

	return buf;
}

static int SendTelOpt(const net_cln_t *cln, int opt)
{
	int n = 0;
	byte cmd[12];

	if (opt & T_WONT) {
		cmd[n++] = T_CMD_IAC;
		cmd[n++] = T_CMD_WONT;
		cmd[n++] = opt & 0xFF;
	}

	if (opt & T_WILL) {
		cmd[n++] = T_CMD_IAC;
		cmd[n++] = T_CMD_WILL;
		cmd[n++] = opt & 0xFF;
	}

	if (opt & T_DONT) {
		cmd[n++] = T_CMD_IAC;
		cmd[n++] = T_CMD_DONT;
		cmd[n++] = opt & 0xFF;
	}

	if (opt & T_DO) {
		cmd[n++] = T_CMD_IAC;
		cmd[n++] = T_CMD_DO;
		cmd[n++] = opt & 0xFF;
	}

	Info("Negotiating %s %s%s%s%s",
	     GetOptStr(opt & 0xFF),
	     (opt & T_WONT) ? "WONT " : "",
	     (opt & T_WILL) ? "WILL " : "",
	     (opt & T_DONT) ? "DONT " : "",
	     (opt & T_DO)   ? "DO "   : "");

	return send(cln->handle, cmd, n, 0);
}

static const char *GetOptStr(byte opt)
{
	switch (opt) {
	#define OPT(n, s) \
	case n: return #s;
	TEL_OPT_LIST
	#undef OPT
	}

	return "UNKNOWN";
}

#if 0
int NET_NextEvent(net_cln_t *cln, net_evt_t *out)
{
	static int state = T_STATE_NONE;
	byte raw[MAX_MSG_LEN];
	byte msg[MAX_MSG_LEN];
	byte *head, *tail;
	int type, n, j = 0;

	AS_NEQ_NULL(cln);
	AS_NEQ_NULL(out);

	// TODO: Need some kind of queue or at least a plan when
	// the message buffer can be discarded. Like when the user
	// presses the left and right to switch the current mode.

	if ((n = recv(cln->handle, raw, MAX_MSG_LEN - 1, 0)) < 0)
		Error(E_RXDATA " from '%s'", cln->name);

	head = raw;
	tail = raw + n;
	type = T_EVT_NONE;

	// Parse telnet commands
	for (; head < tail; head++) {
		switch (state) {
		case T_STATE_NONE:
			if (*head == T_CMD_IAC) {
				state = T_STATE_IAC;
				break;
			}

			// Special key events
			switch (*head) {
			case 0x03: // CTRL-C
				out->type = T_EVT_QUIT;
				return out->type;

			case 0x1B:
				if ((tail - head) < 2)
					break;

				if ((head[1] == 0x5B)
				&& ((head[2] == 0x43))) {
					out->type = T_EVT_KEYDOWN;
					out->keycode = T_KEY_RIGHT;
					return out->type;
				}

				if ((head[1] == 0x5B)
				&& ((head[2] == 0x44))) {
					out->type = T_EVT_KEYDOWN;
					out->keycode = T_KEY_LEFT;
					return out->type;
				}
			}

			// Plain old data
			msg[j++] = *head;
			break;

		case T_STATE_IAC:
			switch (*head) {
			case T_CMD_IAC: // Escaped IAC
				state = T_STATE_NONE;
				msg[j++] = 0xFF;
				break;
			case T_CMD_SB:  // Subnegotiation
				state = T_STATE_SB;
				break;
			case T_CMD_WONT:
				state = T_STATE_WONT;
				break;
			case T_CMD_WILL:
				state = T_STATE_WILL;
				break;
			case T_CMD_DONT:
				state = T_STATE_DONT;
				break;
			case T_CMD_DO:
				state = T_STATE_DO;
				break;
			}
			break;

		case T_STATE_WONT:
		case T_STATE_WILL:
		case T_STATE_DONT:
		case T_STATE_DO:
			// TODO: Negotiation table
			Info("Received negotiation %s %s", GetOptStr(*head),
			     (state == T_STATE_WONT) ? "WONT" :
			     (state == T_STATE_WILL) ? "WILL" :
			     (state == T_STATE_DONT) ? "DONT" :
			     (state == T_STATE_DO)   ? "DO"   : "");

			if ((*head == T_OPT_TTYPE)
			&& ((state == T_STATE_WILL)))
				SendTelOpt(cln, T_DO | T_OPT_TTYPE);

			if ((*head == T_OPT_NAWS)
			&& ((state == T_STATE_WILL)))
				SendTelOpt(cln, T_DO | T_OPT_NAWS);

			if ((*head == T_OPT_LINEMODE)
			&& ((state == T_STATE_WILL)))
				SendTelOpt(cln, T_DONT | T_OPT_LINEMODE);

			state = T_STATE_NONE;
			break;

		case T_STATE_SB:
			if (*head == T_OPT_NAWS)
				out->type = T_EVT_RESIZE;
			// else if (*head == T_OPT_TTYPE)
			//	out->type = T_EVT_TTYPE;
			if (*head == T_CMD_SE) {
				state = T_STATE_NONE;
				return out->type;
			}
			break;
		}
	}

	// Waiting for more data
	if (state != T_STATE_NONE)
		type = T_EVT_NONE;

	// Passthrough
	else if (j > 0) {
		msg[j] = '\0';
		type = T_EVT_DATA;
		memcpy(out->data, msg, j + 1);
	}

	out->size = j;
	out->type = type;
	return out->type;
}
#endif



#if 0
void NET_Write(const net_cln_t *cln, const byte *data, int size)
{
	AS_NEQ_NULL(cln);
	AS_NEQ_NULL(data);
	AS_GEQ_ZERO(size);

	if (send(cln->handle, data, size, 0) < 0)
		Error(E_TXDATA " to %s", cln->name);
}

static void Negotiate(net_cln_t *cln, int state, int type)
{
	byte cmd[32];

	// TODO: https://tools.ietf.org/html/rfc1143#section-7
	// Everything below is just for testing at the moment
	// Does not protect against endless negotiation loops

	AS_NEQ_NULL(cln);

	Info(M_SETTLE " '%s_%s'",
	     (state == T_STATE_WONT) ? "WONT"
	   : (state == T_STATE_WILL) ? "WILL"
	   : (state == T_STATE_DONT) ? "DONT"
	   : (state == T_STATE_DO)   ? "DO"
	   : "UNKNOWN", GetOptStr(type));

	// Client wants to send terminal type
	if ((state == T_STATE_WILL)
	&& ((type  == T_OPT_TTYPE))) {
		Info(M_ANSWER" with 'DO_TTYPE'");
		// Directly send subnegotiation
		cmd[0] = T_CMD_IAC;
		cmd[1] = T_CMD_DO;
		cmd[2] = T_OPT_TTYPE;
		cmd[3] = T_CMD_IAC;
		cmd[4] = T_CMD_SB;
		cmd[5] = T_OPT_TTYPE;
		cmd[6] = T_OPT_SEND;
		cmd[7] = T_CMD_IAC;
		cmd[8] = T_CMD_SE;
		NET_Write(cln, cmd, 9);
	}

	// Client wants to negotiate window size
	if ((state == T_STATE_WILL)
	&& ((type  == T_OPT_NAWS))) {
		Info(M_ANSWER" with 'DO_NAWS'");
		cmd[0] = T_CMD_IAC;
		cmd[1] = T_CMD_DO;
		cmd[2] = T_OPT_NAWS;
		NET_Write(cln, cmd, 3);
	}

	// Client wants us to suppress go ahead
	if ((state == T_STATE_DO)
	&& ((type  == T_OPT_SGA))) {
		Info(M_ANSWER" with 'WILL_SGA'");
		cmd[0] = T_CMD_IAC;
		cmd[1] = T_CMD_WILL;
		cmd[2] = T_OPT_SGA;
		NET_Write(cln, cmd, 3);
	}

	// Client wants to use linemode
	if ((state == T_STATE_WILL)
	&& ((type  == T_OPT_LINEMODE))) {
		Info(M_ANSWER" with 'DONT_LINEMODE'");
		cmd[0] = T_CMD_IAC;
		cmd[1] = T_CMD_DONT;
		cmd[2] = T_OPT_LINEMODE;
		NET_Write(cln, cmd, 3);
	}

	// Client wants us to send status
	if ((state == T_STATE_DO)
	&& ((type  == T_OPT_STATUS))) {
		Info(M_ANSWER" with 'WONT_STATUS'");
		cmd[0] = T_CMD_IAC;
		cmd[1] = T_CMD_WONT;
		cmd[2] = T_OPT_STATUS;
		NET_Write(cln, cmd, 3);
	}

	// Client wants to send XDISPLOC
	if ((state == T_STATE_WILL)
	&& ((type  == T_OPT_XDISPLOC))) {
		Info(M_ANSWER" with 'DONT_XDISPLOC'");
		cmd[0] = T_CMD_IAC;
		cmd[1] = T_CMD_DONT;
		cmd[2] = T_OPT_XDISPLOC;
		NET_Write(cln, cmd, 3);
	}
}

static void Subnegotiate(net_cln_t *cln, int type, byte *buf, int len)
{
	byte *head, *tail;
	short rows, cols;
	int i = 0;

	Info(M_NEWSUB " '%s'", GetOptStr(type));

	switch (type) {
	case T_OPT_NAWS:
		if (len != 4) {
			Warning(E_ERRSUB);
			return;
		}
		rows = (((short) buf[3]) << 8) | buf[2];
		cols = (((short) buf[1]) << 8) | buf[0];
		cln->rows = ntohs(rows);
		cln->cols = ntohs(cols);
		Info(M_SUBNAW " to '%dx%d'", cln->cols, cln->rows);
		break;

	case T_OPT_TTYPE:
		if (buf[0] != T_OPT_IS) {
			Warning(E_ERRSUB);
			break;
		}

		head = buf + 1;
		tail = buf + len;
		while (head < tail) {
			if ((buf[i+0] != T_CMD_IAC)
		        || ((buf[i+1] != T_CMD_SE))) {
				cln->type[i++] = *head++;
				continue;
			}
			break;
		}

		cln->type[i] = '\0';
		Info(M_SUBTTY " to '%s'", cln->type);
		break;
	}
}

int NET_Read(net_cln_t *cln, byte *out)
{
	int n, max, len = 0;
	byte data[MAX_MSG_LEN];
	const byte *head;
	const byte *tail;

	AS_NEQ_NULL(cln);
	AS_NEQ_NULL(out);

	max = MAX_MSG_LEN - 1;
	if ((n = recv(cln->handle, data, max, 0)) <= 0)
		return -1;

	head = data;
	tail = data + n;

	while (head < tail) // Seperate commands and data
		len += NET_Parse(cln, *head++, out + len);


	out[len] = '\0';
	return len;
}

int NET_Parse(net_cln_t *cln, byte c, byte *out)
{
	static byte buf[MAX_MSG_LEN];
	static int state = T_STATE_NONE;
	static int type, n;

	AS_NEQ_NULL(cln);
	AS_NEQ_NULL(out);

	switch(state) {
	case T_STATE_NONE:
		if (c == T_CMD_IAC) {
			state = T_STATE_IAC;
			break;
		}

		*out = c;
		return 1;

	case T_STATE_IAC:
		switch (c) {
		case T_CMD_IP: // Interrupt process
			Info(M_CLOSED " '%s'.", cln->name);
			close(cln->handle);
			exit(EXIT_SUCCESS);
			break;

		// Negotiate
		case T_CMD_WONT: state = T_STATE_WONT; break;
		case T_CMD_WILL: state = T_STATE_WILL; break;
		case T_CMD_DONT: state = T_STATE_DONT; break;
		case T_CMD_DO:   state = T_STATE_DO;   break;

		// Subnegotiate
		case T_CMD_SB:   state = T_STATE_SB;   break;

		} break;

	case T_STATE_SB:
		state = T_STATE_SB_DATA;
		type  = c;
		n     = 0;
		break;

	case T_STATE_SB_DATA:
		if (c == T_CMD_IAC)
			state = T_STATE_SB_DATA_IAC;
		else    // FIXME: Possible overflow
			buf[n++] = c;
		break;

	case T_STATE_SB_DATA_IAC:
		if (c == T_CMD_SE) {
			state = T_STATE_NONE;
			Subnegotiate(cln, type, buf, n);
		}
		break;

	case T_STATE_WONT:
	case T_STATE_WILL:
	case T_STATE_DONT:
	case T_STATE_DO:
		Negotiate(cln, state, c);
		state = T_STATE_NONE;
		break;
	}

	return 0;
}
#endif
