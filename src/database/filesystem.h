#ifndef FILESYSTEM_H
#define FILESYSTEM_H

typedef struct csv_s csv_t;

int FS_ReadRAM(const char *path, char *out, int n);
int FS_LoadCSV(const char *path, csv_t *out);

struct csv_s
{
	int     size;
};

#endif // FILESYSTEM_H
