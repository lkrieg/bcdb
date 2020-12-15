#include "common.h"
#include "filesystem.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int FS_ReadRAM(const char *path, char *out, int n)
{
	int fd, m = 0;
	int total = 0;

	Assert(path != NULL);
	Assert(out != NULL);

	fd = open(path, O_RDONLY);

	if (fd < 0) {
		Warning(E_FDOPEN " '%s'", path);
		return -1;
	}

	do {
		if ((m && n == total)
		|| ((m = read(fd, out + total, n - total)) < 0)) {
			Warning(E_NOREAD " '%s'", path);
			return -2;
		}

		total += m;
	} while (m);

	close(fd);
	return total;
}

int FS_ParseCSV(const char *path, csv_t *out)
{
	UNUSED(path);
	UNUSED(out);

	return -1;
}
