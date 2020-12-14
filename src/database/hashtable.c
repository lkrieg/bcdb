#include "common.h"
#include "database.h"
#include "hashtable.h"
#include <string.h>
#include <limits.h>

#define FNV_32_BASIS 0x811c9dc5
#define FNV_32_PRIME 0x01000193

static int Hash(const char *key)
{
	unsigned long hash;

	Assert(key != NULL);
	hash = FNV_32_BASIS;

	while (*key) {
		hash ^= *key++;
		hash *= FNV_32_PRIME;
	}

	return hash % INT_MAX;
}

int Table_Init(table_t *tab, int hashsize)
{
	int n, ndat, nent;
	entry_t ** entries;
	entry_t * data;

	Assert(tab != NULL);

	n     = hashsize;
	ndat  = n * sizeof(*data);
	nent  = n * sizeof(*entries);

	data     = Allocate(ndat);
	entries  = Allocate(nent);

	if (!data || !entries)
		return -1; // Free?

	memset(data, 0, ndat);

	tab->hashsize    = n;
	tab->capacity    = n;
	tab->numentries  = 0;
	tab->entries     = entries;
	tab->data        = data;

	return 0;
}

long Table_Insert(table_t *tab, const entry_t *ent)
{
	int hash;

	Assert(ent != NULL);
	Assert(ent->key != NULL);

	if (tab->numentries == tab->capacity) {
		Verbose("Resizing hashtable...");
		return -1; // TODO
	}

	hash = Hash(ent->key);

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
