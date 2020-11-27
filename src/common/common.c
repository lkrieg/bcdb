#include "common.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct mem_block_s mem_block_t;
static mem_block_t *FindBlock(void *data);
static mem_block_t *mem;

struct mem_block_s {
	int           size;
	void        * data;
	int           line;
	const char  * file;
	mem_block_t * next;
};

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
	mem_block_t *block;

	block = _Allocate(sizeof(*block));
	block->data = _Allocate(size);
	block->size = size;
	block->line = line;
	block->file = file;
	block->next = mem;
	mem = block;

	return block->data;
}

void _Free(void *ptr)
{
	free(ptr);
}

void _FreeDebug(void *ptr)
{
	mem_block_t *block;

	// TODO: Unlink block
	block = FindBlock(ptr);
	_Free(block->data);
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

void CMD_Parse(int argc, char **argv)
{
	UNUSED(argc);
	UNUSED(argv);
}

int CMD_Get(arg_t *arg)
{
	UNUSED(arg);
	return -1;
}

static mem_block_t *FindBlock(void *data)
{
	UNUSED(data);
	return NULL;
}
