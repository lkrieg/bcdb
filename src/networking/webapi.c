#include "common.h"
#include "webapi.h"

static void Route(const net_cln_t *cln, const byte *url);

int WEB_Parse(net_cln_t *cln, const byte *data, int size)
{
	Verbose("Parsing:\n%.*s", size, data);

	UNUSED(Route);
	UNUSED(cln);

	return -1;
}

static void Route(const net_cln_t *cln, const byte *url)
{
	UNUSED(cln);
	UNUSED(url);
}
