#include "common.h"
#include "webapi.h"

#include <string.h>
#include <stdio.h>

enum web_cache_index
{
	T_CACHE_HTML,
	T_CACHE_STYLE,
	T_CACHE_SCRIPT,
	T_CACHE_ICON,

	CACHE_SIZE,
	T_CACHE_JSON
};

static int sizes[CACHE_SIZE];
static char cache[CACHE_SIZE][MAX_FILEBUF];
const char *paths[CACHE_SIZE] = {
	"/var/lib/barkeeper/index.html",
	"/var/lib/barkeeper/style.css",
	"/var/lib/barkeeper/main.js",
	"/var/lib/barkeeper/favicon.ico"
};

static void SendList(net_cln_t *cln);
static void SendHeader(net_cln_t *cln, int type, int length);
static int Cache(int index);

int WEB_Init(void)
{
	int n;

	// Intialize static file cache
	for (n = 0; n < CACHE_SIZE; n++) {
		if (Cache(n) < 0)
			return -1;
	}

	return 0;
}

int WEB_Parse(net_cln_t *cln, const byte *data, int size)
{
	const byte *head;
	const byte *tail;
	int len;

	head = data;
	tail = head;

	// Just a rough test how to handle http requests
	// But we should keep static paths for the routing
	// and triple check for any buffer overflows here

	// Skip leading whitespace
	for (; *head; head++) {
		if (*head > ' ')
			break;
	}

	// Find request line tail
	for (tail = head; *tail; tail++) {
		if (*tail == '\r' || *tail == '\n')
			break;
	}

	size = tail - head;
	Verbose("Parsing HTTP request '%.*s'...", size, data);

	if ((size >= 16)
	&& ((!memcmp(head, "GET /favicon.ico", 16)))) {
		len = sizes[T_CACHE_ICON];
		SendHeader(cln, T_CACHE_ICON, len);
		NET_Send(cln, (byte *) cache[T_CACHE_ICON], len);

	} else if ((size >= 14)
	&& ((!memcmp(head, "GET /style.css", 14)))) {
		len = sizes[T_CACHE_STYLE];
		SendHeader(cln, T_CACHE_STYLE, len);
		NET_Send(cln, (byte *) cache[T_CACHE_STYLE], len);

	} else if ((size >= 12)
	&& ((!memcmp(head, "GET /main.js", 12)))) {
		len = sizes[T_CACHE_SCRIPT];
		SendHeader(cln, T_CACHE_SCRIPT, len);
		NET_Send(cln, (byte *) cache[T_CACHE_SCRIPT], len);

	} else if ((size >= 9)
	&& ((!memcmp(head, "GET /list", 9)))) {
		SendList(cln);

	} else if ((size >= 6)
	&& ((!memcmp(head, "GET / ", 6)))) {
		len = sizes[T_CACHE_HTML];
		SendHeader(cln, T_CACHE_HTML, len);
		NET_Send(cln, (byte *) cache[T_CACHE_HTML], len);
	}

	return 0;
}

static void SendList(net_cln_t *cln)
{
	const char *out;
	int len;

	len = DAT_GetCache(&out);
	SendHeader(cln, T_CACHE_JSON, len);

	if (len == 0)
		return;

	NET_Send(cln, (byte *) out, len);
}

static void SendHeader(net_cln_t *cln, int type, int length)
{
	char buf[MAX_LINEBUF];
	int n;

	// Header should be directly sent with the response data
	// But keeping it simple for now, just to get it working

	n = snprintf(buf, MAX_LINEBUF, "%s%s%s%d%s",
	            "HTTP/1.1 200 OK\r\nContent-Type: ",

	  	    (type == T_CACHE_HTML)   ? "text/html"       :
		    (type == T_CACHE_SCRIPT) ? "text/javascript" :
		    (type == T_CACHE_STYLE)  ? "text/css"        :
		    (type == T_CACHE_ICON)   ? "image/x-icon"    :
		                               "application/json",

		    "\r\nContent-Length: ", length,
		    "\r\nConnection: Closed\r\n\r\n");

	NET_Send(cln, (byte *) buf, n);
}

static int Cache(int index)
{
	const char *path;
	int *size, n;
	char *data;

	Assert(index >= 0);
	Assert(index < CACHE_SIZE);

	path = paths[index];
	data = cache[index];
	size = sizes + index;

	if ((n = FS_ReadRAM(path, data, MAX_FILEBUF)) < 0) {
		Warning(E_CACHED " '%s'", path);
		return -1;
	}

	*size = n;
	return 0;
}
