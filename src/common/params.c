#include "common.h"
#include "parser.h"
#include "params.h"

#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <getopt.h>
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

static int    GetCvar(const char *key, int len);
static int    GetArgs(const struct option *opts, const char *argstr);
static int    Store(int id, const char *val, int len);

static const cvar_t vardefs[NUM_VARDEFS] = {
	{T_CFG_DAEMON,  T_VAR_BOOL, 'd', "daemon",  {0}, {.bol = false}},
	{T_CFG_VERBOSE, T_VAR_BOOL, 'v', "verbose", {0}, {.bol = false}},
	{T_CFG_RESTART, T_VAR_BOOL, 'r', "restart", {0}, {.bol = false}},
	{T_CFG_KILL,    T_VAR_BOOL, 'k', "kill",    {0}, {.bol = false}},
	{T_CFG_HELP,    T_VAR_BOOL, 'h', "help",    {0}, {.bol = false}},
	{T_CFG_FILE,    T_VAR_STR,  'f', "file",    {0}, {.str = NULL}},
	{T_CFG_PORT,    T_VAR_NUM,  'p', "port",    {0}, {.num = 0}},
	{T_CFG_HTTP,    T_VAR_NUM,  'w', "http",    {0}, {.num = 0}}};

int CFG_ParseFile(const char *path)
{
	char buf[MAX_FILEBUF + 1];
	char *head = buf;
	char *key, *val;
	int id, len, n;

	Assert(path != NULL);

	if ((n = FS_ReadRAM(path, buf, MAX_FILEBUF)) < 0) {
		Warning(E_NOREAD " '%s'", path);
		return -1;
	}

	buf[n] = '\0';
	while (SkipWhitespace(&head)) {
		if ((len = ReadConfKey(&head, &key)) < 1)
			return -2; // Missing key

		if ((id = GetCvar(key, len)) < 0)
			return -3; // Unknown key

		if ((len = ReadConfVal(&head, &val)) < 1)
			return -4; // Invalid value

		if (Store(id, val, len) < 0)
			return -5; // Reached limit
	}

	return 0;
}

int CFG_ParseArgs(int argc, char **argv)
{
	char argstr[MAX_ARGSTR];
	struct option opts[NUM_OPTDEFS];
	struct option *opt;
	const cvar_t *def;
	int n, i, len;

	if ((argc < 2)
	|| ((argv == NULL)))
		return 0;

	vargc  = argc;     // Number of arguments
	vargv  = argv;     // Argument text buffer
	opt    = opts;     // Iterator for options
	def    = vardefs;  // Iterator for vardefs

	// Generate options from vardefs
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
	if ((len = GetArgs(opts, argstr)) < 0)
		return -1;

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

static int GetCvar(const char *key, int len)
{
	int id;
	bool found;

	Assert(key != NULL);

	if (len == 0)
		return -1;

	// Using hashtables here would be overkill
	// since the number of vardefs stays manageable
	// and at worst only effects the startup time

	found = false;
	for (id = 0; id < NUM_VARDEFS; id++) {
		if (!strncmp(vardefs[id].key, key, len)) {
			found = true; // Matching vardef
			break;
		}
	}

	if (!found) {
		Warning(E_GETKEY ": '%.*s'", len, key);
		return -1;
	}

	return id;
}

static int GetArgs(const struct option *opts, const char *argstr)
{
	int id = 0;
	int opt = 0;
	int len, n;
	const char *cp;

	Assert(opts != NULL);
	Assert(argstr && *argstr);

	opterr = 0;
	while (opt >= 0) {
		opt = getopt_long(vargc, vargv, argstr, opts, &n);
		if (opt == '?' || opt == ':') {
			// Check if option is unknown or missing value
			for (cp = argstr; *cp && *cp != optopt; cp++);
			Warning((*cp && *cp != ':') ? E_OPTVAL : E_OPTYPE);
			return -1;
		}

		// Could propably use GetCvar() here
		for (id = 0; id < NUM_VARDEFS; id++) {
			if (vardefs[id].argchar != opt)
				continue;

			switch (vardefs[id].type) {
			case T_VAR_NUM:
			case T_VAR_STR:
				len = strlen(optarg);
				if (Store(id, optarg, len) < 0)
					return -2;
				break;
			default:
			case T_VAR_BOOL:
				if (Store(id, "true", 4) < 0)
					return -2;
			}
		}
	}

	return 0;
}

static int Store(int id, const char *val, int len)
{
	cvar_t *out;

	Assert(id >= 0);
	Assert(id < NUM_VARDEFS);
	Assert(val != NULL);

	if (len >= MAX_CFG_VAL) {
		Warning(E_CFGLEN);
		return -1;
	}

	if (varnum > MAX_CFG_NUM) {
		Warning(E_CFGNUM);
		return -2;
	}

	// Copy to cvar buffer
	out = &varbuf[varnum++];
	*out = vardefs[id];
	strncpy(out->val, val, len);
	out->val[len] = '\0';

	switch (out->type) {
	case T_VAR_NUM: // Numeric value
		out->as.num = strtol(out->val, NULL, 0);
		if (out->as.num <= 0 || out->as.num > INT_MAX) {
			Warning(E_NOTNUM " '%s' for %s",
			        out->val, out->key);
			return -3;
		}
		break;
	case T_VAR_STR: // String value
		out->as.str = out->val;
		if ((strlen(out->val) == 0)
		|| ((out->val[0] == '-'))) {
			Warning(E_NOTSTR " '%s' for %s",
			       out->val, out->key);
			return -3;
		}
		break;
	default:
	case T_VAR_BOOL: // Boolean value
		if (!strcmp("true", out->val)) {
			out->as.bol = true;
		} else if (!strcmp("false", out->val)) {
			out->as.bol = false;
		} else {
			Warning(E_NOTBOL " '%s' for %s",
			        out->val, out->key);
			return -3;
		}
	}

	return 0;
}
