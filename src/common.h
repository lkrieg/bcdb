#ifndef  COMMON_H
#define  COMMON_H

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

//       =========
//       CONSTANTS
//       =========

#define  DEBUG                     // Enable debug mode
#define  DEFAULT_PORT       23     // Default server socket port
#define  MAX_PATH           4096   // Cannot rely on PATH_MAX from limits.h
#define  MAX_ARG_LEN        1024   // Command line argument length limit
#define  MAX_MSG_LEN        2048   // Log message length limit
#define  MAX_REQ_LEN        2048   // Client request length limit
#define  MAX_HASH_SIZE      4096   // Number of possible hashes

//       ========
//       TYPEDEFS
//       ========

typedef  uint32_t           u32;   // Guaranteed 32 bit data type
typedef  uint16_t           u16;   // Guaranteed 16 bit data type
typedef  unsigned char      byte;  // uint8_t could be a non-character
                                   //   type and therefore break aliasing
//       ======
//       ERRORS
//       ======

#define  E_ASSERT           "Assertion failure"
#define  E_NOMEM            "Memory allocation failure"
#define  E_NOSOCK           "Could not bind to network port"
#define  E_FSINIT           "Could not initialize filesystem"
#define  E_ARGVAL           "Unknown command line argument"
#define  E_ACCEPT           "Could not accept request"
#define  E_THREAD           "Could not create new thread"

//       =======
//       UTILITY
//       =======

#define  BIT(n)             (1UL << (n))   // Bitmask for nth bit
#define  UNUSED(sym)        ((void)(sym))  // Suppress -Wunused warnings
#define  Assert(exp)        UNUSED(0)      // DEBUG: Runtime assertion
#define  AS_(exp)           Assert(exp)      // Assertion shorthands:
#define  AS_EQL_(l, r)      AS_((l) == (r))  // - Equality
#define  AS_NEQ_(l, r)      AS_((l) != (r))  // - Inequality
#define  AS_LEQ_(l, r)      AS_((l) <= (r))  // - Less or equal
#define  AS_GEQ_(l, r)      AS_((l) >= (r))  // - Greater or equal
#define  AS_LTH_(l, r)      AS_((l) <  (r))  // - Less
#define  AS_GTH_(l, r)      AS_((l) >  (r))  // - Greater
#define  AS_EQL_NULL(p)     AS_EQL_((p), NULL)
#define  AS_NEQ_NULL(p)     AS_NEQ_((p), NULL)
#define  AS_GTH_ZERO(n)     AS_GTH_((n), 0)
#define  AS_GEQ_ZERO(n)     AS_GEQ_((n), 0)

//       ======
//       MEMORY
//       ======

#define  Allocate(n)        _Allocate(n)   // Replacement for malloc()
#define  Free(ptr)          _Free(ptr)     // Replacement for free()
#define  MemCheck()         UNUSED(0)      // DEBUG: Leak report

//       =======
//       LOGGING
//       =======

void     Info(const char    * fmt, ...);   // Info message
void     Warning(const char * fmt, ...);   // Warning message
void     Error(const char   * fmt, ...);   // Non-recoverable error
#define  Verbose(...)       UNUSED(0)      // DEBUG: Verbose message

//       ==========
//       DEBUG MODE
//       ==========

#ifdef   DEBUG              // DEBUG MODE has multiple effects:
#undef   Allocate           // - Tracks all memory allocations
#undef   Verbose            // - Outputs verbose log messages
#undef   Assert             // - Enables runtime assertions
#undef   MemCheck           // - Prints detailed leak reports
#undef   Free

#define  Verbose(...)       _Verbose(__VA_ARGS__)
#define  Assert(exp)        _Assert((exp), #exp, __FILE__, __LINE__)
#define  Allocate(n)        _AllocateDebug((n),  __FILE__, __LINE__)
#define  Free(ptr)          _FreeDebug(ptr)
#define  MemCheck()         _MemCheck()
#endif

//       ==========
//       NETWORKING
//       ==========

typedef  struct req_s req_t;
typedef  void (*req_fun_t)(req_t *req);

int      NET_Init(int port, req_fun_t func);
void     NET_Accept(void);
void     NET_Shutdown(void);

struct   req_s {
         int   type;
         int   handle;
         char  data[MAX_REQ_LEN];
};

enum     req_type {
         T_REQ_QUERY     = 0x1,
         T_REQ_INSERT    = 0x2,
         T_REQ_DELETE    = 0x3,
         T_REQ_LIST_ALL  = 0x4,
         T_REQ_LIST_DONE = 0x5,
         T_REQ_LIST_TODO = 0x6
};

//       ==========
//       FILESYSTEM
//       ==========

int      FS_Init(void);
void     FS_AddPath(const char *path);
int      FS_Open(const char *path);
int      FS_Read(int fd, void *dest, int n);
int      FS_Write(int fd, const void *src, int n);
void     FS_Close(int handle);
void     FS_Shutdown(void);

//       =========
//       HASHTABLE
//       =========

typedef  u32 (*ht_fun_t)(const char *);
typedef  struct ht_tab_s ht_tab_t;
typedef  struct ht_ent_s ht_ent_t;

int      Hash_Init(ht_tab_t *tab);
void     Hash_Free(ht_tab_t *tab);
int      Hash_Insert(ht_tab_t *tab, const char *key);
int      Hash_Exists(ht_tab_t *tab, const char *key);
int      Hash_Delete(ht_tab_t *tab, const char *key);

struct   ht_tab_s {
         ht_ent_t  ** table;
         ht_fun_t     func;
         int          size;
};

struct   ht_ent_s {
         ht_ent_t   * next;
         const char * key;
};

//       ===========
//       COMMANDLINE
//       ===========

typedef  struct arg_s arg_t;

void     CMD_Parse(int argc, char **argv);
int      CMD_Next(arg_t *arg);

struct   arg_s {
         int type;
         union {
         	long     num;
         	char     str[MAX_ARG_LEN];
         } as;
};

enum     arg_type_e {
         T_ARG_PORT,
         T_ARG_PATH
};

//       ========
//       INTERNAL
//       ========

// Internal helper functions - do not use these directly.
// Use the macros without prefixed underscore instead.

void *   _Allocate(int size);
void *   _AllocateDebug(int size, const char *file, int line);
void     _Assert(bool exp, const char *text, const char *file, int line);
void     _Message(const char *pre, const char *fmt, va_list arg);
void     _Verbose(const char *fmt, ...);
void     _Free(void *ptr);
void     _FreeDebug(void *ptr);
void     _MemCheck(void);

#endif // COMMON_H
