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
static void Store(int id, const char *val, int len);

static const cvar_t vardefs[NUM_VARDEFS] = {
	{T_CFG_DAEMON,  T_VAR_BOOL, 'd', "daemon",  {0}, {.bol = false}},
	{T_CFG_KILL,    T_VAR_BOOL, 'k', "kill",    {0}, {.bol = false}},
	{T_CFG_VERBOSE, T_VAR_BOOL, 'v', "verbose", {0}, {.bol = false}},
	{T_CFG_HELP,    T_VAR_BOOL, 'h', "help",    {0}, {.bol = false}},
	{T_CFG_FILE,    T_VAR_STR,  'f', "file",    {0}, {.str = NULL}},
	{T_CFG_PORT,    T_VAR_NUM,  'p', "port",    {0}, {.num = 0}}};

void CFG_ParseFile(const char *path)
{
	char buf[MAX_FILEBUF];
	char *tail, *head = buf;
	int len, klen, vlen;
	int spaces, n;
	bool found;

	// TODO: Split into multiple functions
	// TODO: Clean up and write unit-tests

	if ((len = GetFile(path, buf)) < 0) {
		Warning(E_NOCONF " '%s'", path);
		return;
	}

	while (*head != '\0') {
		if (*head <= ' ') {
			head++;
			continue;
		}

		if (*head == '#') {
			while ((*head != '\0')
			   && ((*head != '\n')))
				head++;
			continue;
		}

		spaces = 0;
		tail = head;
		while (*tail != '=') {
			if (*tail <= ' ')
				spaces++;

			// Expect OP_ASSIGN
			if ((*tail == '#')
			|| ((*tail == '\0'))
			|| ((*tail == '\n')))
				Error(E_EXPECT " '='");

			// Non-trailing whitespace
			if (spaces && *tail > ' ')
				Error(E_SPACES);

			tail++;
		}

		found = false;
		klen = tail - head - spaces;
		if (klen >= MAX_CFG_KEY)
			Error(E_KEYLEN);

		for (n = 0; n < NUM_VARDEFS; n++) {
			if (!strncmp(vardefs[n].key, head, klen)) {
				found = true;
				break;
			}
		}

		if (!found) // Invalid key name
			Error(E_CFGKEY ": '%.*s'", klen, head);

		head = tail + 1;
		while (*head && *head <= ' ') {
			if (*head == '\n')
				break;
			head++;
		}

		tail = head;
		while ((*tail > ' ')
		   && ((*tail != '#')))
			tail++;

		vlen = tail - head;

		if (vlen == 0)
			Error(E_NOVAL);
		if (vlen >= MAX_CFG_VAL)
			Error(E_VALLEN);

		Store(n, head, vlen);
		head = tail;
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
		|| ((n = read(fd, out + total, MAX_FILEBUF - total)) < 0))
			return -1;

		total += n;
	} while (n);

	out[total] = '\0';
	close(fd);

	return total;
}

static int GetOpts(const struct option *opts, const char *argstr)
{
	int opt, j, n;

	opterr = 0;
	opt = j = 0;
	while (opt >= 0) {
		opt = getopt_long(vargc, vargv, argstr, opts, &j);
		if (opt == '?' || opt == ':')
			return -1;

		for (n = 0; n < NUM_VARDEFS; n++) {
			if (vardefs[n].argchar != opt)
				continue;

			switch (vardefs[n].type) {
			case T_VAR_NUM:
			case T_VAR_STR:
				Store(n, optarg, strlen(optarg));
				break;
			default:
			case T_VAR_BOOL:
				Store(n, "true", 4);
			}
		}
	}

	return 0;
}

static void Store(int id, const char *val, int len)
{
	cvar_t *out;

	out = &varbuf[varnum++];
	if (varnum >= MAX_CFG_NUM)
		Error(E_ARGNUM);

	*out = vardefs[id]; // Memcpy
	strncpy(out->val, val, len);
	out->val[len] = '\0';

	Verbose("Parsed config value %s='%s'",
	        out->key, out->val);

	switch (out->type) {
	case T_VAR_NUM: // Numeric
		out->as.num = strtol(out->val, NULL, 0);
		if (out->as.num <= 0 || out->as.num > INT_MAX)
			Error(E_NOTNUM);
		break;
	case T_VAR_STR: // Textual
		out->as.str = out->val;
		break;
	default:
	case T_VAR_BOOL: // Boolean
		if (!strcmp("true", out->val))
			out->as.bol = true;
		else if (!strcmp("false", out->val))
			out->as.bol = false;
		else
			Error(E_NOBOOL ": '%s'", out->val);
	}
}
