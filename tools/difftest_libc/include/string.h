/*
STRING.H

The C library functions that the game's code uses, for the image that the
function tests run (tools/difftest.py). libc.c defines them.
*/

#ifndef __DIFFTEST_STRING_H
#define __DIFFTEST_STRING_H

#include <stddef.h>

void *memchr(const void *memory, int value, size_t size);
int memcmp(const void *a, const void *b, size_t size);
void *memcpy(void *destination, const void *source, size_t size);
void *memmove(void *destination, const void *source, size_t size);
void *memset(void *memory, int value, size_t size);
int strcmp(const char *a, const char *b);
size_t strlen(const char *string);
int strncmp(const char *a, const char *b, size_t size);

#endif
