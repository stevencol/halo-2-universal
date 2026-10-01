/*
STDIO.H

For the image that the function tests run (tools/difftest.py). libc.c
defines the functions.
*/

#ifndef __DIFFTEST_STDIO_H
#define __DIFFTEST_STDIO_H

#include <stdarg.h>
#include <stddef.h>

/* %s, %c, %d, %i, %u, %x, %X and %%, with a width, a precision and the
flags - and 0 */
int vsnprintf(char *buffer, size_t size, const char *format, va_list arguments);

#endif
