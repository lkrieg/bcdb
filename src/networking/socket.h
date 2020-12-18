#ifndef SOCKET_H
#define SOCKET_H

typedef struct net_cln_s net_cln_t;
typedef struct net_evt_s net_evt_t;
typedef void (*net_func_t)(net_evt_t*);

int   NET_Init(int tel, int web);
void  NET_SetHandler(net_func_t func);
int   NET_Update(void);
void  NET_Shutdown(void);

struct net_cln_s
{
	int   id;
	int   type;
	int   socket;
	char  addr[MAX_IPADDR];
};

struct net_evt_s
{
	int          type;
	int          length;
	net_cln_t *  client;
	byte      *  data;
};

enum net_cln_type
{
	T_CLN_TEL,
	T_CLN_WEB
};

enum net_evt_type
{
	T_EVT_CONNECTED,
	T_EVT_RECEIVED,
	T_EVT_CLOSED
};

#endif // SOCKET_H
