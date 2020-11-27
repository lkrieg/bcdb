#include "common.h"

int main(int argc, char **argv)
{
	int type;
	arg_t arg;

	if (FS_Init() < 0)
		Error(E_FSINIT);

	if (NET_Init() < 0)
		Error(E_NOSOCK);

	CMD_Parse(argc, argv);
	while ((type = CMD_Get(&arg))) {
		switch(type) {
		case C_SETPORT:
			break;
		case C_VERBOSE:
			break;

		default: // Unknown
			Error(E_ARGVAL);
		}
	}

	NET_Shutdown();
	FS_Shutdown();

	return 0;
}
