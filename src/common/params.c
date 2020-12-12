#include "common.h"
#include "params.h"

#include <getopt.h>
#include <string.h>
#include <limits.h>

#define NUM_OPTDEFS (NUM_CVAR_IDS)
#define NUM_VARDEFS (NUM_CVAR_IDS - 1)
#define MAX_ARGSTR  (NUM_VARDEFS * 2 + 1)

static int     varnum, index;
static cvar_t  varbuf[MAX_CFG_NUM];

static const cvar_t vardefs[NUM_VARDEFS] = {
	{T_CFG_DAEMON,  T_VAR_BOOL, 'd', "daemon",  {0}, {.bol = false}},
	{T_CFG_KILL,    T_VAR_BOOL, 'k', "kill",    {0}, {.bol = false}},
	{T_CFG_VERBOSE, T_VAR_BOOL, 'v', "verbose", {0}, {.bol = false}},
	{T_CFG_HELP,    T_VAR_BOOL, 'h', "help",    {0}, {.bol = false}},
	{T_CFG_FILE,    T_VAR_STR,  'f', "file",    {0}, {.str = NULL}},
	{T_CFG_PORT,    T_VAR_NUM,  'p', "port",    {0}, {.num = 23}}};

void CFG_ParseFile(const char *path)
{
	UNUSED(path);
}

void CFG_ParseArgs(int argc, char **argv)
{
	char argstr[MAX_ARGSTR];
	struct option opts[NUM_OPTDEFS];
	struct option *opt;
	const cvar_t * def;
	int n, m, i, j;
	cvar_t *out;

	if (argc == 0)
		return;

	opt = opts;
	def = vardefs;
	opterr = m = i = j = 0;
	memset(opts, 0, sizeof(opts));

	// Translate vardefs to options
	for (n = 0; n < NUM_VARDEFS; n++) {
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
	while (m >= 0) { // Parse command-line arguments
		m = getopt_long(argc, argv, argstr, opts, &j);
		if (m == '?' || m == ':')
			Error(E_ARGVAL);

		for (n = 0; n < NUM_VARDEFS; n++) {
			if (vardefs[n].argchar != m)
				continue;

			out = &varbuf[varnum++];
			if (varnum >= MAX_CFG_NUM)
				Error(E_ARGNUM);

			*out = vardefs[n];
			switch (out->type) {
			case T_VAR_NUM: // Numberic
				strncpy(out->val, optarg, MAX_CFG_VAL);
				out->as.num = strtol(out->val, NULL, 0);
				if (out->as.num <= 0 || out->as.num > INT_MAX)
					Error(E_NOTNUM);
				break;
			case T_VAR_STR: // String
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
