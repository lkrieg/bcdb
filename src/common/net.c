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

static struct {
	int        fd;
	int        port;
	req_fun_t  func;
} net;

static void *RunThread(void *fd);
static int ParseRequest(req_t *req);

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

void NET_Answer(req_t *req, const char *data)
{
	if (write(req->handle, data, strlen(data)) < 0)
		Warning(E_ANSWER);
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

	do {
		data = req.data;
		len = MAX_REQ_LEN;

		// Receive and handle client request message
		if ((n = recv(req.handle, data, len, 0)) > 0) {
			req.data[n] = '\0';
			if (ParseRequest(&req) < 0) {
				Warning(E_REQVAL);
				continue;
			}
			net.func(&req);
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

	// Filter control codes
	for (; *head; head++) {
		if (isalnum(*head))
			*tail++ = *head;
	}

	*tail = '\0';
	head = req->data;
	req->type = T_REQ_INVAL;

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
			&& ((head[3] == 'T' || head[3] == 't'))) {
				req->type = T_REQ_EXIT;
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
		case 'Q': case 'q': // QUERY
			if ((head[1] == 'U' || head[1] == 'u')
			&& ((head[2] == 'E' || head[2] == 'e'))
			&& ((head[3] == 'R' || head[3] == 'r'))
			&& ((head[4] == 'Y' || head[4] == 'y'))) {
				req->type = T_REQ_QUERY;
				req->params = head + 5;
			}
			break;
		case 'L': case 'l': // LIST X
			break;
	}

	return req->type;
}

