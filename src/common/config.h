#ifndef  CONFIG_H
#define  CONFIG_H

int     CFG_Init(void);
int     CFG_GetNum(int key);
char *  CFG_GetStr(int key);

enum cfg_index
{
         T_CFG_PORT,
         T_CFG_MODE,
	 NUM_CFG_VARS
};

#endif // CONFIG_H
