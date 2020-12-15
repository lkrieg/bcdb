#include "common.h"
#include "filesystem.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static int SkipWhitespace(char **buf);

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

int FS_LoadCSV(const char *path, csv_t *out)
{
	bool quoted;
	int n, len;
	char buf[MAX_FILEBUF + 1];
	char *tail, *head = buf;

	if ((n = FS_ReadRAM(path, buf, MAX_FILEBUF)) < 0)
		return -1;

	buf[n] = '\0';
	quoted = false;

	while (SkipWhitespace(&head)) {
		for (tail = head;; tail++) {
			if (*tail == '\0')
				break;

			if (quoted)
				continue;

			if (*tail == '"')
				quoted = !quoted;

			if ((*tail == ',')
			|| ((*tail == ';') || (*tail == '\n'))) {
				tail++;
				break;
			}
		}

		if ((len = tail - head - 1) > 0)
			Verbose("Found value '%.*s'", len, head);

		head = tail;
		UNUSED(out);
	}

	if (quoted) {
		Warning(E_NQUOTE);
		return -1;
	}

	return 0;
}

// FIXME: Duplication with params.c
static int SkipWhitespace(char **buf)
{
	char *head = *buf;

	Assert(buf && *buf);

	while (*head != '\0') {
		if (*head <= ' ') {
			head++;
			continue;
		}

		if (*head != '#')
			break; // Done

		// Comment
		while (*head != '\n') {
			if (*head == '\0')
				break;
			head++;
		}
	}

	*buf = head;
	return (*head != '\0');
}
