#ifndef  PARAMS_H
#define  PARAMS_H

#include "common.h"

typedef struct arg_s arg_t;

int CMD_Init(int argc, char **argv);
int CMD_Next(arg_t *out);

struct arg_s
{
         int   type;
         char  str[MAX_ARG_LEN];
         int   num;
};

enum arg_type
{
         T_ARG_INVALID  = -1,
         T_ARG_NONE     =  0,
         T_ARG_DAEMON,
	 T_ARG_VERBOSE,
         T_ARG_KILL,
         T_ARG_FILE,
	 T_ARG_PORT,
	 T_ARG_HELP
};

#endif // PARAMS_H
