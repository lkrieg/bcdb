#include "common.h"

int main(int argc, char **argv)
{
	bool running;
	arg_t arg;

	if (FS_Init() < 0)
		Error(E_FSINIT);

	if (NET_Init() < 0)
		Error(E_NOSOCK);

	running = true;
	CMD_Parse(argc, argv);
	while (CMD_Get(&arg))
		switch(arg.type) {
		case C_SETPORT:
			break;
		case C_VERBOSE:
			break;
		default: // Unknown
			Error(E_ARGVAL);
		}

	while (running)
		break; // TODO

	Allocate(20);

	NET_Shutdown();
	FS_Shutdown();
	MemCheck();

	return 0;
}
