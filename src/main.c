#include "common.h"

#include <unistd.h>

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
	switch(req->type) {
	case T_REQ_QUERY:
		Info("QUERY %s", req->params);
		// NET_Answer(req, "OK");
		break;
	case T_REQ_INSERT:
		Info("INSERT %s", req->params);
		break;
	case T_REQ_DELETE:
		Info("DELETE %s", req->params);
		break;
	case T_REQ_LIST_FULL:
		Info("LIST_FULL");
		break;
	case T_REQ_LIST_DONE:
		Info("LIST_DONE");
		break;
	case T_REQ_LIST_TODO:
		Info("LIST_TODO");
		break;
	case T_REQ_AUTH:
		Info("AUTH %s", req->params);
		req->privileged = true;
		break;
	case T_REQ_EXIT:
		Info("EXIT");
		close(req->handle);
		break;
	}
}
