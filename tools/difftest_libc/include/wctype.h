/*
WCTYPE.H

For the image that the function tests run (tools/difftest.py). libc.c
defines the functions.
*/

#ifndef __DIFFTEST_WCTYPE_H
#define __DIFFTEST_WCTYPE_H

/* a wide character is 16 bits in the game */
typedef unsigned short wint_t;

wint_t towlower(wint_t character);

#endif
