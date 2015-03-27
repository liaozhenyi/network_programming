#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "error.h"

/*
 * Print a message and return to caller.
 */
static void err_doit(int errnoflag, int error, const char *fmt, va_list ap)
{
	char buf[MSG_SIZE];

	vsnprintf(buf, MSG_SIZE, fmt, ap);
	if (errnoflag)
		snprintf(buf+strlen(buf), MSG_SIZE-strlen(buf), ":%s", strerror(error));
	strcat(buf, "\n");
	fflush(stdout);
	fputs(buf, stderr);
	fflush(NULL);
}

/*
 * Nonfatal error, unrelated to a system call
 */
void err_msg(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	err_doit(0, 0, fmt, ap);
	va_end(ap);
}

/*
 * Fatal error, unrelated to a system call
 */
void err_exit(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	err_doit(0, 0, fmt, ap);
	va_end(ap);
	exit(1);
}

/*
 * Nonfatal error, related to a system call 
 */
void err_ret(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
}

/*
 * Fatal error, related to a system call
 */
void err_sys(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
	exit(1);
}


