#include "common/common.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

static int   InitSignalHandlers(void);
static void  HandleInterrupt(int sig);
static void  HandleChild(int sig);

// Just for prototyping the GUI...
static void DrawMainMenu(net_cln_t *cln)
{
	int i, ch;
	WINDOW *w;
	char item[64];
	char *menu[3] = {
		" Verladen ",
		" Sammeln  ",
		" Beenden  "
	};

	start_color();
	init_pair(1, COLOR_BLACK, COLOR_WHITE);
	init_pair(2, COLOR_BLACK, COLOR_BLUE);

	bkgd(COLOR_PAIR(2));
	refresh();

	w = newwin(7, 14, 4, 5);
	wbkgd(w, COLOR_PAIR(1) | A_BOLD);

	box(w, 0, 0);

	//wattron(w, A_BOLD);
	//mvwprintw(w, 0, 5, "MENU");
	//wattroff(w, A_BOLD);
	wrefresh(w);

	for (i = 0; i < 3; i++) {
		if (i == 0)
			wattron(w, A_STANDOUT | A_BOLD);
		else
			wattroff(w, A_STANDOUT | A_BOLD);
		sprintf(item, "%-7s", menu[i]);
		mvwprintw(w, i + 2, 2, "%s", item);
	}

	i = 0;
	noecho();
	keypad(w, TRUE);
	curs_set(0);

	while ((ch = wgetch(w)) != 'q') {
		sprintf(item, "%-7s", menu[i]);
		mvwprintw(w, i + 2, 2, "%s", item);

		switch(ch) {
		case KEY_UP:
		case 0x32:
			i--;
			i = (i < 0) ? 2 : i;
			break;
		case KEY_DOWN:
		case 0x38:
			i++;
			i = (i > 2) ? 0 : i;
			break;
		case 0x0A:
			if (i == 2) {
				endwin();
				NET_Close(cln);
				exit(EXIT_SUCCESS);
			}
			break;
		}

		wattron(w, A_STANDOUT | A_BOLD);
		sprintf(item, "%-7s", menu[i]);
		mvwprintw(w, i + 2, 2, "%s", item);
		wattroff(w, A_STANDOUT | A_BOLD);
	}

	delwin(w);
	endwin();
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
			DrawMainMenu(&cln);
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


