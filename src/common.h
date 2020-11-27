#ifndef  COMMON_H
#define  COMMON_H

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

//       Constants
#define  DEBUG                           // Enable debug mode
#define  DEFAULT_PORT      23            // Default server socket port
#define  MAX_MSG_LEN       2048          // Maximum log message length
#define  MAX_HASH_SIZE     4096          // Maximum number of hash buckets
#define  MAX_PATH          4096          // Cannot rely on system header

//       Messages
#define  E_ASSERT          "Assertion failure"
#define  E_NOMEM           "Memory allocation failure"
#define  E_NOSOCK          "Could not bind to socket"

//       Utility
#define  BIT(n)            (1UL << (n))  // Get bitmask for nth bit
#define  UNUSED(sym)       ((void)(sym)) // Suppress -Wunused warnings
#define  Assert(exp)       UNUSED(0)     // Check runtime assertion

//       Memory
#define  Allocate(n)       _Allocate(n)  // Replacement for malloc()
#define  Free(ptr)         _Free(ptr)    // Replacement for free()

//       Logging
void     Info (const char  *fmt, ...);   // Output general info message
void     Error(const char  *fmt, ...);   // Handle non-recoverable failure
#define  Verbose(...)      UNUSED(0)     // Output debug info message

//       Filesystem
int      FS_Open(const char *path);
int      FS_Read(int fd, void *dest, int n);
int      FS_Write(int fd, const void *src, int n);
void     FS_Close(int handle);

//       Typedefs
typedef  uint32_t          u32;          // Guaranteed 32 bit data type
typedef  uint16_t          u16;          // Guarenteed 16 bit data type
typedef  unsigned char     byte;         // uint8_t could be a non-character
                                         //   type and break strict aliasing
//       Debug
#ifdef   DEBUG                           // DEBUG mode has multiple effects:
#undef   Allocate                        // - Tracks all memory allocations
#undef   Verbose                         // - Outputs verbose log messages
#undef   Assert                          // - Enables runtime assertions
#undef   Free

#define  Verbose(...)     _Verbose(__VA_ARGS__)
#define  Assert(exp)      _Assert(exp, #exp, __FILE__, __LINE__)
#define  Allocate(n)      _AllocateDebug( n, __FILE__, __LINE__)
#define  Free(ptr)        _FreeDebug(ptr)
#endif

// Internal helper functions - do not use these directly!
// Use the macro versions without the prefixed underscore.

void *   _Allocate(int size);
void *   _AllocateDebug(int size, const char *file, int line);
void     _Assert(bool exp, const char *text, const char *file, int line);
void     _Message(const char *pre, const char *fmt, va_list arg);
void     _Verbose(const char *fmt, ...);
void     _Free(void *ptr);
void     _FreeDebug(void *ptr);

#endif // COMMON_H
