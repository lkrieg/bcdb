#ifndef  COMMON_H
#define  COMMON_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <signal.h>
#include <errno.h>

#include "config.h"

typedef  uint32_t       u32;
typedef  uint16_t       u16;
typedef  unsigned char  byte;

#define  MAX_PATH       4096
#define  MAX_FILEBUF    8192
#define  MAX_LINEBUF    2048
#define  MAX_BACKLOG    10
#define  MAX_KEY_LEN    32
#define  MAX_CFG_NUM    64
#define  MAX_CFG_KEY    16
#define  MAX_CFG_VAL    128
#define  MAX_TTYPE      48
#define  MIN_ROWS       14
#define  MIN_COLS       24
#define  BASE_TEL_PORT  23
#define  BASE_WEB_PORT  80
#define  BASE_FILE      NULL

#define  CFGPATH        "/etc/barkeeper.conf"
#define  LOGPATH        "/var/log/barkeeper.log"
#define  PIDPATH        "/var/run/barkeeper.pid"
#define  VARDIR         "/var/lib/barkeeper"

#define  BIT(n)         (1UL << (n))
#define  UNUSED(sym)    ((void)(sym))
#define  ARSIZE(ptr)    (sizeof(ptr)/sizeof(*(ptr)))
#define  Assert(exp)    UNUSED(0) // Debug mode only
#define  Memcheck()     UNUSED(0) // Debug mode only
#define  Allocate(n)    _Allocate(n)
#define  Free(ptr)      _Free(ptr)

#ifdef   DEBUG          // DEBUG MODE:
#undef   Assert         // - Enables runtime assertions
#undef   Allocate       // - Tracks memory allocations
#undef   Memcheck       // - Prints memory leak report
#undef   Free

#define  Assert(exp)    _Assert((exp), #exp, __FILE__, __LINE__)
#define  Allocate(n)    _AllocateDebug((n),  __FILE__, __LINE__)
#define  Free(ptr)      _FreeDebug(ptr)
#define  Memcheck()     _Memcheck()
#endif

void     Info(const char *fmt, ...);
void     Verbose(const char *fmt, ...);
void     Warning(const char *fmt, ...);
void     Error(const char *fmt, ...);
void     Print(const char *fmt, ...);

int      GetActivePid(void);
void     SetPidLock(bool locked);
bool     IsPrivileged(void);
int      ForkProcess(void);
void     KillProcess(void);
void     Sleep(int n);

// Internal helper functions - do not use these directly.
// Use the macros without prefixed underscore instead.

void *   _Allocate(int size);
void *   _AllocateDebug(int size, const char *file, int line);
void     _Assert(int exp, const char *text, const char *file, int line);
void     _Free(void *ptr);
void     _FreeDebug(void *ptr);
void     _Memcheck(void);

// Submodule headers - see the respective files for
// details about interface and implementation.

#include "common/errors.h"
#include "common/params.h"
#include "networking/socket.h"
#include "networking/telnet.h"
#include "networking/webapi.h"
#include "database/database.h"
#include "database/filesystem.h"
#include "database/hashtable.h"
#include "renderer/renderer.h"

extern bool verbose;

#endif // COMMON_H
