#include "common/common.h"

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
	byte buf[MAX_MSG_LEN];
	int n, i;

        // Register signal handlers
        if (InitSignalHandlers() < 0)
                Error(E_SIGNAL);

	if (NET_Init() < 0)
		Error(E_IPFAIL);

	active = true;
	while (active) {
		if (NET_Accept(&cln) < 0)
			continue;

		switch (fork()) {
		case 0: // Client process
			close(cln.parent);

			// TODO: Begin client authentication
			while ((n = NET_Read(&cln, buf)) >= 0) {
				for (i = 0; i < n; i++)
					Print(" %02X", buf[i]);
				Print("%c", (n > 0) ? '\n' : '\0');
			}

			close(cln.handle);
			exit(EXIT_SUCCESS);
			break;

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


