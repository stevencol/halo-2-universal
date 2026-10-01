/*
BITSTREAM.H

A stream of bits over a buffer, which the game's network code packs its
messages with. The bits of the stream go from the low bit of each 32-bit word
of the buffer to the high one.

Halo 1 has no such thing: the names of the functions are a guess.
*/

#ifndef __BITSTREAM_H
#define __BITSTREAM_H

#include "cseries/cseries.h"

/* ---------- structures */

struct bitstream
{
	unsigned long *data;
	/* of the data, in bytes */
	long size;
	long unknown8;
	long unknownC;
	/* in bits; it can go past the end of the data, and what is past the end
	is not written and is read as zeros */
	long position;
};

typedef char verify_bitstream_position_offset[
	offsetof(struct bitstream, position) == 0x10 ? 1 : -1];

/* ---------- prototypes/BITSTREAM.C */

void bitstream_write_bits(struct bitstream *stream, const void *source, long bit_count);
void bitstream_write_integer(struct bitstream *stream, unsigned long value, long bit_count);
boolean bitstream_read_bool(struct bitstream *stream);
void bitstream_read_bits(struct bitstream *stream, void *buffer, long bit_count);
unsigned long bitstream_read_integer(struct bitstream *stream, long bit_count);

#endif
