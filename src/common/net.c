#include "common.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>

static struct {
	int        fd;
	int        port;
	req_fun_t  func;
} net;

static void *RunThread(void *fd);
static int ParseRequest(req_t *req);
static void Prompt(req_t *req);

int NET_Init(int port, req_fun_t func)
{
	struct sockaddr_in in;
	struct sockaddr *addr;

	AS_GEQ_ZERO(port);
	AS_NEQ_NULL(func);

	net.port  = port;
	net.func  = func;
	net.fd    = socket(AF_INET, SOCK_STREAM, 0);

	if (net.fd < 0)
		return -1;

	in.sin_family       = AF_INET;
	in.sin_addr.s_addr  = INADDR_ANY;
	in.sin_port         = htons(net.port);

	addr = (struct sockaddr *) &in;
	if (bind(net.fd, addr, sizeof(in)) >= 0)
		return listen(net.fd, 3);

	return -1;
}

void NET_Accept(void)
{
	int fd;
	struct sockaddr_in in;
	struct sockaddr *addr;
	socklen_t len;
	pthread_t pid;
	void *out;

	AS_GTH_ZERO(net.fd);

	out   = (void*) &fd;
	addr  = (struct sockaddr *) &in;
	len   = sizeof(in);
	fd    = accept(net.fd, addr, &len);

	if (fd < 0) // Create seperate thread for client
		Warning(E_ACCEPT ": %s", strerror(errno));
	else if (pthread_create(&pid, NULL, RunThread, out) < 0)
		Warning(E_THREAD ": %s", strerror(errno));
}

static void NET_Message(req_t *req, const char *fmt, va_list arg)
{
	char msg[MAX_MSG_LEN];

	AS_NEQ_NULL(req);
	AS_NEQ_NULL(fmt);

	vsnprintf(msg, MAX_MSG_LEN, fmt, arg);
	write(req->handle, msg, strlen(msg));
	write(req->handle, "\n", 1);
}

void NET_Answer(req_t *req, const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	NET_Message(req, fmt, arg);
	va_end(arg);
}

void NET_Error(req_t *req, const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	NET_Message(req, fmt, arg);
	va_end(arg);
}

void NET_Shutdown(void)
{
	AS_GTH_ZERO(net.fd);

	close(net.fd);
}

static void *RunThread(void *arg)
{
	char *data;
	req_t req;
	int n, len;

	req.privileged  = false;
	req.handle      = *((int *) arg);
	Prompt(&req);

	do {
		data = req.data;
		len  = MAX_REQ_LEN;

		if ((n = recv(req.handle, data, len, 0)) > 0) {
			req.data[n] = '\0';
			ParseRequest(&req);
			net.func(&req);

			if (req.type != T_REQ_EMPTY)
				Prompt(&req);
		}

	} while (n > 0);
	close(req.handle);

	return 0;
}

