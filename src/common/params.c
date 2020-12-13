#include "common.h"
#include "params.h"

#include <getopt.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define NUM_OPTDEFS (NUM_CVAR_IDS)
#define NUM_VARDEFS (NUM_CVAR_IDS - 1)
#define MAX_ARGSTR  (NUM_VARDEFS * 2 + 1)

static int      varnum, index;
static cvar_t   varbuf[MAX_CFG_NUM];
static int      vargc;
static char **  vargv;

static int GetFile(const char *path, char *out);
static int GetOpts(const struct option *opts, const char *argstr);

static const cvar_t vardefs[NUM_VARDEFS] = {
	{T_CFG_DAEMON,  T_VAR_BOOL, 'd', "daemon",  {0}, {.bol = false}},
	{T_CFG_KILL,    T_VAR_BOOL, 'k', "kill",    {0}, {.bol = false}},
	{T_CFG_VERBOSE, T_VAR_BOOL, 'v', "verbose", {0}, {.bol = false}},
	{T_CFG_HELP,    T_VAR_BOOL, 'h', "help",    {0}, {.bol = false}},
	{T_CFG_FILE,    T_VAR_STR,  'f', "file",    {0}, {.str = NULL}},
	{T_CFG_PORT,    T_VAR_NUM,  'p', "port",    {0}, {.num = 23}}};

void CFG_ParseFile(const char *path)
{
	char buf[MAX_FILEBUF];

	// TODO: Parse config file for option keys
	// defined above and write values to varbuf
	// Print("%s\n", buf);

	if (GetFile(path, buf) < 0) {
		Warning(E_NOCONF " '%s'", path);
		return;
	}
}

void CFG_ParseArgs(int argc, char **argv)
{
	char argstr[MAX_ARGSTR];
	struct option opts[NUM_OPTDEFS];
	struct option *opt;
	const cvar_t *def;
	int n, i;

	if (argc < 2)
		return;

	vargc  = argc;
	vargv  = argv;
	opt    = opts;
	def    = vardefs;

	// Generate list of options
	memset(opts, 0, sizeof(opts));
	for (n = i = 0; n < NUM_VARDEFS; n++) {
		opt->name     = def->key;
		opt->val      = def->argchar;
		opt->has_arg  = no_argument;
		opt->flag     = NULL;

		argstr[i++] = opt->val;
		if (def->type != T_VAR_BOOL) {
			opt->has_arg = required_argument;
			argstr[i++] = ':';
		}

		opt++;
		def++;
	}

	argstr[i] = '\0';
	if (GetOpts(opts, argstr) < 0)
		Error(E_ARGVAL);
}

int CFG_Next(cvar_t *out)
{
	cvar_t *head;

	Assert(out != NULL);
	out->type = T_CFG_END;

	if (index < varnum) {
		head = &varbuf[index++];
		*out = *head; // memcpy
	}

	return out->type;
}

static int GetFile(const char *path, char *out)
{
	int fd, n;
	int total;

	total = 0;

	if ((fd = open(path, O_RDONLY)) < 0)
		return -1;
	do {
		if ((total == MAX_FILEBUF)
		|| ((n = read(fd, out + total, MAX_FILEBUF)) < 0))
			return -1;

		total += n;
	} while (n);

	out[total] = '\0';
	close(fd);

	return 0;
}

static int GetOpts(const struct option *opts, const char *argstr)
{
	cvar_t *out;
	int m = 0;
	int j = 0;
	int n;

	opterr = 0;
	while (m >= 0) {
		m = getopt_long(vargc, vargv, argstr, opts, &j);
		if (m == '?' || m == ':')
			return -1;

		for (n = 0; n < NUM_VARDEFS; n++) {
			if (vardefs[n].argchar != m)
				continue;

			out = &varbuf[varnum++];
			if (varnum >= MAX_CFG_NUM)
				Error(E_ARGNUM);

			*out = vardefs[n];
			switch (out->type) {
			case T_VAR_NUM: // Numeric
				strncpy(out->val, optarg, MAX_CFG_VAL);
				out->as.num = strtol(out->val, NULL, 0);
				if (out->as.num <= 0 || out->as.num > INT_MAX)
					Error(E_NOTNUM);
				break;
			case T_VAR_STR: // Textual
				strncpy(out->val, optarg, MAX_CFG_VAL);
				out->as.str = out->val;
				break;
			default:
			case T_VAR_BOOL: // Boolean
				strcpy(out->val, "true");
				out->as.bol = true;
				break;
			}
		}
	}

	return 0;
}
