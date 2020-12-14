#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
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

static int GetMainPid(void);
static int SetMainPid(bool active);

int GetActivePid(void)
{
	int pid;

	pid = GetMainPid();
	if ((!pid) || (pid == getpid()))
		return 0;

	errno = 0; // PID inactive?
	if (kill(pid, 0) && errno == ESRCH)
		return 0;

	return pid;
}

void SetPidLock(bool active)
{
	// Ensure that only one instance of the daemon is
	// active at the same time by creating and locking
	// the pid file. This file must be deleted when the
	// process terminates by passing false when calling
	// this function again.

	if (SetMainPid(active) < 0)
		Error(E_SETPID);
}

bool IsPrivileged(void)
{
	return geteuid() == 0;
}

int ForkProcess(void)
{
	int pid;
	int flags;
	int logfd;

	// Daemons are typically created by forking twice
	// with the intermediate process exiting after
	// forking the grandchild. This has the effect of
	// orphaning the grandchild process.

	// First fork
	pid = fork();
	if (pid < 0)
		return -1;
	if (pid > 0)
		exit(0);

	// Detach from CTTY
	if (setsid() < 0)
		return -2;

	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	// Second fork
	pid = fork();
	if (pid < 0)
		return -1;
	if (pid > 0)
		exit(0);

	umask(0);
	chdir("/");

	close(STDIN_FILENO);
	if (open("/dev/null", O_RDONLY) < 0)
		return -3;

	// Redirect output streams to logfile
	flags = O_RDWR | O_CREAT | O_APPEND;
	logfd = open(LOGPATH, flags, 0644);

	if (logfd < 0)
		return -3;

	dup2(logfd, STDOUT_FILENO);
	dup2(logfd, STDERR_FILENO);
	close(logfd);

	return 0;
}

void KillProcess(void)
{
	int pid;

	if ((pid = GetActivePid()) == 0) {
		Info("Daemon is inactive");
		return;
	}

	Info("Shutting down daemon...");
	kill(pid, SIGTERM);
	while (access(PIDPATH, F_OK) == 0)
		sleep(1);
}

void Sleep(int n)
{
	sleep(n);
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

	// if (ptr == NULL)
	//	Error(E_ENOMEM);

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

static int GetMainPid(void)
{
	FILE *fp;
	int pid;

	if (access(PIDPATH, F_OK) != 0)
		return 0;

	if (!(fp = fopen(PIDPATH, "r")))
		return 0;

	if (!fscanf(fp, "%d", &pid))
		return 0;

	fclose(fp);
	return pid;
}

static int SetMainPid(bool active)
{
	int fd;
	FILE *fp;
	int pid;

	if (active == false) {
		if ((pid = GetMainPid()) == 0)
			return 0;

		Verbose("Disabling lock for pid %d...", pid);
		unlink(PIDPATH);
		return 0;
	}

	pid = getpid();
	Verbose("Enabling lock for pid %d...", pid);
	fd = open(PIDPATH, O_RDWR | O_CREAT, 0644);

	if ((fd < 0)
	|| (!(fp = fdopen(fd, "r+")))) {
		Warning(E_FDOPEN " for '%s'", PIDPATH);
		return -1;
	}

	if (!fprintf(fp, "%d\n", pid)) {
		Warning(E_SETPID ": %s", strerror(errno));
		return -2;
	}

	fflush(fp);
	close(fd);

	return pid;
}
