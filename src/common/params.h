#ifndef PARAMS_H
#define PARAMS_H

#include "common.h"

typedef struct cvar_s cvar_t;

int CFG_ParseFile(const char *path);
int CFG_ParseArgs(int argc, char **argv);
int CFG_Next(cvar_t *out);

// Access macros with type checking in DEBUG mode
#define CNUM(cvar)   (_C_AS((cvar), NUM),  cvar.as.num)
#define CSTR(cvar)   (_C_AS((cvar), STR),  cvar.as.str)
#define CBOOL(cvar)  (_C_AS((cvar), BOOL), cvar.as.bol)
#define _C_AS(v,t)   Assert((v).type == T_VAR_##t)

struct cvar_s
{
	int   id;
	int   type;
	char  argchar;
	char  key[MAX_CFG_KEY];
	char  val[MAX_CFG_VAL];
	union {
		char *  str;
		int     num;
		bool    bol;
	} as;
};

enum cvar_ids
{
	T_CFG_DAEMON = 1,
	T_CFG_VERBOSE,
	T_CFG_KILL,
	T_CFG_FILE,
	T_CFG_PORT,
	T_CFG_HELP,

	NUM_CVAR_IDS,
	T_CFG_END = 0
};

enum cvar_types
{
	T_VAR_STR,
	T_VAR_NUM,
	T_VAR_BOOL
};

#endif // PARAMS_H
