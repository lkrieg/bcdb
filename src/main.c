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

	if (NET_Init(port) < 0)
		Error(E_NOSOCK " %d", port);

	// Seperate threads for each client
	NET_SetRequestHandler(HandleRequest);

	for (;;) // Handle requests
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
	case T_REQ_INSERT:
	case T_REQ_DELETE:
	case T_REQ_LIST_ALL:
	case T_REQ_LIST_DONE:
	case T_REQ_LIST_TODO:
		break;
	}
}
