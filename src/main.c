#include "common.h"

#include <unistd.h>
#include <string.h>

static ht_tab_t *table;

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

	// Create hashtable
	Hash_Init(table);

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
	int status;

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

	case T_REQ_HELP:
		NET_Answer(req,
		"\tCOMMAND  |  PARAMS  |  DESCRIPTION                       \n"\
		"\t---------+----------+----------------------------------- \n"\
		"\tquery    |  KEY     |  Check if KEY exists and mark done \n"\
		"\tinsert   |  KEY     |  Add KEY to database and mark todo \n"\
		"\tdelete   |  KEY     |  Remove KEY entry from database    \n"\
		"\tauth     |  PWD     |  Request elevated privileges       \n"\
		"\tlist     |          |  Alias for 'list --todo'           \n"\
		"\tlist     |  --full  |  Print list of all barcodes        \n"\
		"\tlist     |  --done  |  Print list of scanned barcodes    \n"\
		"\tlist     |  --todo  |  Print list of missing barcodes    \n"\
		"\texit     |          |  Close connection                  \n"\
		"\tquit     |          |  Alias for 'exit'                  ");
		break;

	case T_REQ_QUERY:
		status = Hash_Exists(table, req->params);
		NET_Answer(req, (!status) ? "OK" : "Error");
		break;

	case T_REQ_INSERT:
		status = Hash_Insert(table, req->params);
		NET_Answer(req, (!status) ? "OK" : "Error");
		break;

	case T_REQ_DELETE:
		status = Hash_Delete(table, req->params);
		NET_Answer(req, (!status) ? "OK" : "Error");
		break;

	case T_REQ_LIST_FULL:
		NET_Answer(req, "Listing all barcodes...");
		break;

	case T_REQ_LIST_DONE:
		NET_Answer(req, "Listing finished barcodes...");
		break;

	case T_REQ_LIST_TODO:
		NET_Answer(req, "Listing missing barcodes...");
		break;

	case T_REQ_AUTH:
		if (strcmp(req->params, "123")) {
			NET_Error(req, E_NOCRED);
			break; // Plaintext!
		}

		req->privileged = true;
		NET_Answer(req, "OK");
		break;

	case T_REQ_EXIT:
		close(req->handle);
		break;
	}
}
