#ifndef  COMMON_H
#define  COMMON_H

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

#define  DEBUG                           // Enable debug mode
#define  DEFAULT_PORT     23             // Default server socket port
#define  MAX_MSG_LEN      2048           // Maximum log message length
#define  MAX_HASH_SIZE    4096           // Maximum number of hash buckets
#define  MAX_PATH         4096           // Cannot rely on system header

#define  E_ASSERT         "Assertion failure"
#define  E_NOMEM          "Memory allocation failure: Out of memory"
#define  E_NOSOCK         "Could not bind to socket"

#define  BIT(n)           (1UL << (n))   // Get bitmask for nth bit
#define  UNUSED(sym)      ((void)(sym))  // Suppress unused symbol warnings
#define  Allocate(n)      _Allocate(n)   // Replacement for malloc()
#define  Free(ptr)        _Free(ptr)     // Replacement for free()
#define  Assert(exp)      UNUSED(0)      // Check runtime assertion
#define  Verbose(...)     UNUSED(0)      // Output debug info message

void     Info (const char *fmt, ...);    // Output general info message
void     Error(const char *fmt, ...);    // Non-recoverable failure

typedef  uint32_t         u32;           // Guaranteed 32 bit data type
typedef  uint16_t         u16;           // Guarenteed 16 bit data type
typedef  unsigned char    byte;          // uint8_t could be a non-character
                                         //   type and break strict aliasing

#ifdef   DEBUG            // DEBUG mode has multiple effects:
#undef   Allocate         // - Tracks all memory allocations
#undef   Free             // - Outputs verbose log messages
#undef   Assert           // - Enables runtime assertions
#undef   Verbose

#define  Allocate(n)      _AllocateDebug(n, __FILE__, __LINE__)
#define  Free(ptr)        _FreeDebug(ptr)
#define  Assert(exp)      _Assert(exp, #exp, __FILE__, __LINE__)
#define  Verbose(...)     _Verbose(__VA_ARGS__)
#endif

// Internal helper functions - do not use these directly!
// Use the macro versions without the prefixed underscore.

void *   _Allocate(int size);
void *   _AllocateDebug(int size, const char *file, int line);
void     _Assert(bool exp, const char *text, const char *file, int line);
void     _Verbose(const char *fmt, ...);
void     _Free(void *ptr);
void     _FreeDebug(void *ptr);

#endif // COMMON_H
