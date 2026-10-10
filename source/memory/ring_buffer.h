/*
RING_BUFFER.H

A ring of bytes: memory of a size in which what is kept starts at a place
and goes on, around the end, for so many bytes. The name of the file is a
guess, and so are the names of its functions.
*/

#ifndef __RING_BUFFER_H
#define __RING_BUFFER_H

#include "cseries/cseries.h"

/* ---------- structures */

struct ring_buffer
{
	long size;
	byte *data;
	/* where what is kept starts, and how many bytes it is */
	long start;
	long count;
};

/* ---------- prototypes/RING_BUFFER.C */

long __attribute__((stdcall)) ring_buffer_append(struct ring_buffer *ring, long count, const void *source);
void __attribute__((stdcall)) ring_buffer_write(struct ring_buffer *ring, long count, long offset, const void *source);
void __attribute__((stdcall)) ring_buffer_read(const struct ring_buffer *ring, long offset, long count,
	void *destination);

#endif
