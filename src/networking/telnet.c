#include "common.h"
#include "telnet.h"

int TEL_Init(void)
{
	return 0;
}

int TEL_Parse(net_cln_t *cln, const byte *data, int size)
{
	const byte *head, *tail;
	char bar[MAX_BARCODE];
	int len;

	len  = 0;
	head = data;
	tail = data + size;

	// TODO: Handle telnet negotiations
	// TODO: Disable linemode, get TTYPE and NAWS
	// TODO: Expect data to be received in parts
	// TODO: Implement linebuffering per client

	for (; head < tail; head++) {
		if (*head & 0x80 || *head <= ' ') {
			if (*head >= 251 && *head <= 254)
				head++;
			continue;
		}

		if (len >= MAX_BARCODE)
			return -1;

		bar[len++] = *head;
	}

	if (len == 0)
		return 0;

	bar[len] = '\0';
	DAT_Lookup("TODO", bar);
	UNUSED(cln);

	return 0;
}
