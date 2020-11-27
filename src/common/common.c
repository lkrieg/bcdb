#include "common.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct mem_block_s mem_block_t;
static mem_block_t *FindBlock(void *data);
static mem_block_t *mem;

struct mem_block_s {
	int           size;
	void        * data;
	const char  * file;
	int           line;

	mem_block_t * prev;
	mem_block_t * next;
};

void *_Allocate(int size)
{
	char *ptr;

	ptr = malloc(size);

	// We cannot really gracefully
	// handle out of memory errors

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
	block->file = file;
	block->line = line;
	block->prev = NULL;
	block->next = mem;
	mem = block;

	if (mem->next != NULL)
		mem->next->prev = mem;

	return block->data;
}

void _Free(void *ptr)
{
	free(ptr);
}

void _FreeDebug(void *ptr)
{
	mem_block_t *block;
	mem_block_t *prev, *next;

	block = FindBlock(ptr);
	if (block == NULL)
		return;

	prev = block->prev;
	next = block->next;

	_Free(block->data);
	_Free(block);

	if (prev == NULL) {
		mem = next;
		if (next != NULL)
			next->prev = NULL;
		return;
	}

	prev->next = next;
	next->prev = prev;
}

void _MemCheck(void)
{
	unsigned long total;
	mem_block_t *block;

	total = 0;
	block = mem;

	if (!block)
		return;

	do {
		total += block->size;
		block  = block->next;
	} while (block);


	Info("Total memory in use: %d bytes", total);
	Info("Showing all active memory blocks:");

	for (block = mem; block; block = block->next)
		Info("\t%d bytes in %s:%d", block->size,
		     block->file, block->line);
}

static mem_block_t *FindBlock(void *data)
{
	mem_block_t *block = mem;

	while (block && block->data != data)
		block = block->next;

	return block;
}

void _Assert(bool exp, const char *text, const char *file, int line)
{
	if (exp == false)
		Error(E_ASSERT": %s (%s:%d)",
		      text, file, line);
}

void _Message(const char *pre, const char *fmt, va_list arg)
{
	static char msg[MAX_MSG_LEN];

	vsnprintf(msg, MAX_MSG_LEN, fmt, arg);
	printf("%s: %s\n", pre, msg);
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
	return 0;
}
