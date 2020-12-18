#ifndef TELNET_H
#define TELNET_H

#include "socket.h"

int TEL_Init(void);
int TEL_Parse(net_cln_t *cln, const byte *data, int size);

#endif // TELNET_H
