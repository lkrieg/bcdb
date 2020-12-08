#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <signal.h>

#define DEBUG

typedef uint32_t        u32;
typedef uint16_t        u16;
typedef unsigned char   byte;

#define MAX_PATH        4096
#define MAX_MSG_LEN     2048
#define MAX_ADR_LEN     46
#define MAX_BACKLOG     10
#define MIN_ROW_NUM     24
#define MIN_COL_NUM     14
#define MAX_VTT_LEN     256

#define BIT(n)          (1UL << (n))
#define UNUSED(sym)     ((void)(sym))
#define ARRAYLEN(a)     (sizeof(a)/sizeof(*(a)))

#define Info(...)       _printl("Info: "    __VA_ARGS__)
#define Warning(...)    _printl("Warning: " __VA_ARGS__)
#define Error(...)      _perror("Error: "   __VA_ARGS__)
#define Verbose(...)    UNUSED(0) // Used in DEBUG mode
#define Assert(exp)     UNUSED(0) // Used in DEBUG mode

#define Allocate(n)     _malloc(n)
#define Free(ptr)       _free(ptr)

#define E_ASSERT        "Assertion failed"
#define E_NOMEM         "Allocation failure: Out of memory"
#define E_GETADR        "Unable to get socket address"
#define E_NOSOCK        "Unable to get socket descriptor"
#define E_NOBIND        "Unable to bind to socket"
#define E_SETOPT        "Unable to set socket option"
#define E_LISTEN        "Unable to listen from socket"
#define E_INITIP        "Network initialization failed"
#define E_SIGNAL        "Unable to create signal handlers"
#define E_ACCEPT        "Unable to accept client"
#define E_NOFORK        "Unable to fork process"
#define E_TXDATA        "Unable to transmit data"
#define E_RXDATA        "Unable to receive data"
#define E_ERRSUB        "Invalid subnegotiation"
#define M_INITIP        "Initializing network"
#define M_SOCKFD        "Getting socket descriptor"
#define M_SETOPT        "Setting socket option"
#define M_BINDFD        "Binding to socket descriptor"
#define M_LISTEN        "Listening for connections"
#define M_ACCEPT        "Accepting new client"
#define M_SETTLE        "Client sent negotiation"
#define M_ANSWER        "Answering negotiation"
#define M_RXDATA        "Client sent data"
#define M_CLOSED        "Disconnected from client"
#define M_NEWSUB        "Client subnegotiates"
#define M_SUBNAW        "Client window size changed"
#define M_SUBTTY        "Client terminal type changed"
#define M_CLNINT        "Client interrupt received"
#define M_EXIT          "Shutting down subsystems"

#define AS_NEQ(l, r)    Assert((l) != (r))
#define AS_GEQ(l, r)    Assert((l) >= (r))
#define AS_NEQ_NULL(e)  AS_NEQ((e), NULL)
#define AS_GEQ_ZERO(e)  AS_GEQ((e), 0)

#define _malloc(n)      malloc(n) // Put non-debug wrapper here
#define _free(ptr)      free(ptr) // Put non-debug wrapper here
#define _assert(exp)    if (!(exp)) Error(E_ASSERT ": %s " \
			"(%s:%d)", #exp, __FILE__, __LINE__)

#define _perror(...)    do { _printl(__VA_ARGS__); _fatal(); } while(0)
#define _printl(...)    do {  printf(__VA_ARGS__); _eolff(); } while(0)
#define _print(...)     do {  printf(__VA_ARGS__); _flush(); } while(0)
#define _eolff()        putchar('\n'); _flush()
#define _flush()        fflush(stdout);
#define _fatal()        exit(EXIT_FAILURE)

#ifdef  DEBUG           // DEBUG mode
#undef  Assert          // Enable runtime assertions
#undef  Verbose         // Output verbose log messages
#undef  Allocate        // Track all memory allocations
#undef  Free            // Enable detailed leak reports

#define Assert(exp)     _assert(exp)
#define Verbose(...)    _print(__VA_ARGS__)
#define Allocate(n)     _malloc(n) // Put debug wrapper here
#define Free(ptr)       _free(ptr) // Put debug wrapper here
#endif

#endif // COMMON_H
