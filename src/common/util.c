#include "common.h"
#include <stdlib.h>
#include <stdio.h>

enum log_levels {
	T_LOG_INFO,
	T_LOG_VERBOSE,
	T_LOG_WARNING,
	T_LOG_ERROR,
	T_LOG_NONE
};

static const char *prefixes[] = {
	"[INFO] ", "[DEBUG] ",
	"[WARN] ", "[ERROR] ",
	""
};

static void Log(int level, const char *fmt, va_list arg)
{
	char msg[MAX_LINEBUF];

	Assert(fmt != NULL);

	if (level < 0 || level > 4)
		level = T_LOG_NONE;

	vsnprintf(msg, MAX_LINEBUF, fmt, arg);
	printf("%s%s\n", prefixes[level], msg);

	fflush(stdout);
}

void Print(const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	Log(T_LOG_NONE, fmt, arg);
	va_end(arg);
}

void Info(const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	Log(T_LOG_INFO, fmt, arg);
	va_end(arg);
}

void Warning(const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	Log(T_LOG_WARNING, fmt, arg);
	va_end(arg);
}

void Error(const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	Log(T_LOG_ERROR, fmt, arg);
	va_end(arg);

	// Critical failure
	exit(EXIT_FAILURE);
}

void _Verbose(const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	Log(T_LOG_VERBOSE, fmt, arg);
	va_end(arg);
}

void _Assert(int exp, const char *text, const char *file, int line)
{
	if (exp == 0)
		Error(E_ASSERT ": '%s' (%s:%d)", text, file, line);
}

void *_Allocate(int size)
{
	void *ptr;

	// There is no way of safely recovering from out of memory
	// errors, but things have to go total haywire for that. The
	// Linux kernel will do things like disk swapping to prevent
	// the worst case from ever happening.

	ptr = malloc(size);

	if (ptr == NULL)
		Error(E_NOMEM);

	return ptr;
}

void *_AllocateDebug(int size, const char *file, int line)
{
	// TODO
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
	// TODO
	_Free(ptr);
}

void _Memcheck(void)
{
	// TODO
	return;
}
