#include "common.h"

int FS_Init(void)
{
	return -1;
}

int FS_Open(const char *path)
{
	UNUSED(path);

	return -1;
}

int FS_Read(int fd, void *dest, int n)
{
	UNUSED(fd);
	UNUSED(dest);
	UNUSED(n);

	return -1;
}

int FS_Write(int fd, const void *src, int n)
{
	UNUSED(fd);
	UNUSED(src);
	UNUSED(n);

	return -1;
}

void FS_Close(int handle)
{
	UNUSED(handle);
}

void FS_Shutdown(void)
{

}
