#ifndef DATABASE_H
#define DATABASE_H

typedef struct entry_s entry_t;

int DB_Init(void);
int DB_Query(const char *key, entry_t *out);
int DB_Insert(const entry_t *ent);
int DB_Delete(int index);

struct entry_s
{
	const char    key[MAX_KEY_LEN];
	entry_t    *  next;
};

#endif // DATABASE_H
