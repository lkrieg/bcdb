#include "common.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

static struct {
	int        fd;
	int        port;
	req_fun_t  func;
} net;

static void *RunThread(void *fd);

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

	if (fd < 0) {
		Warning(E_ACCEPT ": %s", strerror(errno));
		return;
	}

	if (pthread_create(&pid, NULL, RunThread, out) < 0)
		Warning(E_THREAD ": %s", strerror(errno));
}

static void *RunThread(void *arg)
{
	req_t req;
	int n;

	req.privileged  = false;
	req.handle      = *((int *) arg);

	while ((n = recv(req.handle, req.data, MAX_REQ_LEN, 0)) > 0) {
		req.type = T_REQ_QUERY;
		req.data[n] = '\0';
		net.func(&req);
	}

	if (n < 0)
		Warning(E_RECV);

	close(req.handle);
	return 0;
}

void NET_Shutdown(void)
{
	AS_GTH_ZERO(net.fd);

	close(net.fd);
}

/*
int NET_Listen(req_t *out)
{
	int socklen;
	struct sockaddr_in addr;

	listen(handle, 3);
	socklen = sizeof(addr);
	out->handle = accept(handle,
	                     (struct sockaddr *) &addr,
	                     (socklen_t *) &socklen);

	memset(out->data, 0, MAX_REQ_LEN);
	read(out->handle, out->data, MAX_REQ_LEN);
	out->type = GetRequestType(out);

	return out->type;
}

static int GetRequestType(req_t *req)
{
	char *head, c;
	char buf[MAX_REQ_LEN];
	int i = 0;

	head = req->data;
	while ((c = *head++))
		if (isalnum(c))
			buf[i++] = c;

	// TODO
	buf[i] = '\0';
	Info("%s\n", buf);

	return 1;
}

void NET_Answer(req_t *req, const char *msg)
{
	UNUSED(req);
	UNUSED(msg);
}
*/
