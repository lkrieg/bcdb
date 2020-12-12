#include "common.h"

static void Usage(void);
static void Run(bool daemon);
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
	arg_t arg;
	int port;

	// config file settings
	if (CFG_Init() < 0)
		Error(E_CFGVAL);

	// command-line arguments
	if (CMD_Init(argc, argv) < 0)
		Error(E_ARGVAL);

	verbose  = CFG_GetBool(T_CFG_VERBOSE);
	port     = CFG_GetNum(T_CFG_PORT);

	while (CMD_Next(&arg)) {
		switch (arg.type) {

		// -d, --daemon
		case T_ARG_DAEMON:
			Run(true);
			break;

		// -k, --kill
		case T_ARG_KILL:
			Shutdown();
			break;

		// -f, --file
		case T_ARG_FILE:
			Import(arg.str);
			break;

		// -p, --port
		case T_ARG_PORT:
			port = arg.num;
			break;

		// -v, --verbose
		case T_ARG_VERBOSE:
			verbose = true;
			break;

		// -h, --help
		case T_ARG_HELP:
			Usage();
			exit(0);
		}
	}

	UNUSED(verbose);
	UNUSED(port);

	Memcheck();
	return 0;
}

static void Run(bool daemon)
{
	if (daemon == true)
		Info("Running as daemon");
}

static void Import(const char *filename)
{
	Info("Importing '%s'", filename);
}

static void Shutdown(void)
{
	Info("Shutting down");
}
