#include "common.h"

// General settings
static bool do_fork;
static char *file;
static int port;

static void  Usage(void);
static void  Configure(int argc, char **argv);
static void  Import(const char *filename);
static int   Run(bool do_fork, int port);
static void  Shutdown(void);

static void Usage(void)
{
	Print( // Output usage information for command-line arguments
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

static void Configure(int argc, char **argv)
{
	cvar_t cvar;

	CFG_ParseFile(CONFPATH);
	CFG_ParseArgs(argc, argv);

	while (CFG_Next(&cvar)) {
		switch (cvar.id) {

		// -v, --verbose
		// verbose = true
		case T_CFG_VERBOSE:
			verbose = CBOOL(cvar);
			break;

		// -d, --daemon
		// daemon = true
		case T_CFG_DAEMON:
			do_fork = CBOOL(cvar);
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

		// -k, --kill
		case T_CFG_KILL:
			Shutdown();
			exit(0);

		// -h, --help
		case T_CFG_HELP:
			Usage();
			exit(0);
		}
	}
}

static int Run(bool do_fork, int port)
{
	Verbose("Log level set to verbose");
	Info("Running in %s mode", (do_fork)
	     ? "daemon" : "interactive");

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

int main(int argc, char **argv)
{
	Configure(argc, argv);

	if (file != NULL)
		Import(file);

	// TODO: Is daemon active?
	// Begin normal execution
	return Run(do_fork, port);
}
