#include "common.h"

void Usage(void)
{
	Print( // Command-line argument usage description:
	"Usage: barkeeper [ -d | -k ] [ -f filename ]  \n"
	"See 'man barkeeper' for more information      \n"
	"                                              \n"
	"  -d, --daemon  run in daemon mode            \n"
	"  -k, --kill    stop active daemon            \n"
	"  -f, --file    load additional barcode data  \n"
	"                (this can be done while the   \n"
	"                daemon process is running)    \n"
	"  -h, --help    output this help text         ");
}

int main(int argc, char **argv)
{
	arg_t arg;

	CMD_Init(argc, argv);
	while (CMD_Next(&arg)) {
		switch (arg.type) {

		// -d, --daemon
		case T_ARG_FORK:
			Info("Running in daemon mode");
			break; // TODO: Fork main process

		// -k, --kill
		case T_ARG_KILL:
			Info("Shutting down daemon");
			break; // TODO: Kill active forks

		// -f, --file [filename]
		case T_ARG_FILE:
			Info("Importing '%s'", arg.value);
			break; // TODO: Import from CSV file

		// -h, --help
		case T_ARG_HELP:
		case T_ARG_INVALID:
			Usage();
			exit(0);
			break;
		}
	}

	return 0;
}
