#ifndef TELNET_H
#define TELNET_H

#include "socket.h"

int TEL_Init(void);
void TEL_Negotiate(net_cln_t *cln);
int TEL_Parse(net_cln_t *cln, const byte *data, int size);

#endif // TELNET_H
