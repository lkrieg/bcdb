#include "common.h"

static int  Run(bool do_fork, int port);
static void Import(const char *path);
static void Shutdown(void);

static void Usage(void)
{
	Print( // output usage information for command-line arguments
	"Usage: barkeeper [ -d | -k | -v | -h ] [ -f filename ] [ -p port ] \n"
	"See 'man barkeeper' for more details about command-line options    \n"
	"                                                                   \n"
	"  -d, --daemon   run in daemon mode                                \n"
	"  -k, --kill     stop active daemon                                \n"
	"  -f, --file     load additional barcode data, can be done while   \n"
	"                 the daemon process is running in the background   \n"
	"  -p, --port     set telnet port, this option has priority over    \n"
	"                 settings found in /etc/barkeeper.cfg              \n"
	"  -v, --verbose  output verbose log messages                       \n"
	"  -h, --help     print this help text                              ");
}

int main(int argc, char **argv)
{
	cvar_t cvar;
	bool do_fork;
	bool do_kill;
	bool do_help;
	char *file;
	int port;

	// User configuration
	CFG_ParseFile(CONFPATH);
	CFG_ParseArgs(argc, argv);

	while (CFG_Next(&cvar)) {
		switch (cvar.id) {

		// -v, --verbose
		// verbose = true
		case T_CFG_VERBOSE:
			verbose = CBOL(cvar);
			break;

		// -d, --daemon
		// daemon = true
		case T_CFG_DAEMON:
			do_fork = CBOL(cvar);
			break;

		// -k, --kill
		case T_CFG_KILL:
			do_kill = CBOL(cvar);
			break;

		// -h, --help
		case T_CFG_HELP:
			do_help = CBOL(cvar);
			break;

		// -f, --file
		// file = foo.csv
		case T_CFG_FILE:
			file = CSTR(cvar);
			break;

		// -p, --port
		// port = 2323
		case T_CFG_PORT:
			port = CNUM(cvar);
			break;

		}
	}

	if (do_help) {
		Usage();
		return 0;
	}

	if (do_kill) {
		Shutdown();
		return 0;
	}

	// Begin program execution
	return Run(do_fork, port);

	UNUSED(Import);
	UNUSED(file);
}

static int Run(bool do_fork, int port)
{
	Verbose("Log level set to verbose");

	if (do_fork)
		Info("Running as daemon");

	if (NET_Init(port) < 0)
		Error(E_NOSOCK);

	return 0;
}

static void Import(const char *filename)
{
	Info("Importing '%s'", filename);
}

static void Shutdown(void)
{
	Info("Shutting down");
}
