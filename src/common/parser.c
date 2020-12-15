#include "common.h"
#include "parser.h"

int ReadConfKey(char **buf, char **key)
{
	char *head = *buf;
	char *tail = head;
	int len, spaces = 0;

	Assert(buf && *buf);
	Assert(key != NULL);

	for (;; tail++) {
		if (*tail == '=')
			break;

		if (*tail <= ' ')
			spaces++;

		if ((*tail == '#') // Require OP_ASSIGN
		|| ((*tail == '\0') || (*tail == '\n'))) {
			Warning(E_EXPECT " '='");
			return -1;
		}

		// Illegal whitespace
		if (spaces && (*tail > ' ')) {
			Warning(E_SPACES);
			return -2;
		}
	}

	len = tail - head - spaces;
	if (!len || len >= MAX_CFG_KEY) {
		Warning(E_GETKEY);
		return -3;
	}

	*buf = tail + 1;
	*key = head;

	return len;
}

int ReadConfVal(char **buf, char **val)
{
	char *head = *buf;
	char *tail = head;
	int len;

	Assert(buf && *buf);
	Assert(val != NULL);

	for (;; head++) {
		if ((*head == '#') // Missing token value
		|| ((*head == '\0' || *head == '\n'))) {
			Warning(E_ENOVAL);
			return -1;
		}

		if (*head <= ' ')
			continue;

		break;
	}

	tail = head;

	// Find end of token
	while (*tail > ' ') {
		if (*tail == '#')
			break;
		tail++;
	}

	len = tail - head;
	if (len > MAX_CFG_VAL) {
		Warning(E_CFGLEN " '%.*s'", len, head);
		return -2;
	}

	*buf = tail;
	*val = head;

	return len;
}

int SkipWhitespace(char **buf)
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
