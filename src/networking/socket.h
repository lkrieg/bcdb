#ifndef SOCKET_H
#define SOCKET_H

typedef struct net_cln_s net_cln_t;

int   NET_Init(int tel, int web);
int   NET_Accept(net_cln_t *out);
void  NET_Shutdown(void);

struct net_cln_s
{
	char addr[MAX_IPADDR];
};

#endif // SOCKET_H
