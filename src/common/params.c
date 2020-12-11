#include "common.h"
#include "params.h"

#include <getopt.h>
#include <string.h>

static int    argnum, index;
static arg_t  argbuf[MAX_ARG_NUM];

void CMD_Init(int argc, char **argv)
{
	arg_t *head;
	int c, i = 0;

	static char opstr[] = "dkhf:";
	static struct option opts[] = {
		{"daemon",  no_argument,        0,  'd'},
		{"kill",    no_argument,        0,  'k'},
		{"help",    no_argument,        0,  'h'},
		{"file",    optional_argument,  0,  'f'},
		{0,         0,                  0,   0 }};

	if (argc > MAX_ARG_NUM)
		Error(E_ARGNUM);

	index  = 0;
	opterr = 0;

	while ((c = getopt_long(argc, argv, opstr, opts, &i)) >= 0) {
		head = &argbuf[argnum++];
		switch (c) {
		case 'd':  head->type = T_ARG_FORK; break;
		case 'k':  head->type = T_ARG_KILL; break;
		case 'h':  head->type = T_ARG_HELP; break;
		case -1 :  head->type = T_ARG_NONE; return;
		case 'f':  if (optarg != NULL) {
				if (strlen(optarg) >= MAX_ARG_LEN)
					Error(E_ARGLEN);

				head->type = T_ARG_FILE;
				strcpy(head->value, optarg);
				break;
		           } // fallthrough
		default:
		case ':':
		case '?':  argbuf[0].type = T_ARG_INVALID;
		           argnum++;
		           return;
		}
	}
}

int CMD_Next(arg_t *out)
{
	arg_t *head;

	if (index >= argnum) {
		out->type = T_ARG_NONE;
		return out->type;
	}

	head = &argbuf[index++];
	*out = *head;

	return out->type;
}
