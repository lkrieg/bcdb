#include "common.h"

static void Usage(void);
static void Run(bool daemon);
static void Import(const char *path);
static void Shutdown(void);

static void Usage(void)
{
	Print( // command-line argument descriptions
	"Usage: barkeeper [ -d | -k ] [ -f filename ] [ -p port ] [ -h ] \n"
	"See 'man barkeeper' for more details about command-line options \n"
	"                                                                \n"
	"  -d, --daemon  run in daemon mode                              \n"
	"  -k, --kill    stop active daemon                              \n"
	"  -f, --file    load additional barcode data, can be done while \n"
	"                the daemon process is running in the background \n"
	"  -p, --port    set telnet port, this option has priority over  \n"
	"                settings found in /etc/barkeeper.cfg            \n"
	"  -h, --help    print this help text                            ");
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

	port = CFG_GetNum(T_CFG_PORT);
	while (CMD_Next(&arg)) {
		switch (arg.type) {

		// -d, --daemon
		case T_ARG_FORK:
			Run(true);
			break;

		// -k, --kill
		case T_ARG_KILL:
			Shutdown();
			break;

		// -f, --file
		case T_ARG_FILE:
			Import(arg.value);
			break;

		// -p, --port
		case T_ARG_PORT:
			errno = 0;
			port = strtol(arg.value, NULL, 0);
			if (port <= 0 || errno == ERANGE)
				Error(E_NOTNUM ": '%s'", arg.value);
			Info("Setting port to %d", port);
			break;

		// -h, --help
		case T_ARG_HELP:
			Usage();
			exit(0);
		}
	}

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
