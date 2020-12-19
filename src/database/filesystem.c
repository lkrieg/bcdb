#include "common.h"
#include "filesystem.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
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

int FS_LoadCSV(const char *path)
{
	int fd, n, nval;
	int ncat, nbar, ndst;
	char chunk[MAX_CHUNK + 1];
	char *head, *tail, c;
	bool comment = false;
	bool quoted = false;
	entry_t ent;

	// TODO: Split into multiple functions
	// TODO: Clean up and check for problems
	// TODO: Handle escaped quotes

	ncat = nbar = ndst = nval = 0;
	if ((fd = open(path, O_RDONLY)) < 0) {
		Warning(E_FDOPEN " '%s': %s", path,
		        strerror(errno));
		return -1;
	}

	for (;;) {
		if ((n = read(fd, chunk, MAX_CHUNK)) < 0) {
			Warning(E_FDREAD " '%s': %s", path,
			        strerror(errno));
			return -1;
		}

		if (n == 0)
			break;

		head = chunk;
		tail = head + n;

		while (head < tail) {
			c = *head++;

			// Skip comment
			if (comment) {
				if (c == '\r' || c == '\n')
					comment = !comment;
				continue;
			}

			// Logical line terminator
			if ((c =='\r' || c == '\n')
			|| ((!quoted && ((c == ';') || c == '#')))) {
				if (ncat) { // Any values to insert?
					if (!nbar || !ndst) {
						Warning(E_FIELDS);
						close(fd);
						return -1;
					}

					if (quoted) {
						Warning(E_QUOTED);
						close(fd);
						return -1;
					}

					// Remove trailing whitespace
					while (isspace(ent.cat[ncat-1])) ncat--;
					while (isspace(ent.bar[nbar-1])) nbar--;
					while (isspace(ent.dst[ndst-1])) ndst--;

					ent.cat[ncat] = '\0';
					ent.bar[nbar] = '\0';
					ent.dst[ndst] = '\0';
					ent.status = T_DAT_WAITING;
					nval = ncat = nbar = ndst = 0;

					// Insert value into database
					DAT_Insert(ent.cat, &ent);
				}

				if (c == '#') // Comment
					comment = true;
				continue;
			}

			// Quoted values
			// Escaped quotes are currently not
			// supported since the syntax sucks

			if (c == '"') {
				quoted = !quoted;
				continue;
			}

			if (c == ',' && !quoted) {
				if ((nval == 0 && !ncat)
				|| ((nval == 1 && !nbar))
				|| ((nval == 2 && !ndst))) {
					Warning(E_FIELDS);
					close(fd);
					return -1;
				}
				nval++;
				continue;
			}

			if ((ncat >= MAX_CATEGORY)
			|| ((nbar >= MAX_BARCODE)
			|| ((ndst >= MAX_DEST)))) {
				Warning(E_MAXVAL);
				close(fd);
				return -1;
			}

			if (nval == 0 && (ncat || c > ' '))
				ent.cat[ncat++] = c;
			if (nval == 1 && (nbar || c > ' '))
				ent.bar[nbar++] = c;
			if (nval == 2 && (ndst || c > ' '))
				ent.dst[ndst++] = c;
		}
	}

	close(fd);
	return 0;
}

#if 0

/*
if (c == '\r' || c == '\n' || c == ';') {
	head += (c == '\r' && *head == '\n');
	if (quoted) {
		Warning(E_QUOTED);
		return -1;
	}
	continue;
}
*/

/*
if (c == '"') {
	if (!quoted || *head != '"') {
		quoted = !quoted;
		continue;
	}
	// Escaped
	head++;
}
*/

if (!quoted) {
	if (c == ',') {
	}
	if (c == ';') {
	}
			}

//if (c <= ' ')
//	continue;


FILE *fp;
char line[MAX_LINEBUF + 1];

if ((fp = fopen(path, "r")) == NULL) {
	Warning(E_FDOPEN " '%s'", path);
	return -1;
}

while ((n = getline(&line)) // ...

int FS_LoadCSV(const char *path)
{

	int cols;
	int n, max = MAX_FILEBUF;
	char buf[MAX_FILEBUF + 1];
	char *cat, *bar, *com;
	char *head = buf;
	bool quoted;
	entry_t ent;

	n = FS_ReadRAM(path, buf, max);

	if (n < 0)
		return -1;

	buf[n]  = '\0';
	quoted  = false;
	cols    = 0;

	do {
		if (!SkipWhitespace(&head))
			break;

		cat = head;
		bar = NULL;
		com = NULL;

		for (;; head++) {
			if (*head == '\n' || *head == '\0') {
				if (quoted) {
					Warning(E_QUOTED);
					return -2;
				}
				break;
			}

			if (*head == '"')
				quoted = !quoted;

			if (quoted)
				continue;

			if (*head == ',') {
				*head = '\0';
				switch (cols++) {
				case 0: bar = head + 1; break;
				case 1: com = head + 1; break;
				}
				continue;
			}

			if (*head == ';' || *head == '#')
				break;
		}

		if (!cat || !bar)
			return -1;

		if (*cat == 0 || *bar == 0)
			return -1;

		if (*head != '\0')
			*head++ = '\0';

		Info("|%s|%s|%s|", cat, bar, com);
		UNUSED(ent);

		// if (DAT_Insert(bar, &ent) < 0)
		//	return -3;

		cols = 0;

	} while (1);
	return 0;
}
#endif

#if 0
csv_row_t *FS_LoadCSV(const char *path)
{
	int n, i, j, len;
	char buf[MAX_FILEBUF + 1];
	char *tail, *head = buf;
	csv_row_t *rows, *row;
	csv_row_t tmp;
	bool quoted;

	// TODO: Split into multiple functions
	// TODO: Further testing and clean up
	// TODO: Free memory on non-fatal error

	n = FS_ReadRAM(path, buf, MAX_FILEBUF);
	if (n < 0)
		return NULL;

	buf[n] = '\0';
	quoted = false;
	rows = NULL;
	i = j = 0;

	while (SkipWhitespace(&head)) {
		for (tail = head;; tail++) {
			if (*tail == '\0')
				break;

			if (*tail == '"')
				quoted = !quoted;

			if ((*tail == ',') || (*tail == '#')
			|| ((*tail == ';') || (*tail == '\n'))) {
				if (quoted == false)
					break;
			}

			if (i >= MAX_CSV_DATA) {
				Warning(E_ROWLEN);
				return NULL;
			}

			tmp.data[i++] = *tail;
		}

		if ((len = tail - head) >= 0) {
			if (j >= MAX_CSV_COLS) {
				Warning(E_NUMCOL);
				return NULL;
			}

			while (tmp.data[i - 1] <= ' ')
				len--, i--;

			tmp.cols[j++] = i - len;
			tmp.data[i++] = '\0';
		}

		if (*tail == '#') {
			Info("SKIP_COMMENT ???");
			while (*tail && *tail != '\n')
				tail++;
		}

		if ((*tail == ',' || *tail == '\0')
		|| ((*tail == ';' || *tail == '\n'))) {
			if (j && *tail != ',') {
				row = Allocate(sizeof(*row));
				memcpy(row, &tmp, sizeof(*row));
				row->size = j;
				while (j < MAX_CSV_COLS)
					row->cols[j++] = 0;

				row->next = rows;
				rows = row;
				i = j = 0;
			}
			tail++;
		}

		head = tail;
	}

	if (quoted) {
		Warning(E_NQUOTE);
		return NULL;
	}

	return rows;
}
#endif
