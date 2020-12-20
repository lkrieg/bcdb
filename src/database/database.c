#include "common.h"
#include "database.h"
#include "filesystem.h"
#include "hashtable.h"

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static table_t tab;
static bool active;
static char cache[MAX_CACHE];
static int cachesize;
static time_t t, updated;

static int CacheTable(void);

int DAT_Init(void)
{
	Info("Initializing database module...");
	if (Table_Init(&tab, MAX_HASH) < 0)
		return -1;

	t = time(NULL);
	active = true;
	return 0;
}

int DAT_Import(const char *path)
{
	Assert(active);
	Assert(path != NULL);

	Info("Importing data file '%s'...", path);
	if (FS_LoadCSV(path) < 0) {
		Warning(E_GETCSV);
		return -1;
	}

	if (CacheTable() < 0)
		return -1;

	return 0;
}

int DAT_Lookup(const char *cat, const char *bar)
{
	entry_t *ent;

	Assert(active);
	Assert(cat != NULL);
	Assert(bar != NULL);

	Verbose("Looking up '%s' in database...", bar);
	if (Table_Lookup(&tab, bar, &ent)) {
		if (ent->status != T_DAT_SCANNED) {
			ent->status = T_DAT_SCANNED;
			CacheTable();
		}
	}

	// TODO: Store invalid and unknown scans
	// TODO: Improve caching efficiency

	UNUSED(cat);
	return 0;
}

int DAT_GetCache(const char **out)
{
	*out = cache;
	return cachesize;
}

long DAT_GetTime(void)
{
	Assert(active);

	if (!updated)
		return 0;

	return (long) updated - t;
}

int DAT_Insert(const char *key, const entry_t *ent)
{
	Assert(active);
	Assert(ent != NULL);

	Verbose("Loading %s (%s)...", ent->bar, ent->dst);
	return Table_Insert(&tab, key, ent);
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

	Info("Shutting down database module...");
	Table_Serialize(&tab, -1);
	Table_Free(&tab);
}

static int CacheTable(void)
{
	int i, n, total;
	entry_t *ent;

	// Placeholder function so that the server
	// can send some data to the front end

	// TODO: Structure for quick access
	// by two keys: barcode and category

	// FIXME: Table_Delete does not allow us
	// to do serial access like below

	// FIXME: Cache must be quickly updatable
	// when the status changes. Everything below
	// is just inefficient

	if (tab.numentries == 0)
		return 0;

	total = 0;
	cachesize = 0;
	cache[total++] = '[';
	cache[total++] = '\n';

	updated = time(NULL);
	Verbose("Rebuilding database cache...");
	for (i = 0; i < tab.numentries; i++) {
		ent = tab.data + i;
		n = snprintf(cache + total, MAX_CACHE - total,
		             "{\n\t\"barcode\" :\"%s\",\n"
		             "\t\"loadnum\": \"%s\",\n"
		             "\t\"recipient\": \"%s\",\n"
		             "\t\"status\": %d\n"
		             "}%s",
		             ent->key, ent->cat,
		             ent->dst, ent->status,
		             (i+1 == tab.numentries) ? "\n]" : ",\n");

		if (n < 0)
			return -1;

		if (n >= MAX_CACHE - total)
			return -2;

		total += n;
	}

	cachesize = total;
	return 0;
}
