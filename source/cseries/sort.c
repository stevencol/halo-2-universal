/*
SORT.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

Sorting arrays of numbers of 4 and of 2 bytes, with a function that tells
whether one goes after another. The names of the file and of the two sorts
are Halo 1's; where the file goes is a guess.
*/

#include "sort.h"

/* ---------- code */

/* 0x0013de30: sorts numbers of 4 bytes: one is put where it goes, with the
ones that do not go after it before it and the others after it, and the same
is done with each of the two parts. The first argument on the stack is not
looked at */
void qsort_4byte(
	long *elements,
	unsigned long element_count,
	void *unused,
	sort_compare_4byte_proc compare,
	void *context)
{
	long *first_stack[MAXIMUM_SORT_PARTS];
	long *last_stack[MAXIMUM_SORT_PARTS];
	long part_count= 0;
	long *first= elements;
	long *last;

	(void)unused;

	if (element_count<2)
	{
		return;
	}
	last= elements+(element_count-1);

	for (;;)
	{
		unsigned long count= (unsigned long)(last-first)+1;

		if (count<=MAXIMUM_SHORT_SORT_ELEMENTS)
		{
			short_sort_4byte(first, last, compare, context);
		}
		else
		{
			long *middle= first+(count>>1);
			long *low= first;
			long *high= last+1;
			long element= *first;

			/* the one in the middle is the one that is put in its place */
			*first= *middle;
			*middle= element;
			for (;;)
			{
				do
				{
					low++;
				} while (low<=last && !compare(*low, *first, context));
				do
				{
					high--;
				} while (high>first && !compare(*first, *high, context));
				if (high<low)
				{
					break;
				}
				element= *low;
				*low= *high;
				*high= element;
			}
			element= *first;
			*first= *high;
			*high= element;

			/* the larger part is left for later, and the smaller one is
			sorted now */
			if ((long)(high-1-first)>=(long)(last-low))
			{
				if (first+1<high)
				{
					first_stack[part_count]= first;
					last_stack[part_count]= high-1;
					part_count++;
				}
				if (low<last)
				{
					first= low;
					continue;
				}
			}
			else
			{
				if (low<last)
				{
					first_stack[part_count]= low;
					last_stack[part_count]= last;
					part_count++;
				}
				if (first+1<high)
				{
					last= high-1;
					continue;
				}
			}
		}

		part_count--;
		if (part_count<0)
		{
			return;
		}
		first= first_stack[part_count];
		last= last_stack[part_count];
	}
}

/* 0x0013df80: sorts numbers of 2 bytes, as qsort_4byte does */
void qsort_2byte(
	word *elements,
	unsigned long element_count,
	void *unused,
	sort_compare_2byte_proc compare,
	void *context)
{
	word *first_stack[MAXIMUM_SORT_PARTS];
	word *last_stack[MAXIMUM_SORT_PARTS];
	long part_count= 0;
	word *first= elements;
	word *last;

	(void)unused;

	if (element_count<2)
	{
		return;
	}
	last= elements+(element_count-1);

	for (;;)
	{
		unsigned long count= (unsigned long)(last-first)+1;

		if (count<=MAXIMUM_SHORT_SORT_ELEMENTS)
		{
			short_sort_2byte(first, last, compare, context);
		}
		else
		{
			word *middle= first+(count>>1);
			word *low= first;
			word *high= last+1;
			word element= *first;

			*first= *middle;
			*middle= element;
			for (;;)
			{
				do
				{
					low++;
				} while (low<=last && !compare(*low, *first, context));
				do
				{
					high--;
				} while (high>first && !compare(*first, *high, context));
				if (high<low)
				{
					break;
				}
				element= *low;
				*low= *high;
				*high= element;
			}
			element= *first;
			*first= *high;
			*high= element;

			if ((long)(high-1-first)>=(long)(last-low))
			{
				if (first+1<high)
				{
					first_stack[part_count]= first;
					last_stack[part_count]= high-1;
					part_count++;
				}
				if (low<last)
				{
					first= low;
					continue;
				}
			}
			else
			{
				if (low<last)
				{
					first_stack[part_count]= low;
					last_stack[part_count]= last;
					part_count++;
				}
				if (first+1<high)
				{
					last= high-1;
					continue;
				}
			}
		}

		part_count--;
		if (part_count<0)
		{
			return;
		}
		first= first_stack[part_count];
		last= last_stack[part_count];
	}
}

/* 0x0013e0e0: sorts a few numbers of 4 bytes, from the first to the last:
the one that goes after all the others is put at the end, and the same is
done with those that are left */
void short_sort_4byte(
	long *first,
	long *last,
	sort_compare_4byte_proc compare,
	void *context)
{
	while (last>first)
	{
		long *greatest= first;
		long *element;
		long value;

		for (element= first+1; element<=last; element++)
		{
			if (compare(*element, *greatest, context))
			{
				greatest= element;
			}
		}
		value= *greatest;
		*greatest= *last;
		*last= value;
		last--;
	}
}

/* 0x0013e140: sorts a few numbers of 2 bytes, as short_sort_4byte does */
void short_sort_2byte(
	word *first,
	word *last,
	sort_compare_2byte_proc compare,
	void *context)
{
	while (last>first)
	{
		word *greatest= first;
		word *element;
		word value;

		for (element= first+1; element<=last; element++)
		{
			if (compare(*element, *greatest, context))
			{
				greatest= element;
			}
		}
		value= *greatest;
		*greatest= *last;
		*last= value;
		last--;
	}
}
