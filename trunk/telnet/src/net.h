#ifndef NET_H
#define NET_H

#include "common.h"

typedef struct net_cln_s net_cln_t;
typedef struct net_evt_s net_evt_t;

int   NET_Init(void);
int   NET_Accept(net_cln_t *out);
void  NET_Negotiate(net_cln_t *cln, int flags);
int   NET_NextEvent(net_cln_t *cln, net_evt_t *out);
void  NET_Shutdown(void);

struct net_cln_s {
	int          handle;
	int          parent;
	char         name[MAX_ADR_LEN];
	net_evt_t *  queue;
	net_cln_t *  next;

};

struct net_evt_s {
	int          type;
	int          size;
	byte         data[MAX_MSG_LEN];
	int          keycode;
	short        rows;
	short        cols;
	net_evt_t *  next;
};

enum net_evt_type {
	T_EVT_QUIT = 0,
	T_EVT_NONE,
	T_EVT_DATA,
	T_EVT_KEYDOWN,
	T_EVT_RESIZE,
	T_EVT_TTYPE
};

enum evt_key_type {
	T_KEY_LEFT,
	T_KEY_RIGHT
};

// ======
// Telnet
// ======

// Negotiation flags      FLAG  OPT_TYPE
#define T_WILL  0x100  // 0001  00000000
#define T_WONT  0x200  // 0010  00000000
#define T_DO    0x400  // 0100  00000000
#define T_DONT  0x800  // 1000  00000000

// Commands
#define TEL_CMD_LIST \
CMD(  0xF0,  SE              ) \
CMD(  0xF1,  NOP             ) \
CMD(  0xF2,  DM              ) \
CMD(  0xF3,  BREAK           ) \
CMD(  0xF4,  IP              ) \
CMD(  0xF5,  AO              ) \
CMD(  0xF6,  AYT             ) \
CMD(  0xF7,  EC              ) \
CMD(  0xF8,  EL              ) \
CMD(  0xF9,  GA              ) \
CMD(  0xFA,  SB              ) \
CMD(  0xFB,  WILL            ) \
CMD(  0xFC,  WONT            ) \
CMD(  0xFD,  DO              ) \
CMD(  0xFE,  DONT            ) \
CMD(  0xFF,  IAC             ) \
// TEL_CMD_LIST

// Options
#define TEL_OPT_LIST \
OPT(  0x00,  BINARY          ) \
OPT(  0x01,  ECHO            ) \
OPT(  0x02,  RCP             ) \
OPT(  0x03,  SGA             ) \
OPT(  0x04,  NAMS            ) \
OPT(  0x05,  STATUS          ) \
OPT(  0x06,  TM              ) \
OPT(  0x07,  RCTE            ) \
OPT(  0x08,  NAOL            ) \
OPT(  0x09,  NAOP            ) \
OPT(  0x0A,  NAOCRD          ) \
OPT(  0x0B,  NAOHTS          ) \
OPT(  0x0C,  NAOHTD          ) \
OPT(  0x0D,  NAOFFD          ) \
OPT(  0x0E,  NAOVTS          ) \
OPT(  0x0F,  NAOVTD          ) \
OPT(  0x10,  NAOLFD          ) \
OPT(  0x11,  XASCII          ) \
OPT(  0x12,  LOGOUT          ) \
OPT(  0x13,  BM              ) \
OPT(  0x14,  DET             ) \
OPT(  0x15,  SUPDUP          ) \
OPT(  0x16,  SUPDUPOUTPUT    ) \
OPT(  0x17,  SNDLOC          ) \
OPT(  0x18,  TTYPE           ) \
OPT(  0x19,  EOR             ) \
OPT(  0x1A,  TUID            ) \
OPT(  0x1B,  OUTMRK          ) \
OPT(  0x1C,  TTYLOC          ) \
OPT(  0x1D,  3270REGIME      ) \
OPT(  0x1E,  X3PAD           ) \
OPT(  0x1F,  NAWS            ) \
OPT(  0x20,  TSPEED          ) \
OPT(  0x21,  LFLOW           ) \
OPT(  0x22,  LINEMODE        ) \
OPT(  0x23,  XDISPLOC        ) \
OPT(  0x24,  ENVIRON         ) \
OPT(  0x25,  AUTHENTICATION  ) \
OPT(  0x26,  ENCRYPT         ) \
OPT(  0x27,  NEW_ENVIRON     ) \
OPT(  0x2D,  SLE             ) \
OPT(  0x46,  MSSP            ) \
OPT(  0x55,  COMPRESS        ) \
OPT(  0x56,  COMPRESS2       ) \
OPT(  0x5D,  ZMP             ) \
OPT(  0xFF,  EXOPL           ) \
// TEL_OPT_LIST

// Subnegotiation
#define TEL_SUB_LIST \
SUB(  0x00,  IS              ) \
SUB(  0x01,  SEND            ) \
// TEL_SUB_LIST

enum tel_state {
#define CMD(n, s) T_STATE_##s,
#define OPT(n, s) T_STATE_##s,
#define SUB(n, s) T_STATE_##s,
T_STATE_NONE = 0,
T_STATE_COMMAND,
T_STATE_NEGOTIATE,
T_STATE_SUB,
T_STATE_SUB_DATA,
TEL_CMD_LIST
TEL_OPT_LIST
TEL_SUB_LIST
TEL_NUM_STATES
#undef SUB
#undef OPT
#undef CMD
};

enum tel_cmd_type {
#define CMD(n, s) T_CMD_##s = n,
TEL_CMD_LIST
#undef CMD
};

enum tel_opt_type {
#define OPT(n, s) T_OPT_##s = n,
TEL_OPT_LIST
#undef OPT
};

enum tel_sub_type {
#define SUB(n, s) T_SUB_##s = n,
TEL_SUB_LIST
#undef SUB
};

#endif // NET_H
