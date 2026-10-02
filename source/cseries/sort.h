/*
SORT.H
*/

#ifndef __SORT_H
#define __SORT_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	/* so many elements or fewer are sorted by taking the last of them, one
	at a time */
	MAXIMUM_SHORT_SORT_ELEMENTS= 8,
	/* the parts that are left to sort: there cannot be more, since the
	larger of two is the one that is left */
	MAXIMUM_SORT_PARTS= 30
};

/* ---------- types */

/* the game's functions that are given to other functions take their
arguments off the stack themselves */
#if defined(__i386__)
#define SORT_CALLBACK __attribute__((stdcall))
#else
#define SORT_CALLBACK
#endif

/* whether an element goes after another */
typedef boolean (SORT_CALLBACK *sort_compare_4byte_proc)(long element, long other, void *context);
typedef boolean (SORT_CALLBACK *sort_compare_2byte_proc)(word element, word other, void *context);

/* ---------- prototypes/SORT.C */

void qsort_4byte(long *elements, unsigned long element_count, void *unused, sort_compare_4byte_proc compare,
	void *context);
void qsort_2byte(word *elements, unsigned long element_count, void *unused, sort_compare_2byte_proc compare,
	void *context);
void short_sort_4byte(long *first, long *last, sort_compare_4byte_proc compare, void *context);
void short_sort_2byte(word *first, word *last, sort_compare_2byte_proc compare, void *context);

#endif
