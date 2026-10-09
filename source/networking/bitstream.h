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

/* ---------- constants */

enum
{
	MAXIMUM_BITSTREAM_POSITIONS= 4,
	/* what a stream is doing: being written, and being read */
	_bitstream_writing= 1,
	_bitstream_reading= 3,
	/* 'debg': what a build for testing starts what it writes with */
	BITSTREAM_DEBUG_SIGNATURE= 0x64656267
};

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
	/* something went wrong with what is read: it starts with the word that
	only a build for testing writes, or a text has no end */
	boolean error;
	/* positions that were kept to go back to, the last one last */
	long position_count;
	long positions[MAXIMUM_BITSTREAM_POSITIONS];
	/* how many bits the last thing that was taken back was, and how many
	things were */
	long unknown2C;
	long unknown30;
};

typedef char verify_bitstream_position_offset[
	offsetof(struct bitstream, position) == 0x10 ? 1 : -1];

/* ---------- prototypes/BITSTREAM.C */

void bitstream_write_bits(struct bitstream *stream, const void *source, long bit_count);
void bitstream_write_integer(struct bitstream *stream, unsigned long value, long bit_count);
boolean bitstream_read_bool(struct bitstream *stream);
void bitstream_read_bits(struct bitstream *stream, void *buffer, long bit_count);
unsigned long bitstream_read_integer(struct bitstream *stream, long bit_count);
boolean bitstream_overflowed(const struct bitstream *stream);
void bitstream_begin_reading(struct bitstream *stream);
void __attribute__((stdcall)) bitstream_write_bool(struct bitstream *stream, boolean value);
void __attribute__((stdcall)) bitstream_write_integer_checked(unsigned long value, long bit_count,
	struct bitstream *stream);
void __attribute__((stdcall)) bitstream_pop_position(struct bitstream *stream, boolean go_back);
void __attribute__((stdcall)) bitstream_read_wide_string(struct bitstream *stream, word *string, long maximum_count);

#endif
