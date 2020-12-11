#include "common.h"

static void Usage(void);
static void Run(bool daemon);
static void Import(const char *path);
static void Shutdown(void);

static void Usage(void)
{
	Print( // command-line argument descriptions
	"Usage: barkeeper [ -d | -k ] [ -f filename ]  \n"
	"See 'man barkeeper' for more information      \n"
	"                                              \n"
	"  -d, --daemon  run in daemon mode            \n"
	"  -k, --kill    stop active daemon            \n"
	"  -f, --file    load additional barcode data  \n"
	"                (this can be done while the   \n"
	"                daemon process is running)    \n"
	"  -h, --help    print this help text          ");
}

int main(int argc, char **argv)
{
	arg_t arg;
	int port;

	if (CFG_Init() < 0)
		Error(E_CFGVAL);

	// get configuration file settings
	port = CFG_GetNumber(T_CFG_PORT);

	// parse command-line
	CMD_Init(argc, argv);
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

		// -h, --help
		case T_ARG_HELP:
		case T_ARG_INVALID:
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
