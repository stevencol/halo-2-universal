/*
RING_BUFFER.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

A ring of bytes: memory of a size in which what is kept starts at a place
and goes on, around the end, for so many bytes. The name of the file is a
guess, and so are the names of its functions.
*/

#include "ring_buffer.h"

#include <string.h>

/* ---------- code */

/* 0x001a1900: so many bytes more are kept in a ring, after the ones that it
has; where they are in its memory, or NONE, with nothing done, if there is
no room for them */
long __attribute__((stdcall)) ring_buffer_append(
	struct ring_buffer *ring,
	long count,
	const void *source)
{
	long offset;

	if ((long)((unsigned long)ring->count+(unsigned long)count)>ring->size)
	{
		return NONE;
	}
	offset= (long)((unsigned long)ring->start+(unsigned long)ring->count)%ring->size;
	if (count>0)
	{
		ring_buffer_write(ring, count, offset, source);
	}
	ring->count+= count;

	return offset;
}

/* 0x001a1940: so many bytes are put in the memory of a ring, from a place
in it: up to its end, and the rest from its start */
void __attribute__((stdcall)) ring_buffer_write(
	struct ring_buffer *ring,
	long count,
	long offset,
	const void *source)
{
	long first= (long)((unsigned long)ring->size-(unsigned long)offset);
	long rest;

	if (first>count)
	{
		first= count;
	}
	rest= (long)((unsigned long)count-(unsigned long)first);
	if (first>0)
	{
		memcpy(ring->data+offset, source, first);
	}
	if (rest>0)
	{
		memcpy(ring->data, (const byte *)source+first, rest);
	}
}

/* 0x001a19a0: so many bytes are taken from the memory of a ring, from a
place in it: up to its end, and the rest from its start */
void __attribute__((stdcall)) ring_buffer_read(
	const struct ring_buffer *ring,
	long offset,
	long count,
	void *destination)
{
	long first= (long)((unsigned long)ring->size-(unsigned long)offset);
	long rest;

	if (first>count)
	{
		first= count;
	}
	rest= (long)((unsigned long)count-(unsigned long)first);
	if (first>0)
	{
		memcpy(destination, ring->data+offset, first);
	}
	if (rest>0)
	{
		memcpy((byte *)destination+first, ring->data, rest);
	}
}

/* 0x001a1a00: the pages (0x1000 bytes) of a range of something that starts
at its long at 0x04 lose their bits (at 0x10), and the count that it points
to at 0xFFC goes down by them */
void __attribute__((stdcall)) memory_unknown_001a1a00(
	byte *data,
	long address,
	long size,
	long unused)
{
	long offset= address-*(const long *)(data+0x04);
	long first= offset/0x1000;
	long last= (offset+size+0xFFF)/0x1000;
	long page;

	for (page= first; page<last; page++)
	{
		((unsigned long *)(data+0x10))[page>>5]&= ~(1UL<<(page&0x1F));
	}
	**(long *const *)(data+0xFFC)+= first-last;
}
