#ifndef __ERROR_H_
#define __ERROR_H_

#include <stdarg.h>		// va_*

#define MSG_SIZE 1024

void err_msg(const char *fmt, ...);
void err_exit(const char *fmt, ...);
void err_ret(const char *fmt, ...);
void err_sys(const char *fmt, ...);

#endif	// __ERROR_H_
