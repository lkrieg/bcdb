#include "common.h"
#include "config.h"

union cvar_u
{
	char *  str;
	int     num;
};

typedef union cvar_u cvar_t;
static cvar_t cvars[NUM_CFG_VARS];

int CFG_Init(void)
{
	// TODO
	UNUSED(cvars);
	return 0;
}

int CFG_GetNum(int key)
{
	// TODO
	UNUSED(key);
	return 0;
}

char *CFG_GetStr(int key)
{
	// TODO
	UNUSED(key);
	return NULL;
}
