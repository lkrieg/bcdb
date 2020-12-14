#include "common.h"
#include "database.h"
#include "hashtable.h"

static long Hash(const char *key)
{
	Assert(key != NULL);

	UNUSED(key);

	return 0;
}

int Table_Init(table_t *tab)
{
	Assert(tab != NULL);

	UNUSED(tab);

	return 0;
}

long Table_Insert(table_t *tab, const entry_t *ent)
{
	long hash;

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
	UNUSED(tab);
}
