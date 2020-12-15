#ifndef FILESYSTEM_H
#define FILESYSTEM_H

int FS_ReadRAM(const char *path, char *out, int n);
int FS_LoadCSV(const char *path);

#endif // FILESYSTEM_H
