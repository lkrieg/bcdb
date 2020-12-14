#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

bool verbose = false;
static const char *prefixes[] = {
	"[INFO] ", "[DEBUG] ",
	"[WARN] ", "[ERROR] ",
	""
};

enum log_levels
{
	T_LOG_INFO,
	T_LOG_VERBOSE,
	T_LOG_WARNING,
	T_LOG_ERROR,
	T_LOG_NONE
};

static int GetPid(void);
static int SetPid(bool active);

void SetPidLock(bool active)
{
	// Ensure that only one instance of the daemon is
	// active at the same time by creating and locking
	// the pid file. This file must be deleted when the
	// process terminates by passing false when calling
	// this function again.

	if (SetPid(active) < 0)
		Error(E_SETPID);
}

bool IsAlreadyActive(void)
{
	int pid;

	pid = GetPid();
	if ((!pid) || (pid == getpid()))
		return false;

	errno = 0; // PID inactive?
	if (kill(pid, 0) && errno == ESRCH)
		return false;

	return true;
}

bool IsPrivileged(void)
{
	return geteuid() == 0;
}

static void Log(int level, const char *fmt, va_list arg)
{
	char msg[MAX_LINEBUF];

	Assert(fmt != NULL);

	if (level < 0 || level > 4)
		level = T_LOG_NONE;

	vsnprintf(msg, MAX_LINEBUF, fmt, arg);
	printf("%s%s\n", prefixes[level], msg);

	fflush(stdout);
}

void Print(const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	Log(T_LOG_NONE, fmt, arg);
	va_end(arg);
}

void Info(const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	Log(T_LOG_INFO, fmt, arg);
	va_end(arg);
}

void Verbose(const char *fmt, ...)
{
	va_list arg;

	if (!verbose)
		return;

	va_start(arg, fmt);
	Log(T_LOG_VERBOSE, fmt, arg);
	va_end(arg);
}

void Warning(const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	Log(T_LOG_WARNING, fmt, arg);
	va_end(arg);
}

void Error(const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	Log(T_LOG_ERROR, fmt, arg);
	va_end(arg);

	// Critical failure
	exit(EXIT_FAILURE);
}

void _Assert(int exp, const char *text, const char *file, int line)
{
	if (exp == 0)
		Error(E_ASSERT ": '%s' (%s:%d)", text, file, line);
}

void *_Allocate(int size)
{
	void *ptr;

	// There is no way of safely recovering from out of memory
	// errors, but things have to go total haywire for that. The
	// Linux kernel will do things like disk swapping to prevent
	// the worst case from ever happening. And maybe we can do
	// the most stuff with memory from the stack.

	ptr = malloc(size);

	if (ptr == NULL)
		Error(E_ENOMEM);

	return ptr;
}

void *_AllocateDebug(int size, const char *file, int line)
{
	// TODO
	UNUSED(file);
	UNUSED(line);

	return _Allocate(size);
}

void _Free(void *ptr)
{
	free(ptr);
}

void _FreeDebug(void *ptr)
{
	// TODO
	_Free(ptr);
}

void _Memcheck(void)
{
	// TODO
	return;
}

static int GetPid(void)
{
	FILE *fp;
	int pid;

	if (!(fp = fopen(PIDPATH, "r")))
		return 0;

	if (!fscanf(fp, "%d", &pid))
		return 0;

	fclose(fp);
	return pid;
}

static int SetPid(bool active)
{
	int fd;
	FILE *fp;
	int pid;

	if (active == false)
		return unlink(PIDPATH);

	fd = open(PIDPATH, O_RDWR | O_CREAT, 0644);

	if ((fd < 0)
	|| (!(fp = fdopen(fd, "r+")))) {
		Warning(E_FDOPEN " for '%s'", PIDPATH);
		return -1;
	}

	pid = getpid();
	Verbose("Setting active pid to %d", pid);
	if (!fprintf(fp, "%d\n", pid)) {
		Warning(E_SETPID ": %s",
		        strerror(errno));
		return -3;
	}

	fflush(fp);
	close(fd);
	return pid;
}
