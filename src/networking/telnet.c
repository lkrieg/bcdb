#include "common.h"
#include "telnet.h"

int TEL_Init(void)
{
	return 0;
}

int TEL_Parse(net_cln_t *cln, const byte *data, int size)
{
	UNUSED(cln);
	UNUSED(data);
	UNUSED(size);

	return -1;
}
