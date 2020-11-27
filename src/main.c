#include "common.h"

int main(int argc, char **argv)
{
	if (FS_Init() < 0)
		Error(E_FSINIT);

	if (NET_Init() < 0)
		Error(E_NOSOCK);

	UNUSED(argc);
	UNUSED(argv);

	FS_Shutdown();

	return 0;
}
