#include "common.h"
#include <stdlib.h>
#include <stdio.h>

void *_Allocate(int size)
{
	char *ptr;

	ptr = malloc(size);

	// We cannot really handle out
	// of memory errors gracefully

	if (ptr == NULL)
		Error(E_NOMEM);

	return ptr;
}

void *_AllocateDebug(int size, const char *file, int line)
{
	UNUSED(file);
	UNUSED(line);

	return _Allocate(size);
}

void _Free(void *ptr)
{
	free(ptr);
}

void _FreeDebug(void *ptr)
{
	_Free(ptr);
}

void _Assert(bool exp, const char *text, const char *file, int line)
{
	if (exp)
		return;

	Error(E_ASSERT": %s (%s:%d)",
	      text, file, line);
}

void _Message(const char *pre, const char *fmt, va_list arg)
{
	static char msg[MAX_MSG_LEN];

	vsnprintf(msg, MAX_MSG_LEN, fmt, arg);
	printf("%s: %s.\n", pre, msg);
	fflush(stdout);
}

void _Verbose(const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	_Message("Verbose", fmt, arg);
	va_end(arg);
}

void Info(const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	_Message("Info", fmt, arg);
	va_end(arg);
}

void Error(const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	_Message("Error", fmt, arg);
	va_end(arg);

	// Non-recoverable
	exit(EXIT_FAILURE);
}
