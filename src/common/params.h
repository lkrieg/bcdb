#ifndef PARAMS_H
#define PARAMS_H

#include "common.h"

typedef struct cvar_s cvar_t;

int CFG_ParseFile(const char *path);
int CFG_ParseArgs(int argc, char **argv);
int CFG_Next(cvar_t *out);

// Access macros with type checking in DEBUG mode
#define CNUM(cvar)   (_C_AS((cvar), NUM), cvar.as.num)
#define CSTR(cvar)   (_C_AS((cvar), STR), cvar.as.str)
#define CBOOL(cvar)  (_C_AS((cvar), STR), cvar.as.bol)
#define _C_AS(v,t)   Assert((v).type == T_VAR_##t)

struct cvar_s
{
	int   id;
	int   type;
	char  raw[MAX_CFG_LEN];
	union {
		char *  str;
		int     num;
		bool    bol;
	} as;
};

enum cvar_ids
{
	T_CFG_INVALID  = -1,
	T_CFG_NONE     =  0,
	T_CFG_DAEMON,
	T_CFG_VERBOSE,
	T_CFG_KILL,
	T_CFG_FILE,
	T_CFG_PORT,
	T_CFG_HELP
};

enum cvar_types
{
	T_VAR_STR,
	T_VAR_NUM,
	T_VAR_BOL
};

#endif // PARAMS_H
