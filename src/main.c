#include "common.h"

#include <unistd.h>
#include <string.h>

static void HandleRequest(req_t *req);

int main(int argc, char **argv)
{
	int port;
	arg_t arg;

	if (FS_Init() < 0)
		Error(E_FSINIT);

	port = DEFAULT_PORT;
	CMD_Parse(argc, argv);

	while (CMD_Next(&arg))
		switch(arg.type) { // Parse command line flags
		case T_ARG_PORT: port = arg.as.num;      break;
		case T_ARG_PATH: FS_AddPath(arg.as.str); break;
		default:         Error(E_ARGVAL);
		}

	// Initialize threaded request handling
	if (NET_Init(port, HandleRequest) < 0)
		Error(E_NOSOCK " %d", port);

	for (;;) // Next client
		NET_Accept();

	NET_Shutdown();
	FS_Shutdown();
	MemCheck();

	return 0;
}

static void HandleRequest(req_t *req)
{
	if ((!req->privileged)
	&& ((req->type == T_REQ_INSERT)
	|| ((req->type == T_REQ_DELETE)))) {
		NET_Error(req, E_ACCESS);
		return;
	}

	if ((!req->params[0])
	&& ((req->type == T_REQ_QUERY)
	|| ((req->type == T_REQ_INSERT))
	|| ((req->type == T_REQ_DELETE))
	|| ((req->type == T_REQ_AUTH)))) {
		NET_Error(req, E_CMDARG);
		return;
	}

	switch(req->type) {
	case T_REQ_INVAL:
		NET_Error(req, E_REQVAL);
		break;
	case T_REQ_QUERY:
		NET_Answer(req, "QUERY %s", req->params);
		break;
	case T_REQ_INSERT:
		NET_Answer(req, "INSERT %s", req->params);
		break;
	case T_REQ_DELETE:
		NET_Answer(req, "DELETE %s", req->params);
		break;
	case T_REQ_LIST_FULL:
		NET_Answer(req, "LIST_FULL");
		break;
	case T_REQ_LIST_DONE:
		NET_Answer(req, "LIST_DONE");
		break;
	case T_REQ_LIST_TODO:
		NET_Answer(req, "LIST_TODO");
		break;
	case T_REQ_AUTH:
		if (req->privileged)
			break;

		if (strcmp(req->params, "123")) {
			NET_Error(req, E_NOCRED);
			break; // Plaintext!
		}

		req->privileged = true;
		NET_Answer(req, "OK");
		break;
	case T_REQ_HELP:
		NET_Answer(req, "HELP");
		break;
	case T_REQ_EXIT:
		close(req->handle);
		break;
	}
}