static int ParseRequest(req_t *req)
{
	char *head;
	char *tail;

	head = req->data;
	tail = head;

	for (; *head; head++) {
		if (!isalnum(*head))
			continue;

		// Filter garbage
		*tail++ = *head;
	}

	req->type = T_REQ_INVAL;
	head      = req->data;
	*tail     = '\0';

	switch (*head) {
		case 'A': case 'a': // AUTH
			if ((head[1] == 'U' || head[1] == 'u')
			&& ((head[2] == 'T' || head[2] == 't'))
			&& ((head[3] == 'H' || head[3] == 'h'))) {
				req->type = T_REQ_AUTH;
				req->params = head + 4;
			}
			break;

		case 'D': case 'd': // DELETE
			if ((head[1] == 'E' || head[1] == 'e')
			&& ((head[2] == 'L' || head[2] == 'l'))
			&& ((head[3] == 'E' || head[3] == 'e'))
			&& ((head[4] == 'T' || head[4] == 't'))
			&& ((head[5] == 'E' || head[5] == 'e'))) {
				req->type = T_REQ_DELETE;
				req->params = head + 6;
			}
			break;

		case 'E': case 'e': // EXIT
			if ((head[1] == 'X' || head[1] == 'x')
			&& ((head[2] == 'I' || head[2] == 'i'))
			&& ((head[3] == 'T' || head[3] == 't'))
			&& ((head[4] == '\0'))) {
				req->type = T_REQ_EXIT;
				req->params = head + 4;
			}
			break;

		case 'H': case 'h': // HELP
			if ((head[1] == 'E' || head[1] == 'e')
			&& ((head[2] == 'L' || head[2] == 'l'))
			&& ((head[3] == 'P' || head[3] == 'p'))
			&& ((head[4] == '\0'))) {
				req->type = T_REQ_HELP;
				req->params = head + 4;
			}
			break;

		case 'I': case 'i': // INSERT
			if ((head[1] == 'N' || head[1] == 'n')
			&& ((head[2] == 'S' || head[2] == 's'))
			&& ((head[3] == 'E' || head[3] == 'e'))
			&& ((head[4] == 'R' || head[4] == 'r'))
			&& ((head[5] == 'T' || head[5] == 't'))) {
				req->type = T_REQ_INSERT;
				req->params = head + 6;
			}
			break;

		case 'Q': case 'q': // QUERY | QUIT
			if ((head[1] == 'U' || head[1] == 'u')
			&& ((head[2] == 'E' || head[2] == 'e'))
			&& ((head[3] == 'R' || head[3] == 'r'))
			&& ((head[4] == 'Y' || head[4] == 'y'))) {
				req->type = T_REQ_QUERY;
				req->params = head + 5;

			} else if ((head[1] == 'U' || head[1] == 'u')
			&& ((head[2] == 'I' || head[2] == 'i'))
			&& ((head[3] == 'T' || head[3] == 't'))
			&& ((head[4] == '\0'))) {
				req->type = T_REQ_EXIT;
				req->params = head + 4;
			}
			break;

		case 'L': case 'l': // LIST
			if ((head[1] == 'I' || head[1] == 'i')
			&& ((head[2] == 'S' || head[2] == 's'))
			&& ((head[3] == 'T' || head[3] == 't'))) {

				if ((head[4] == '\0')
				|| ((head[4] == 'T' || head[4] == 't')
				&& ((head[5] == 'O' || head[5] == 'o'))
				&& ((head[6] == 'D' || head[6] == 'd'))
				&& ((head[7] == 'O' || head[7] == 'o'))
				&& ((head[8] == '\0')))) {
					req->type = T_REQ_LIST_TODO;
					req->params = head + 8;
					break;
				}

				if ((head[4] == 'D' || head[4] == 'd')
				&& ((head[5] == 'O' || head[5] == 'o'))
				&& ((head[6] == 'N' || head[6] == 'n'))
				&& ((head[7] == 'E' || head[7] == 'e'))
				&& ((head[8] == '\0'))) {
					req->type = T_REQ_LIST_DONE;
					req->params = head + 8;
				}

				if ((head[4] == 'F' || head[4] == 'f')
				&& ((head[5] == 'U' || head[5] == 'u'))
				&& ((head[6] == 'L' || head[6] == 'l'))
				&& ((head[7] == 'L' || head[7] == 'l'))
				&& ((head[8] == '\0'))) {
					req->type = T_REQ_LIST_FULL;
					req->params = head + 8;
					break;
				}

				if ((head[4] == 'A' || head[4] == 'a')
				&& ((head[5] == 'L' || head[5] == 'l'))
				&& ((head[6] == 'L' || head[6] == 'l'))
				&& ((head[7] == '\0'))) {
					req->type = T_REQ_LIST_FULL;
					req->params = head + 7;
					break;
				}
			}
			break;

		case '\0': // EMPTY
			req->type = T_REQ_EMPTY;
			req->params = head;
			break;
	}

	return req->type;
}

static void Prompt(req_t *req)
{
	write(req->handle, "\n$ ", 3);
}
