#include "common.h"
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
static int    GetFile(const char *path, char *out);
static int    GetArgs(const struct option *opts, const char *argstr);
static void   Store(int id, const char *val, int len);
static int    ReadKey(char **buf, char **key);
static int    ReadVal(char **buf, char **val);
static int    SkipWhitespace(char **buf);

static const cvar_t vardefs[NUM_VARDEFS] = {
	{T_CFG_DAEMON,  T_VAR_BOOL, 'd', "daemon",  {0}, {.bol = false}},
	{T_CFG_VERBOSE, T_VAR_BOOL, 'v', "verbose", {0}, {.bol = false}},
	{T_CFG_RESTART, T_VAR_BOOL, 'r', "restart", {0}, {.bol = false}},
	{T_CFG_KILL,    T_VAR_BOOL, 'k', "kill",    {0}, {.bol = false}},
	{T_CFG_HELP,    T_VAR_BOOL, 'h', "help",    {0}, {.bol = false}},
	{T_CFG_FILE,    T_VAR_STR,  'f', "file",    {0}, {.str = NULL}},
	{T_CFG_PORT,    T_VAR_NUM,  'p', "port",    {0}, {.num = 0}}};

int CFG_ParseFile(const char *path)
{
	char buf[MAX_FILEBUF];
	char *head = buf;
	char *key, *val;
	int id, len;

	Assert(path != NULL);

	if (GetFile(path, buf) < 0)
		Error(E_NOREAD " '%s'", path);

	// TODO: Report FILE and LINENO
	while (SkipWhitespace(&head)) {
		if ((len = ReadKey(&head, &key)) < 1)
			Error(E_GETKEY);

		if ((id = GetCvar(key, len)) < 0)
			Error(E_GETKEY ": '%.*s'", len, key);

		if ((len = ReadVal(&head, &val)) < 1)
			Error(E_GETVAL);

		Store(id, val, len);
	}

	return 0;
}

int CFG_ParseArgs(int argc, char **argv)
{
	char argstr[MAX_ARGSTR];
	struct option opts[NUM_OPTDEFS];
	struct option *opt;
	const cvar_t *def;
	int n, i;

	if (argc < 2)
		return 0;

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
	if (GetArgs(opts, argstr) < 0)
		Error(E_ARGVAL);

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

	return (found) ? id : -1;
}

static int GetFile(const char *path, char *out)
{
	int fd, n;
	int total = 0;

	Assert(path != NULL);
	Assert(out != NULL);

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

static int GetArgs(const struct option *opts, const char *argstr)
{
	int opt = 0;
	int n, id = 0;

	Assert(opts != NULL);
	Assert(argstr && *argstr);

	opterr = 0;
	while (opt >= 0) {
		opt = getopt_long(vargc, vargv, argstr, opts, &n);
		if (opt == '?' || opt == ':')
			return -1;

		// TODO: This should use GetCvar()
		for (id = 0; id < NUM_VARDEFS; id++) {
			if (vardefs[id].argchar != opt)
				continue;

			switch (vardefs[id].type) {
			case T_VAR_NUM:
			case T_VAR_STR:
				Store(id, optarg, strlen(optarg));
				break;
			default:
			case T_VAR_BOOL:
				Store(id, "true", 4);
			}
		}
	}

	return 0;
}

static void Store(int id, const char *val, int len)
{
	cvar_t *out;

	Assert(id >= 0);
	Assert(id < NUM_VARDEFS);
	Assert(val != NULL);

	if (len > MAX_CFG_VAL)
		Error(E_CFGLEN);

	if (varnum > MAX_CFG_NUM)
		Error(E_CFGNUM);

	out = &varbuf[varnum++];
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

static int ReadKey(char **buf, char **key)
{
	char *head = *buf;
	char *tail = head;
	int len, spaces = 0;
	char c;

	Assert(buf && *buf);
	Assert(key != NULL);

	for (;;) {
		c = *tail++;

		if (c == '=')
			break;

		if (c <= ' ')
			spaces++;

		if ((c == '#') // Need OP_ASSIGN
		|| ((c == '\0') || (c == '\n')))
			Error(E_EXPECT " '='");

		// Illegal whitespace
		if (spaces && (c > ' '))
			Error(E_SPACES);
	}

	len = tail - head - spaces - 1;
	if (len >= MAX_CFG_KEY)
		return 0;

	*buf = tail;
	*key = head;

	return len;
}

static int ReadVal(char **buf, char **val)
{
	char *head = *buf;
	char *tail = head;
	int len;
	char c;

	Assert(buf && *buf);
	Assert(val != NULL);

	for (;;) {
		c = *head++;

		if (c == '\0')
			break;

		if (c <= ' ')
			continue;

		head--;
		tail = head;
		while (*tail > ' ') {
			if (*tail == '#')
				break;
			tail++;
		}

		break;
	}

	len = tail - head;
	if (len > MAX_CFG_VAL)
		return 0;

	*buf = tail;
	*val = head;

	return len;
}

static int SkipWhitespace(char **buf)
{
	char *head = *buf;

	Assert(buf && *buf);

	while (*head != '\0') {
		if (*head <= ' ') {
			head++;
		} else if (*head == '#') {
			while (*head != '\n') {
				if (*head == '\0')
					break;
				head++;
			}
			continue;
		} else {
			break;
		}
	}

	*buf = head;
	return (*head != '\0');
}
