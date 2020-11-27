#include "common.h"
#include "net.h"

int main(int argc, char **argv)
{
	if (net_init() < 0)
		Error(E_NOSOCK);

	UNUSED(argc);
	UNUSED(argv);

	return 0;
}
