#include "common.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

static int handle;

static int GetRequestType(req_t *req);

int NET_Init(int port)
{
	struct sockaddr_in addr;

	handle = socket(AF_INET, SOCK_STREAM, 0);
	if (handle < 0)
		return -1;

	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port        = htons(port);

	return bind(handle, (struct sockaddr *)
	            &addr,  sizeof(addr));
}

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

void NET_Shutdown(void)
{
	close(handle);
}
