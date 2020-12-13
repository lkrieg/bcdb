#include "common.h"
#include "socket.h"

int NET_Init(int tel, int web)
{
	Info("Binding telnet to 0.0.0.0:%d...", tel);
	Info("Binding webapi to 0.0.0.0:%d...", web);

	// TODO
	return -1;
}

void NET_Shutdown(void)
{
	Info("Closing socket");

	// TODO
	return;
}
