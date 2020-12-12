#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#define TEST_SETUP(g)      static void _test_setup_##g(void)
#define TEST_TEAR_DOWN(g)  static void _test_clean_##g(void)
#define TEST(g, c)         static void _test_main_##g##_##c(void); \
                           static _test_t _test_info_##g##_##c     \
                           __attribute((__section__("test")))      \
                           __attribute((__used__)) = { #g": "#c,   \
                           sizeof(#g#c) + 4, 0, _test_setup_##g,   \
                           _test_clean_##g, _test_main_##g##_##c   \
                           }; static void _test_main_##g##_##c(void)
#define TEST_ASSERT(c)     _test_assert(c)

typedef void (*_test_fn)(void);
typedef struct _test_s _test_t;
void _test_assert(int exp);

struct _test_s {
	char        *name;
	int         length;
	int         status;
	_test_fn    setup;
	_test_fn    clean;
	_test_fn    main;
} __attribute__((
  aligned((32))));

#endif // TEST_UTIL_H
#ifdef TEST_MAIN

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define _TEST_EXIT(n) exit(n)
#define _TEST_FOR_EACH(s, p, m) \
	for (p = &__stop_##s - 1; p >= &__start_##s; \
             m = (p->length > m) ? p->length : m, p--)

extern _test_t __start_test;
extern _test_t __stop_test;
static int _test_numfailed;

void _test_assert(int exp)
{
	if (exp == 0)
		_TEST_EXIT(1);
}

static void _test_run(_test_t *test)
{
	pid_t pid;
	int status;

	pid = fork();
	if (pid == 0) {
		test->setup();
		test->main();
		test->clean();
		_TEST_EXIT(0);
	}

	if ((pid < 0)
	|| (wait(&status) != pid)
	|| (WIFEXITED(status) == 0)
	|| (WEXITSTATUS(status))) {
		_test_numfailed++;
		test->status = 1;
	}
}

static void _test_print(_test_t *test, int max)
{
	char linebuf[4096];
	char *head, c;

	memset(linebuf, '.', max);
	strcpy(linebuf + max, (test->status)
	       ? "FAIL" : "OK");

	head = linebuf;
	while ((c = *test->name++)) {
		if (c == '_') c = ' ';
		*head++ = c;
	}

	printf("[CHECK] %s\n", linebuf);
}

int main(void)
{
	_test_t *it;
	int max = 0;

	_TEST_FOR_EACH(test, it, max)
		_test_run(it);

	_TEST_FOR_EACH(test, it, max)
		_test_print(it, max);

	return _test_numfailed > 0;
}

#endif // TEST_MAIN
