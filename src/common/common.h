#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "external/telnet.h"

#define MAX_BACKLOG     10
#define MAX_ADR_LEN     46
#define MAX_USR_LEN     80
#define MAX_MSG_LEN     4096

#define UNUSED(sym)     ((void)(sym))
#define Info(...)       _printl("Info: "    __VA_ARGS__)
#define Warning(...)    _printl("Warning: " __VA_ARGS__)
#define Error(...)      _perror("Error: "   __VA_ARGS__)
#define Print(...)      _print(__VA_ARGS__)

#define E_SIGNAL        "Unable to set signal handlers"
#define E_GETADR        "Unable to get socket address"
#define E_NOSOCK        "Unable to get socket descriptor"
#define E_SETOPT        "Unable to set socket option"
#define E_NOBIND        "Unable to bind to socket"
#define E_LISTEN        "Unable to listen from socket"
#define E_IPFAIL        "Network initialization failed"
#define E_ACCEPT        "Unable to accept client"
#define E_NOFORK        "Unable to fork process"
#define E_RXDATA        "Unable to receive data"
#define E_TXDATA        "Unable to send data"
#define M_ACCEPT        "Accepting new client"

typedef unsigned char byte;
typedef struct net_cln_s net_cln_t;
typedef struct net_evt_s net_evt_t;

int   NET_Init(void);
int   NET_Accept(net_cln_t *out);
int   NET_NextEvent(net_cln_t *cln, net_evt_t *out);
void  NET_Send(net_cln_t *cln, const char *buf, int size);
void  NET_Close(net_cln_t *cln);
void  NET_Shutdown(void);

struct net_cln_s {
	int         handle;
	int         parent;
	char        address[MAX_ADR_LEN];
	char        username[MAX_USR_LEN];
	telnet_t *  telnet;
};

struct net_evt_s {
	int         type;
};

enum net_evt_type {
	T_EVT_QUIT = 0,
	T_EVT_NONE,
	T_EVT_DATA,
	T_EVT_RESIZE
};

#define _perror(...)    do { _printl(__VA_ARGS__); _fatal(); } while(0)
#define _printl(...)    do {  printf(__VA_ARGS__); _eolff(); } while(0)
#define _print(...)     do {  printf(__VA_ARGS__); _flush(); } while(0)
#define _eolff()        putchar('\n'); _flush()
#define _fatal()        exit(EXIT_FAILURE)
#define _flush()        fflush(stdout);

#endif // COMMON_H
