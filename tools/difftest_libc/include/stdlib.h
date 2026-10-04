/*
STDLIB.H

The C library functions that the game's code uses, for the image that the
function tests run (tools/difftest.py). libc.c defines them.
*/

#ifndef __DIFFTEST_STDLIB_H
#define __DIFFTEST_STDLIB_H

#include <stddef.h>

void qsort(void *elements, size_t count, size_t size, int (*compare)(const void *, const void *));

#endif
