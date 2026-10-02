/*
BINARY_SEARCH.H
*/

#ifndef __BINARY_SEARCH_H
#define __BINARY_SEARCH_H

/* ---------- types */

/* the game's functions that are given to other functions take their
arguments off the stack themselves */
#if defined(__i386__)
#define CALLBACK_CONVENTION __attribute__((stdcall))
#else
#define CALLBACK_CONVENTION
#endif

/* less than zero if the key goes before the element, zero if it is the
element, greater than zero if it goes after */
typedef long (CALLBACK_CONVENTION *binary_search_compare_proc)(const void *key, const void *element, void *context);
typedef long (CALLBACK_CONVENTION *binary_search_4byte_compare_proc)(long key, long element, void *context);

/* ---------- prototypes/BINARY_SEARCH.C */

long binary_search(long count, const void *key, const void *elements, long element_size,
	binary_search_compare_proc compare, void *context);
long binary_search_4byte(long count, long key, const long *elements, binary_search_4byte_compare_proc compare,
	void *context);

#endif
