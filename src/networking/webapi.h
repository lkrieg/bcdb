#ifndef WEBAPI_H
#define WEBAPI_H

#include "socket.h"

int WEB_Init(void);
int WEB_Parse(net_cln_t *cln, const byte *data, int size);

#endif // WEBAPI_H
