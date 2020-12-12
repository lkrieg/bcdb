#include "common.h"
#include "config.h"

struct cvar_s
{
	int type;
	union {
		char *  str;
		int     num;
		bool    bol;
	} as;
};

enum cvar_types
{
         T_NUMBER,
         T_STRING,
         T_BOOL
};

typedef struct cvar_s cvar_t;
static cvar_t cvars[NUM_CFG_VARS];
static int types[NUM_CFG_VARS] = {
	T_NUMBER,  // T_CFG_PORT
	T_BOOL,    // T_CFG_VERBOSE
	T_BOOL     // T_CFG_DAEMON
};

int CFG_Init(void)
{
	// TODO: Parse config file
	cvars[T_CFG_PORT].as.num     = DEFAULT_PORT;
	cvars[T_CFG_VERBOSE].as.bol  = false;
	cvars[T_CFG_DAEMON].as.bol   = false;

	return 0;
}

int CFG_GetNum(int key)
{
	Assert(key >= 0);
	Assert(key < NUM_CFG_VARS);
	Assert(types[key] == T_NUMBER);

	return cvars[key].as.num;
}

char *CFG_GetStr(int key)
{
	Assert(key >= 0);
	Assert(key < NUM_CFG_VARS);
	Assert(types[key] == T_STRING);

	return cvars[key].as.str;
}

bool CFG_GetBool(int key)
{
	Assert(key >= 0);
	Assert(key < NUM_CFG_VARS);
	Assert(types[key] == T_BOOL);

	return cvars[key].as.bol;
}
