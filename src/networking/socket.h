#ifndef SOCKET_H
#define SOCKET_H

typedef struct net_evt_s net_evt_t;
typedef void (*net_func_t)(net_evt_t*);

int   NET_Init(int tel, int web, net_func_t handler);
int   NET_Accept(int socket);
int   NET_Update(void);
void  NET_Shutdown(void);

struct net_evt_s
{
	int type;
	int handle;
	int length;
	byte *data;
};

enum net_evt_type
{
	T_EVT_CLIENT_TEL,
	T_EVT_CLIENT_WEB,
	T_EVT_DATA,
	T_EVT_QUIT
};

#endif // SOCKET_H
