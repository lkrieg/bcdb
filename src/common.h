#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include "common/config.h"
#include "common/errors.h"

typedef  uint32_t       u32;
typedef  uint16_t       u16;
typedef  unsigned char  byte;

#define  MAX_PATH       4096
#define  MAX_LINEBUF    2048
#define  MAX_BACKLOG    10
#define  MIN_ROW_NUM    14
#define  MIN_COL_NUM    24
#define  MAX_TTYPE      48

#define  BIT(n)         (1UL << (n))
#define  UNUSED(sym)    ((void)(sym))
#define  ARSIZE(ptr)    (sizeof(ptr)/sizeof(*(ptr)))
#define  ASSERT(exp)    UNUSED(0) // Debug mode only

#define  Allocate(n)    _Allocate(n)
#define  Free(ptr)      _Free(ptr)
#define  MemCheck()     UNUSED(0)
#define  Verbose(...)   UNUSED(0)

void     Info(const char * fmt, ...);
void     Warning(const char * fmt, ...);
void     Error(const char * fmt, ...);

#ifdef   DEBUG          // DEBUG MODE:
#undef   Allocate       // - Tracks memory allocations
#undef   Verbose        // - Outputs verbose log messages
#undef   Assert         // - Enables runtime assertions
#undef   MemCheck       // - Prints memory leak reports
#undef   Free

#define  Verbose(...)   _Verbose(__VA_ARGS__)
#define  Assert(exp)    _Assert((exp), #exp, __FILE__, __LINE__)
#define  Allocate(n)    _AllocateDebug((n),  __FILE__, __LINE__)
#define  Free(ptr)      _FreeDebug(ptr)
#define  MemCheck()     _MemCheck()
#endif

// Internal helper functions - do not use these directly.
// Use the macros without prefixed underscore instead.

void *   _Allocate(int size);
void *   _AllocateDebug(int size, const char *file, int line);
void     _Assert(bool exp, const char *text, const char *file, int line);
void     _Verbose(const char *fmt, ...);
void     _Free(void *ptr);
void     _FreeDebug(void *ptr);
void     _MemCheck(void);

#endif // COMMON_H
