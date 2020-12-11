#include "common.h"
#include "cmdarg.h"

void CMD_Init(int argc, char **argv)
{
	UNUSED(argc);
	UNUSED(argv);
}

int CMD_Next(arg_t *out)
{
	UNUSED(out);
	// T_ARG_INVALID
	// T_ARG_DAEMON
	// T_ARG_SHUTDOWN
	// T_ARG_IMPORT
	return T_ARG_NONE;
}
