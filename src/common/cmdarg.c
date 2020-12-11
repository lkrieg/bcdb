#include "common.h"
#include "cmdarg.h"
#include <unistd.h>

static int    argnum;
static arg_t  argbuf[MAX_ARG_NUM];

void CMD_Init(int argc, char **argv)
{
	// if (argc > MAX_ARG_NUM)
	//	Error(E_ARGNUM)

	UNUSED(argc);
	UNUSED(argv);
	UNUSED(argnum);
	UNUSED(argbuf);
}

int CMD_Next(arg_t *out)
{
	// T_ARG_INVALID
	// T_ARG_DAEMON
	// T_ARG_SHUTDOWN
	// T_ARG_IMPORT

	out->type = T_ARG_INVALID;
	return out->type;
}
