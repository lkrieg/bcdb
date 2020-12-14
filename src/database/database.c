#include "common.h"
#include "database.h"
#include "filesystem.h"
#include "hashtable.h"

static table_t tab;
static bool active;

int DAT_Init(void)
{
	Info("Initializing barcode database...");
	if (Table_Init(&tab, MAX_HASH) < 0)
		return -1;

	active = true;
	return 0;
}

int DAT_Query(const char *key, entry_t *out)
{
	Assert(active);
	Assert(key != NULL);
	Assert(out != NULL);

	Verbose("Querying entry '%s'", key);
	return Table_Lookup(&tab, key, out);
}

int DAT_Insert(const entry_t *ent)
{
	Assert(active);
	Assert(ent != NULL);

	Verbose("Inserting entry '%s'", ent->key);
	return Table_Insert(&tab, ent);
}

int DAT_Delete(const char *key)
{
	Assert(active);
	Assert(key != NULL);

	Verbose("Deleting entry '%s'", key);
	return Table_Delete(&tab, key);
}

void DAT_Shutdown(void)
{
	Assert(active);

	Info("Shutting down database...");
	Table_Serialize(&tab, -1);
	Table_Free(&tab);
}
