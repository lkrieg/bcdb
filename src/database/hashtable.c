#include "common.h"
#include "database.h"
#include "hashtable.h"
#include <string.h>

static int Hash(const char *key)
{
	Assert(key != NULL);

	UNUSED(key);

	return 0;
}

int Table_Init(table_t *tab, int hashsize)
{
	int size;
	entry_t **data;

	Assert(tab != NULL);

	size = hashsize * (sizeof(*data));
	data = Allocate(size);

	if (data == NULL)
		return -1;

	memset(data, 0, size);
	tab->entries   = data;
	tab->hashsize  = hashsize;
	tab->count     = 0;

	return 0;
}

long Table_Insert(table_t *tab, const entry_t *ent)
{
	int hash;

	Assert(ent != NULL);
	Assert(ent->key != NULL);

	hash = Hash(ent->key);

	UNUSED(tab);
	UNUSED(ent);
	UNUSED(hash);

	return 0;
}

int Table_Lookup(const table_t *tab, const char *key, entry_t *ent)
{
	Assert(tab != NULL);
	Assert(key != NULL);
	Assert(ent != NULL);

	return 0;
}

int Table_Delete(table_t *tab, const char *key)
{
	Assert(tab != NULL);
	Assert(key != NULL);

	UNUSED(tab);
	UNUSED(key);

	return 0;
}

void Table_Serialize(const table_t *tab, int fd)
{
	Assert(tab != NULL);

	UNUSED(tab);
	UNUSED(fd);
}

void Table_Free(table_t *tab)
{
	// free(tab->data);
	UNUSED(tab);
}
