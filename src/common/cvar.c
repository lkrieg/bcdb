#include "common.h"
#include "cvar.h"

static union {
	char *  str;
	int     num;
} cvars[NUM_CVARS];

int CFG_Init(void)
{
	// TODO
	UNUSED(cvars);
	return 0;
}

int CFG_GetNumber(int type)
{
	// TODO
	UNUSED(type);
	return 0;
}

char *CFG_GetString(int type)
{
	// TODO
	UNUSED(type);
	return NULL;
}
