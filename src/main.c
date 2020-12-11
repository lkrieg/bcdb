#include "common.h"

void usage(void);

int main(int argc, char **argv)
{
	arg_t arg;

	CMD_Init(argc, argv);
	while (CMD_Next(&arg)) {
		switch (arg.type) {

		// -d, --daemon
		case T_ARG_DAEMON:
			Info("Running in daemon mode");
			break; // TODO: Fork main process

		// -k, --kill
		case T_ARG_SHUTDOWN:
			Info("Shutting down daemon");
			break; // TODO: Kill active forks

		// -f, --file [filename]
		case T_ARG_IMPORT:
			Info("Importing '%s'", arg.value);
			break; // TODO: Import from CSV file

		// Unknown argument
		case T_ARG_INVALID:
			usage();
		}
	}

	UNUSED(argc);
	UNUSED(argv);

	return 0;
}

void usage(void)
{
	// TODO: Print usage
	exit(EXIT_SUCCESS);
}
