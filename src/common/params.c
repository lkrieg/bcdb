#include "common.h"
#include "params.h"

#include <getopt.h>
#include <string.h>
#include <limits.h>

static int     varnum, index;
static cvar_t  varbuf[MAX_CFG_NUM];

#define NUM_VARDEFS (NUM_CVAR_IDS - 1)
static const cvar_t vardefs[NUM_VARDEFS] = {
	{T_CFG_DAEMON,  T_VAR_BOOL, 'd', "daemon",  {0}, {.bol = false}},
	{T_CFG_KILL,    T_VAR_BOOL, 'k', "kill",    {0}, {.bol = false}},
	{T_CFG_VERBOSE, T_VAR_BOOL, 'v', "verbose", {0}, {.bol = false}},
	{T_CFG_HELP,    T_VAR_BOOL, 'h', "help",    {0}, {.bol = false}},
	{T_CFG_FILE,    T_VAR_STR,  'f', "file",    {0}, {.str = NULL}},
	{T_CFG_PORT,    T_VAR_NUM,  'p', "port",    {0}, {.num = 23}}};

int CFG_ParseFile(const char *path)
{
	// TODO
	UNUSED(path);
	UNUSED(vardefs);
	return 0;
}

int CFG_ParseArgs(int argc, char **argv)
{
	struct option opts[NUM_VARDEFS + 1];
	char optstr[NUM_VARDEFS * 2 + 1];
	struct option * opt;
	const cvar_t * def;
	int n, i = 0;
	int m, j = 0;
	long num;

	if (argc == 0)
		return 0;

	if (argc > MAX_CFG_NUM) {
		Warning(E_ARGNUM);
		argc = MAX_CFG_NUM;
	}

	index  = 0;
	opterr = 0;
	opt = opts;
	def = vardefs;

	memset(opts, 0, sizeof(opts));
	for (n = 0; n < NUM_VARDEFS; n++) {
		opt->name     = def->key;
		opt->val      = def->argchar;
		opt->has_arg  = no_argument;
		opt->flag     = NULL;

		optstr[i++] = opt->val;
		if (def->type != T_VAR_BOOL) {
			opt->has_arg = required_argument;
			optstr[i++] = ':';
		}

		opt++;
		def++;
	}

	optstr[i] = '\0';
	do { // Parse command-line arguments from vardefs
		m = getopt_long(argc, argv, optstr, opts, &j);
		if (m == '?' || m == ':')
			Error(E_ARGVAL);

		for (n = 0; n < NUM_VARDEFS; n++) {
			if (vardefs[n].argchar != m)
				continue;

			if (vardefs[n].type == T_VAR_NUM) {
				num = strtol(optarg, NULL, 0);
				if (num <= 0 || num > INT_MAX)
					Error(E_NOTNUM);
				Info("%s = %d", vardefs[n].key, num);
			} else if (vardefs[n].type == T_VAR_STR) {
				Info("%s = %s", vardefs[n].key, optarg);
			} else {
				Info("%s = true", vardefs[n].key);
			}
		}
	} while (m >= 0);

	return 0;
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
