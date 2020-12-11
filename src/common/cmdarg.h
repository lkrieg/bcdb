#ifndef  CMDARG_H
#define  CMDARG_H

#include "common.h"

typedef  struct arg_s  arg_t;

void     CMD_Init(int argc, char **argv);
int      CMD_Next(arg_t *out);

struct arg_s
{
         int   type;
         char  value[MAX_ARG_LEN];
};

enum arg_type
{
         T_ARG_INVALID  = -1,
         T_ARG_NONE     =  0,
         T_ARG_DAEMON,
         T_ARG_SHUTDOWN,
         T_ARG_IMPORT,
};

#endif // CMDARG_H
