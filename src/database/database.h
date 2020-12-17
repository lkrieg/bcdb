#ifndef DATABASE_H
#define DATABASE_H

typedef struct entry_s entry_t;

int   DAT_Init(void);
int   DAT_Import(const char *path);
int   DAT_Query(const char *key, entry_t *out);
int   DAT_Insert(const char *key, const entry_t *ent);
int   DAT_Delete(const char *key);
void  DAT_Shutdown(void);

struct entry_s
{
	char    *  key;
	char       cat[MAX_CATEGORY];
	char       bar[MAX_BARCODE];
	char       dst[MAX_DEST];
	entry_t *  next;
};

#endif // DATABASE_H
