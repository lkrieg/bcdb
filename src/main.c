#include "common.h"

int main(int argc, char **argv)
{
	int arg;
	char val[MAX_ARG_LEN];

	if (FS_Init() < 0)
		Error(E_FSINIT);

	if (NET_Init() < 0)
		Error(E_NOSOCK);

	Arg_Parse(argc, argv);
	while ((arg = Arg_Get(val)))
		switch(arg) {

		case C_SETPORT:
			break; // TODO

		case C_VERBOSE:
			break; // TODO

		default: // Unknown
			Error(E_ARGVAL);
		}

	NET_Shutdown();
	FS_Shutdown();

	UNUSED(val);

	return 0;
}
