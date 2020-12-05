#include "common/common.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

static int   InitSignalHandlers(void);
static void  HandleInterrupt(int sig);
static void  HandleChild(int sig);

static void InitWindow(net_cln_t *cln)
{
	WINDOW *wnd;

	noecho();
	start_color();
	init_pair(1, COLOR_BLACK, COLOR_BLUE);
	init_pair(2, COLOR_BLACK, COLOR_WHITE);

	wnd = newwin(7, 14, LINES/2-3, COLS/2-7);
	bkgd(COLOR_PAIR(1));
	wbkgd(wnd, COLOR_PAIR(2) | A_BOLD);
	box(wnd, 0, 0);

	refresh();
	cln->window = wnd;
}

static void DrawMenu(net_cln_t *cln, int selected)
{
	int i;
	char item[64];
	char *menu[3] = {
		" Verladen ",
		" Sammeln  ",
		" Beenden  "
	};

	for (i = 0; i < 3; i++) {
		if (i == selected)
			wattron(cln->window, A_STANDOUT);
		sprintf(item, "%-7s", menu[i]);
		mvwprintw(cln->window, i + 2, 2, "%s", item);
		wattroff(cln->window, A_STANDOUT);
	}

	wrefresh(cln->window);
}

static void HandleInput(net_cln_t *cln, int code)
{
	static int mode = T_MOD_MAINMENU;
	static int n;

	switch (mode) {
	case T_MOD_MAINMENU:
		switch (code) {
		case T_KEY_UP:
			n--;
			n = (n < 0) ? 2 : n;
			DrawMenu(cln, n);
			break;
		case T_KEY_DOWN:
			n++;
			n = (n > 2) ? 0 : n;
			DrawMenu(cln, n);
			break;
		case T_KEY_RETURN:
			if (n == 2) {
				endwin();
				NET_Close(cln);
				exit(EXIT_SUCCESS);
			}
			break;
		case T_EVT_RESIZE:
			InitWindow(cln); // FIXME
			DrawMenu(cln, n);
			break;
		}
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
		if (NET_Accept(&cln, HandleInput) < 0)
			continue;

		switch (fork()) {
		case 0: // Client process
			close(cln.parent);
			InitWindow(&cln);
			DrawMenu(&cln, 1);
			while (NET_NextEvent(&cln));

			// Disconnected
			endwin();
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


