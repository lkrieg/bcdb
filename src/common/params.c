#include "common.h"
#include "params.h"

#include <getopt.h>
#include <string.h>

static int   argnum, index;
static arg_t argbuf[MAX_ARG_NUM];

int CMD_Init(int argc, char **argv)
{
	arg_t *head;
	int c, i = 0;

	static const char opstr[] = "dkhf:p:";
	static const struct option opts[] = {
		{"daemon",  no_argument,        0,  'd'},
		{"kill",    no_argument,        0,  'k'},
		{"help",    no_argument,        0,  'h'},
		{"file",    required_argument,  0,  'f'},
		{"port",    required_argument,  0,  'p'},
		{0,         0,                  0,   0 }};

	if (argc > MAX_ARG_NUM)
		Error(E_ARGNUM);

	index  = 0;
	opterr = 0;

	while ((c = getopt_long(argc, argv, opstr, opts, &i)) >= 0) {
		head = &argbuf[argnum++];
		switch (c) {
		case 'd':  head->type = T_ARG_DAEMON;  break;
		case 'k':  head->type = T_ARG_KILL;    break;
		case 'h':  head->type = T_ARG_HELP;    break;
		case -1 :  head->type = T_ARG_NONE;    return 0;

		case 'f':  // T_ARG_FILE || T_ARG_PORT
		case 'p':  if ((optarg == NULL) // required value
		           || ((strlen(optarg) >= MAX_ARG_LEN)))
				return -1;

		           strcpy(head->value, optarg);
		           head->type = (c == 'f') ? T_ARG_FILE :
		                        (c == 'p') ? T_ARG_PORT : 0;
		           break;

		default:
		case ':':
		case '?':  // invalid
		           return -1;
		}
	}

	return 0;
}

int CMD_Next(arg_t *out)
{
	arg_t *head;

	if (index >= argnum) {
		out->type = T_ARG_NONE;
		return out->type;
	}

	head = &argbuf[index++];
	*out = *head; // memcpy

	return out->type;
}
