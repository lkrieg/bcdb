#include "common.h"

void Info(const char * fmt, ...)
{
	UNUSED(fmt);
}

void Warning(const char * fmt, ...)
{
	UNUSED(fmt);
}

void Error(const char * fmt, ...)
{
	UNUSED(fmt);
}

void _Verbose(const char *fmt, ...)
{
	UNUSED(fmt);
}

void * _Allocate(int size)
{
	UNUSED(size);
	return NULL;
}

void * _AllocateDebug(int size, const char *file, int line)
{
	UNUSED(size);
	UNUSED(file);
	UNUSED(line);
	return NULL;
}

void _Assert(bool exp, const char *text, const char *file, int line)
{
	UNUSED(exp);
	UNUSED(text);
	UNUSED(file);
	UNUSED(line);
}

void _Free(void *ptr)
{
	UNUSED(ptr);
}

void _FreeDebug(void *ptr)
{
	UNUSED(ptr);
}

void _MemCheck(void)
{
}
