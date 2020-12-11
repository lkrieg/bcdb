#ifndef  CVAR_H
#define  CVAR_H

int      CFG_Init(void);
int      CFG_GetNumber(int type);
char *   CFG_GetString(int type);

enum cvar_type
{
         T_CFG_PORT,
         T_CFG_MODE,
         NUM_CVARS
};

#endif // CVAR_H
