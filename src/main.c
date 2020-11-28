#include "common.h"

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
		switch(arg.type) { // Handle command line flags
		case T_ARG_PORT: port = arg.as.num;      break;
		case T_ARG_PATH: FS_AddPath(arg.as.str); break;
		default:         Error(E_ARGVAL);
		}

	// Seperate thread for each client
	if (NET_Init(port, HandleRequest) < 0)
		Error(E_NOSOCK " %d", port);

	for (;;) // Handle request
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
		Info("QUERY %s", req->data);
		break;
	case T_REQ_INSERT:
		Info("INSERT %s", req->data);
		break;
	case T_REQ_DELETE:
		Info("DELETE %s", req->data);
		break;
	case T_REQ_LIST_ALL:
		Info("LIST_ALL");
		break;
	case T_REQ_LIST_DONE:
		Info("LIST_DONE");
		break;
	case T_REQ_LIST_TODO:
		Info("LIST_TODO");
		break;
	case T_REQ_AUTH:
		req->privileged = true;
		Info("AUTH %s", req->data);
		break;
	case T_REQ_EXIT:
		Info("EXIT");
		break;
	}
}
