#include "common.h"

#include <unistd.h>
#include <string.h>

static ht_tab_t table;

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
	Hash_Init(&table);

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
	int n = 0;
	int status, i;
	ht_ent_t *ent;

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
		"COMMANDS  ARGUMENTS  DESCRIPTION                                \n"\
		"--------  ---------  -------------------------------------------\n"\
		" query     BARCODE    Check if BARCODE exists and mark as done. \n"\
		" insert    BARCODE    Add BARCODE to database and mark as todo. \n"\
		" delete    BARCODE    Remove BARCODE from database.             \n"\
		" auth      PASSWD     Request elevated privileges.              \n"\
		" list                 Alias for 'list todo'.                    \n"\
		"           --full     Print full barcode list. Alias for 'all'. \n"\
		"           --done     Print list of already scanned barcodes.   \n"\
		"           --todo     Print list of still missing barcodes.     \n"\
		" quit                 Close connection. Alias for 'exit'.       \n");
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

	case T_REQ_QUERY:
		if (!(ent = Hash_Get(&table, req->params))) {
			NET_Error(req, E_NOKEY);
			break;
		}

		ent->status = T_ENT_DONE;
		NET_Answer(req, "Key '%s' found in database,"
		           " setting status to DONE", req->params);
		break;

	case T_REQ_INSERT:
		status = Hash_Insert(&table, req->params);
		if (status == -1) { NET_Error(req, E_KEYLEN); break; }
		if (status == -2) { NET_Error(req, E_EXISTS); break; }
		NET_Answer(req, "Sucessfully inserted key '%s'"
		           " into database", req->params);
		break;

	case T_REQ_DELETE:
		if (!Hash_Delete(&table, req->params)) {
			NET_Error(req, E_NOIMPL);
			break;
		}

		NET_Answer(req, "Successfully deleted key '%s'"
		           " from database", req->params);
		break;

	case T_REQ_LIST_FULL:
		// FIXME: Shamefully inefficient
		for (i = 0; i < MAX_HASH_SIZE; i++) {
			if (table.table[i] != NULL) {
				ent = table.table[i]; n++;
				NET_Answer(req, "%s", ent->key);
			}
		}

		if (n == 0)
			NET_Answer(req, "NONE");
		break;

	case T_REQ_LIST_DONE:
		// FIXME: Shamefully inefficient
		for (i = 0; i < MAX_HASH_SIZE; i++) {
			if ((table.table[i] != NULL)
			&& ((table.table[i]->status == T_ENT_DONE))) {
				ent = table.table[i]; n++;
				NET_Answer(req, "%s", ent->key);
			}
		}

		if (n == 0)
			NET_Answer(req, "NONE");
		break;

	case T_REQ_LIST_TODO:
		// FIXME: Shamefully inefficient
		for (i = 0; i < MAX_HASH_SIZE; i++) {
			if ((table.table[i] != NULL)
			&& ((table.table[i]->status == T_ENT_TODO))) {
				ent = table.table[i]; n++;
				NET_Answer(req, "%s", ent->key);
			}
		}

		if (n == 0)
			NET_Answer(req, "NONE");
		break;
	}
}
