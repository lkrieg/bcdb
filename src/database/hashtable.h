#ifndef HASHTABLE_H
#define HASHTABLE_H

typedef struct table_s table_t;

struct table_s
{
	entry_t **  entries;
	int         hashsize;
	int         count;
};

int   Table_Init(table_t *tab, int hashsize);
long  Table_Insert(table_t *tab, const entry_t *ent);
int   Table_Lookup(const table_t *tab, const char *key, entry_t *ent);
int   Table_Delete(table_t *tab, const char *key);
void  Table_Serialize(const table_t *tab, int fd);
void  Table_Free(table_t *tab);

#endif // HASHTABLE_H
