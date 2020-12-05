#include "common/common.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

static int   InitSignalHandlers(void);
static void  HandleInterrupt(int sig);
static void  HandleChild(int sig);

static void HandleEvents(net_cln_t *cln, net_evt_t *evt)
{
	switch (evt->type) {
	case T_EVT_DATA:
		Print("T_EVT_DATA:");
		for (int i = 0; i < evt->size; i++)
			Print(" %02X", evt->data[i]);
		Print("\n");
		break;
	case T_EVT_RESIZE:
		Info("T_EVT_RESIZE: %dx%d", evt->rows, evt->cols);
		cln->rows = evt->rows;
		cln->cols = evt->cols;
		break;
	case T_EVT_TTYPE:
		Info("T_EVT_TTYPE: %s", evt->data);
		break;
	}
}

int main(void)
{
	bool active;
	net_cln_t cln;

	if (NET_Init() < 0)
		Error(E_IPFAIL);

	if (InitSignalHandlers() < 0)
		Error(E_SIGNAL);

	active = true;
	while (active) {
		if (NET_Accept(&cln) < 0)
			continue;

		switch (fork()) {
		case 0: // Client process
			close(cln.parent);
			NET_SetHandler(&cln, HandleEvents);
			while (NET_NextEvent(&cln));

			// Disconnected
			NET_Close(&cln);
			exit(EXIT_SUCCESS);

		case -1: // Unable to fork
			Error(E_NOFORK ": %s", strerror(errno));

		default: // Parent process
			close(cln.handle);

		}
	}

	NET_Shutdown();
	return 0;
}

static int InitSignalHandlers(void)
{
	struct sigaction st, sc;

	// Reap zombie child processes
	sc.sa_handler  = HandleChild;
	sc.sa_flags    = SA_RESTART;
	sigemptyset(&sc.sa_mask);

	// Handle termination signals
	st.sa_handler  = HandleInterrupt;
	st.sa_flags    = 0;
	sigemptyset(&st.sa_mask);

	if ((sigaction(SIGINT,  &st, NULL) < 0)
	|| ((sigaction(SIGHUP,  &st, NULL) < 0))
	|| ((sigaction(SIGTERM, &st, NULL) < 0))
	|| ((sigaction(SIGCHLD, &sc, NULL) < 0)))
		return -1;

        return 0;
}

static void HandleInterrupt(int sig)
{
	NET_Shutdown();
	exit(EXIT_SUCCESS);
	UNUSED(sig);
}

static void HandleChild(int sig)
{
	int eno = errno;

	// Reap zombie child
	while (waitpid(-1, NULL, WNOHANG) > 0);

	errno = eno;
	UNUSED(sig);
}


