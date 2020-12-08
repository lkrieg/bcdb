#include "common.h"
#include "net.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

static int   InitSignalHandlers(void);
static void  HandleInterrupt(int sig);
static void  HandleChild(int sig);

int main(void)
{
	bool active;
	net_cln_t cln;
	net_evt_t evt;
	int n;

	// Initialize network
	if (NET_Init() < 0)
		Error(E_INITIP);

	// Register signal handlers
	if (InitSignalHandlers() < 0)
		Error(E_SIGNAL);

	active = true;
	while (active) { // Accept clients
		if (NET_Accept(&cln) < 0) {
			Warning(E_ACCEPT);
			continue;
		}

		switch (fork()) {
		case 0: // Client process
			close(cln.parent);

			// Set telnet negotiation parameters
			NET_Negotiate(&cln, T_DO   | T_OPT_SLE);
			NET_Negotiate(&cln, T_WILL | T_OPT_SGA);

			// Keep polling for client events
			while (NET_NextEvent(&cln, &evt)) {
				switch (evt.type) {
				case T_EVT_DATA:
					printf("Info: Client sent data:");
					for (n = 0; n < evt.size; n++)
						printf(" %02X", evt.data[n]);
					printf("\n");
					break;
				case T_EVT_KEYDOWN:
					switch (evt.keycode) {
					case T_KEY_LEFT:
						Info("Client pressed left");
						break;
					case T_KEY_RIGHT:
						Info("Client pressed right");
						break;
					}
					break;
				case T_EVT_RESIZE:
					Info("Client window resized to %dx%d",
					     evt.rows, evt.cols);
					break;
				case T_EVT_TTYPE:
					Info("Client terminal changed");
					break;
				}
			}

			// Client disconnected from server
			Info(M_CLOSED " '%s'", cln.name);
			close(cln.handle);
			exit(EXIT_SUCCESS);
			break;

		case -1: // Unable to fork
			Error(E_NOFORK ": %s", strerror(errno));

		default: // Parent process
			close(cln.handle);
		}
	}

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
	Info(M_EXIT);
	NET_Shutdown();
	UNUSED(sig);
	exit(0);
}

static void HandleChild(int sig)
{
	int eno = errno;

	// Reap zombie child
	while (waitpid(-1, NULL, WNOHANG) > 0);

	errno = eno;
	UNUSED(sig);
}
