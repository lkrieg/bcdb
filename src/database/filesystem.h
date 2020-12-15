#ifndef FILESYSTEM_H
#define FILESYSTEM_H

typedef struct csv_row_s csv_row_t;

int FS_ReadRAM(const char *path, char *out, int n);
csv_row_t *FS_LoadCSV(const char *path);

struct csv_row_s
{
	short        cols[MAX_CSV_COLS];
	char         data[MAX_CSV_DATA];
	int          size;
	csv_row_t *  next;
};

#endif // FILESYSTEM_H
