#include "common.h"

static void    Usage(void);
static void    Configure(int argc, char **argv);
static void    Import(const char *filename);
static void    Shutdown(int signal);
static int     Run(void);

static bool    do_fork;  // Run in daemon mode
static bool    do_kill;  // Kill active daemons
static int     telport;  // External telnet port
static int     webport;  // External http port
static char *  file;     // Data file path

static void Usage(void)
{
	Print( // Output usage information for command-line arguments
	"Usage: [ -d | -k | -r | -v | -h ] [ -f filename ] [ -p | -w port ] \n"
	"See 'man barkeeper' for more details about command-line options    \n"
	"                                                                   \n"
	"  -d, --daemon   run process in background                         \n"
	"  -k, --kill     stop active background process                    \n"
	"  -r, --restart  restart active daemon, same as --kill --daemon    \n"
	"  -f, --file     import csv data from file, can be done while the  \n"
	"                 daemon process is running in the background       \n"
	"  -p, --port     set external telnet port                          \n"
	"  -w, --http     set external http port                            \n"
	"  -v, --verbose  enable verbose logging                            \n"
	"  -h, --help     print this help text                              ");
}

static void Configure(int argc, char **argv)
{
	cvar_t cvar;

	telport  = BASE_TEL_PORT;
	webport  = BASE_WEB_PORT;
	file     = BASE_FILE;

	if (CFG_ParseFile(CFGPATH) < 0)
		Error(E_GETCFG);

	if (CFG_ParseArgs(argc, argv) < 0)
		Error(E_GETARG);

	while (CFG_Next(&cvar)) {
		switch (cvar.id) {

		// -v, --verbose
		// verbose = <cbool>
		case T_CFG_VERBOSE:
			verbose = CBOOL(cvar);
			break;

		// -d, --daemon
		// daemon = <cbool>
		case T_CFG_DAEMON:
			do_fork = CBOOL(cvar);
			break;

		// -k, --kill
		// kill = <cbool>
		case T_CFG_KILL:
			do_kill = CBOOL(cvar);
			break;

		// -p, --port
		// port = <cnum>
		case T_CFG_PORT:
			telport = CNUM(cvar);
			break;

		// -w, --http
		// http = <cnum>
		case T_CFG_HTTP:
			webport = CNUM(cvar);
			break;

		// -f, --file
		// file = <cstr>
		case T_CFG_FILE:
			file = CSTR(cvar);
			break;

		// -r, --restart
		// restart = <cbool>
		case T_CFG_RESTART:
			if (CBOOL(cvar) == true) {
				do_kill = true;
				do_fork = true;
			}
			break;

		// -h, --help
		// help = <cbool>
		case T_CFG_HELP:
			Usage();
			exit(0);
		}
	}
}

static int Run(void)
{
	if (!IsPrivileged())
		Error(E_NOROOT);

	if (do_kill) {
		KillProcess();
		if (!do_fork)
			return 0;
	}

	if (GetActivePid())
		Error(E_ACTIVE);

	Info("Running in %s mode...", (do_fork)
	     ? "daemon" : "interactive");

	if ((do_fork)
	&& ((ForkProcess() < 0)))
		Error(E_NOFORK);

	SetPidLock(true);
	signal(SIGTERM, Shutdown);

	if (DB_Init() < 0)
		Error(E_DBINIT);

	if (file != NULL)
		Import(file);

	if (NET_Init(telport, webport) < 0)
		Warning(E_NOSOCK);

	Info("Waiting...");
	while (1); // TODO

	return 0;
}

static void Import(const char *filename)
{
	Info("Importing data file '%s'...", filename);
}

static void Shutdown(int signal)
{
	Info("Shutting down...");

	SetPidLock(false);
	exit(EXIT_SUCCESS);
	UNUSED(signal);
}

int main(int argc, char **argv)
{
	Configure(argc, argv);
	Verbose("Setting log level to verbose...");

	return Run();
}
