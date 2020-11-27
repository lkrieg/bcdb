#include "common.h"

int main(int argc, char **argv)
{
	arg_t arg;
	bool active;
	int port;

	if (FS_Init() < 0)
		Error(E_FSINIT);

	active = true;
	port   = DEFAULT_PORT;

	CMD_Parse(argc, argv);
	while (CMD_Get(&arg))
		switch(arg.type) {
		case T_ARG_PORT: port = arg.as.num;      break;
		case T_ARG_PATH: FS_AddPath(arg.as.str); break;
		default:         Error(E_ARGVAL);
		}

	if (NET_Init(port) < 0)
		Error(E_NOSOCK);

	while (active)
		break;

	NET_Shutdown();
	FS_Shutdown();
	MemCheck();

	return 0;
}
