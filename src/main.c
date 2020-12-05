#include "common/common.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

static int   InitSignalHandlers(void);
static void  HandleInterrupt(int sig);
static void  HandleChild(int sig);

static void AddMenuEntry(net_cln_t *cln, const char *name, bool active)
{
	int len;
	int pad, n;
	char buf[MAX_MSG_LEN];

	pad = 0;
	len = strlen(name);

	if (len < cln->cols)
		pad = (cln->cols - len) / 2;

	for (n = 0; n < pad; n++)
		buf[n] = ' ';

	NET_Send(cln, buf, n);
	if (active == true) {
		NET_Send(cln, "\033[30m", 5);
		NET_Send(cln, "\033[47m", 5);
	}
	NET_Send(cln, " ", 1);
	NET_Send(cln, name, len);
	NET_Send(cln, " ", 1);
	NET_Send(cln, "\033[37m", 5);
	NET_Send(cln, "\033[40m", 5);
	NET_Send(cln, "\r\n", 2);
}

static void DrawMainMenu(net_cln_t *cln, int selected)
{
	NET_Send(cln, "\033[2J", 4);
	NET_Send(cln, "\033[H", 3);

	NET_Send(cln, "\r\n", 2);
	NET_Send(cln, "\033[37m", 5);
	NET_Send(cln, "\033[40m", 5);
	AddMenuEntry(cln, "BARKEEPER", false);
	AddMenuEntry(cln, "=========", false);
	NET_Send(cln, "\r\n", 2);
	AddMenuEntry(cln, "Verladung", (selected == 0));
	AddMenuEntry(cln, "Sammeln",   (selected == 1));
	AddMenuEntry(cln, "Beenden",   (selected == 2));
}

static void HandleEvents(net_cln_t *cln, net_evt_t *evt)
{
	static int mode = T_MOD_MAINMENU;
	static int selected = 0;

	switch (evt->type) {
	case T_EVT_DATA:
		break;
	case T_EVT_KEYDOWN:
		if (mode == T_MOD_MAINMENU) {
			switch (evt->keycode) {
			case T_KEY_UP:
				if (selected <= 0) break;
				DrawMainMenu(cln, --selected);
				break;
			case T_KEY_DOWN:
				if (selected >= 2) break;
				DrawMainMenu(cln, ++selected);
				break;
			case T_KEY_RETURN:
				switch (selected) {
				case 2: // EXIT
					NET_Close(cln);
					exit(EXIT_SUCCESS);
					break;
				}
			}
		}
		break;
	case T_EVT_RESIZE:
		cln->rows = evt->rows;
		cln->cols = evt->cols;
		if (mode == T_MOD_MAINMENU)
			DrawMainMenu(cln, selected);
		break;
	case T_EVT_TTYPE:
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
			DrawMainMenu(&cln, 0);
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


