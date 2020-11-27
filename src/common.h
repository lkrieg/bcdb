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
#define  MAX_ARG_LEN        1024   // Maxmimum length per command line argument
#define  MAX_MSG_LEN        2048   // Maximum message length
#define  MAX_HASH_SIZE      4096   // Maximum hash size

//       ========
//       TYPEDEFS
//       ========

typedef  uint32_t           u32;   // Guaranteed 32 bit data type
typedef  uint16_t           u16;   // Guaranteed 16 bit data type
typedef  unsigned char      byte;  // uint8_t could be a non-character
                                   //   type and therefore break aliasing
//       ==============
//       ERROR MESSAGES
//       ==============

#define  E_ASSERT           "Assertion failure"
#define  E_NOMEM            "Memory allocation failure"
#define  E_NOSOCK           "Could not bind to network socket"
#define  E_FSINIT           "Could not initialize filesystem"
#define  E_ARGVAL           "Unknown command line argument"

//       =======
//       UTILITY
//       =======

#define  BIT(n)             (1UL << (n))   // Get bitmask for nth bit
#define  UNUSED(sym)        ((void)(sym))  // Suppress -Wunused warnings
#define  Assert(exp)        UNUSED(0)      // Check runtime assertion

//       ======
//       MEMORY
//       ======

#define  Allocate(n)        _Allocate(n)   // Replacement for malloc()
#define  Free(ptr)          _Free(ptr)     // Replacement for free()

//       =======
//       LOGGING
//       =======

void     Info (const char * fmt, ...);     // Output general info message
void     Error(const char * fmt, ...);     // Handle non-recoverable failure
#define  Verbose(...)       UNUSED(0)      // Output debug info message

//       ==========
//       DEBUG MODE
//       ==========

#ifdef   DEBUG              // DEBUG MODE has multiple effects:
#undef   Allocate           // - Tracks all memory allocations
#undef   Verbose            // - Outputs verbose log messages
#undef   Assert             // - Enables runtime assertions
#undef   Free

#define  Verbose(...)       _Verbose(__VA_ARGS__)
#define  Assert(exp)        _Assert(exp, #exp, __FILE__, __LINE__)
#define  Allocate(n)        _AllocateDebug( n, __FILE__, __LINE__)
#define  Free(ptr)          _FreeDebug(ptr)
#endif

//       ==========
//       NETWORKING
//       ==========

int      NET_Init(void);                               // Bind to network socket
void     NET_Shutdown(void);                           // Unbind from network socket

//       ==========
//       FILESYSTEM
//       ==========

int      FS_Init(void);                                // Initialize filesystem
int      FS_Open(const char *path);                    // Get writable file handle
int      FS_Read(int fd, void *dest, int n);           // Read n bytes from handle
int      FS_Write(int fd, const void *src, int n);     // Write n bytes to handle
void     FS_Close(int handle);                         // Close file handle
void     FS_Shutdown(void);                            // Shutdown filesystem

//       =========
//       HASHTABLE
//       =========
                                                       // Hashtable typedefs:
typedef  u32 (*ht_fun_t)(const char *);                // - Hashing function
typedef  struct ht_tab_s ht_tab_t;                     // - Table structure
typedef  struct ht_ent_s ht_ent_t;                     // - Entry structure

int      Hash_Init(ht_tab_t *tab);                     // Initialize hashtable
void     Hash_Free(ht_tab_t *tab);                     // Free all table entries
int      Hash_Insert(ht_tab_t *tab, const char *key);  // Insert new key into table
int      Hash_Exists(ht_tab_t *tab, const char *key);  // Check if key already exists
int      Hash_Delete(ht_tab_t *tab, const char *key);  // Delete key from table

struct   ht_tab_s {                                    // Hashtable structure:
         ht_ent_t  ** table;                           // - Table entries
	 ht_fun_t     func;                            // - Hashing function
         int          size;                            // - Maximum hash size
};

struct   ht_ent_s {                                    // Entry node structure:
         ht_ent_t   * next;                            // - Next entry node
         const char * key;                             // - Entry key value
};

//       ===========
//       COMMANDLINE
//       ===========

typedef  struct arg_s arg_t;                           // Argument value typedef
void     CMD_Parse(int argc, char **argv);             // Parse command line arguments
int      CMD_Get(arg_t *arg);                          // Get argument type and value

struct   arg_s {
         int type;                                     // Argument type, see arg_type_e
         int len;                                      // Length for string values
         union {                                       // Argument value union:
         	long     num;                          // - Numeric argument values
         	char     str[MAX_ARG_LEN];             // - String argument values
         } val;
};

enum     arg_type_e {                                  // Argument type enumeration:
         C_SETPORT = 1,                                // - Overwrite default server port
         C_VERBOSE                                     // - Enable verbose log messages
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

#endif // COMMON_H
