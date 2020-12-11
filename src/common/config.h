#ifndef  CONFIG_H
#define  CONFIG_H

int    CFG_Init(void);
int    CFG_GetNum(int key);
char * CFG_GetStr(int key);
bool   CFG_GetBool(int key);

enum cfg_indices
{
         T_CFG_PORT,
	 T_CFG_VERBOSE,
	 NUM_CFG_VARS
};

#endif // CONFIG_H
