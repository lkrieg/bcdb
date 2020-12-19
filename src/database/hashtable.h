#ifndef HASHTABLE_H
#define HASHTABLE_H

typedef struct table_s table_t;

struct table_s
{
	int         hashsize;
	int         capacity;
	int         numentries;
	entry_t **  entries;
	entry_t  *  data;
};

int   Table_Init(table_t *tab, int hashsize);
long  Table_Insert(table_t *tab, const char *key, const entry_t *ent);
int   Table_Lookup(const table_t *tab, const char *key, entry_t **out);
int   Table_Delete(table_t *tab, const char *key);
void  Table_Serialize(const table_t *tab, int fd);
void  Table_Free(table_t *tab);

#endif // HASHTABLE_H
