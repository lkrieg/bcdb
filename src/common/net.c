#include "common.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

static int handle;

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
	int cln, len, i = 0;
	struct sockaddr_in addr;
	char data[MAX_REQ_LEN];
	char *head;

	// TODO
	listen(handle, 3);
	len = sizeof(addr);
	cln = accept(handle, (struct sockaddr *)
	             &addr,  (socklen_t *) &len);

	out->handle  = cln;
	out->data[0] = '\0';

	while (read(cln, data, sizeof(data))) {
		for (head = data; *head; head++) {
			if (!isalnum(*head))
				continue;

			out->data[i++] = *head;
		}

		out->data[i] = '\0';
		Info("%s", out->data);
		out->data[0] = '\0';
		i = 0;
	}

	close(cln);
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
