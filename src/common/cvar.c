#include "common.h"
#include "cvar.h"

typedef union {
         char *  str;
         int     num;
} cvar;

int CFG_Init(void)
{
	return 0;
}

int CFG_GetNumber(int type)
{
	UNUSED(type);
	return 0;
}

char *CFG_GetString(int type)
{
	UNUSED(type);
	return NULL;
}
