#include "common.h"
#include <string.h>

// TODO: Insert/Get/Delete MUST be thread safe!

#define FNV_BASIS_32 0x811C9DC5
#define FNV_PRIME_32 0x01000193

static u32 GetHash(const char *key);

int Hash_Init(ht_tab_t *tab)
{
	int total;

	tab->func   = GetHash;
	tab->active = NULL;
	tab->size   = MAX_HASH_SIZE;
	total       = tab->size * sizeof(*(tab->table));

	tab->table  = Allocate(total);
	memset(tab->table, 0, total);

	return 0;
}

int Hash_Insert(ht_tab_t *tab, const char *key)
{
	u32 hash;
	ht_ent_t *tmp, **it;

	AS_NEQ_NULL(tab);
	AS_NEQ_NULL(key);

	if (strlen(key) >= MAX_KEY_LEN)
		return -1;

	if (Hash_Get(tab, key))
		return -2;

	tmp  = Allocate(sizeof(ht_ent_t));
	hash = tab->func(key) % tab->size;
	it   = &tab->table[hash];

	while (*it != NULL)
		it = &(*it)->next;

	strcpy(tmp->key, key);
	tmp->status = T_ENT_TODO;
	tmp->next = NULL;
	*it = tmp;

	return hash;
}

ht_ent_t *Hash_Get(ht_tab_t *tab, const char *key)
{
	u32 hash;
	ht_ent_t *it;

	AS_NEQ_NULL(tab);
	AS_NEQ_NULL(key);

	hash = tab->func(key) % tab->size;
	it   = tab->table[hash];

	for (; it; it = it->next) {
		if (!strcmp(key, it->key))
			return it;
	}

	return NULL;
}

// TODO: Thread-safety
int Hash_Delete(ht_tab_t *tab, const char *key)
{
	UNUSED(tab);
	UNUSED(key);

	return -1;
}

void Hash_Free(ht_tab_t *tab)
{
	UNUSED(tab);
}

static u32 GetHash(const char *key)
{
	u32 hash;

	AS_NEQ_NULL(key);

	hash = FNV_BASIS_32;

	while (*key) {
		hash ^= *key++;
		hash *= FNV_PRIME_32;
	}

	return hash;
}
