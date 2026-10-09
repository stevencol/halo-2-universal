/*
BITSTREAM.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The functions that write do not clear the bits they write to: they take the
data to be zeros from the position on. The functions that copy whole words
read, and write, up to the end of the word that the last bit is in.
*/

#include "bitstream.h"

#include <string.h>

/* ---------- macros */

/* the processor takes the low 5 bits of a count of bits to shift by */
#define SHIFT_COUNT(count) ((count)&31)

/* ---------- code */

/* the bits from the position to the end of the data, or the count if it is
less */
static long bitstream_bits_available(
	const struct bitstream *stream,
	long bit_count)
{
	long available= stream->size*8 - stream->position;

	return available>bit_count ? bit_count : available;
}

/* 0x001955d0 */
void bitstream_write_bits(
	struct bitstream *stream,
	const void *source,
	long bit_count)
{
	long position= stream->position;
	long count= bitstream_bits_available(stream, bit_count);

	if (count>0)
	{
		long end;

		if (position%8==0)
		{
			/* at a whole byte: the bytes are copied */
			memcpy((byte *)stream->data+(position>>3), source, (count+7)>>3);

			end= stream->position+count;
			if (end&7)
			{
				((byte *)stream->data)[end>>3]&= 0xFF>>(8-(end&7));
			}
		}
		else
		{
			long shift= position%32;
			const unsigned long *words= source;
			unsigned long *destination= stream->data+(position>>5);
			unsigned long *destination_end= stream->data+((position+count-1)>>5)+1;

			*destination++|= *words<<SHIFT_COUNT(shift);
			while (destination<destination_end)
			{
				*destination= *words>>SHIFT_COUNT(32-shift);
				words++;
				*destination++|= *words<<SHIFT_COUNT(shift);
			}

			end= stream->position+count;
			if (end&31)
			{
				stream->data[end>>5]&= 0xFFFFFFFF>>SHIFT_COUNT(32-(end&31));
			}
		}
	}
	stream->position+= bit_count;

	return;
}

/* 0x00195720 */
void bitstream_write_integer(
	struct bitstream *stream,
	unsigned long value,
	long bit_count)
{
	long position= stream->position;
	long count= bitstream_bits_available(stream, bit_count);

	if (count>0)
	{
		long word_index= position>>5;
		long shift= position&31;

		value&= 0xFFFFFFFF>>SHIFT_COUNT(32-count);
		stream->data[word_index]|= value<<shift;
		if (word_index!=(position+count-1)>>5)
		{
			/* the bits that did not fit in the word */
			stream->data[word_index+1]= value>>SHIFT_COUNT(32-shift);
		}
	}
	stream->position+= bit_count;

	return;
}

/* 0x001957d0: a position at the end of the data is read too */
boolean bitstream_read_bool(
	struct bitstream *stream)
{
	long position= stream->position;
	boolean result= FALSE;

	if (position<=stream->size*8)
	{
		result= (((byte *)stream->data)[position/8] & (byte)(1<<SHIFT_COUNT(position%8)))!=0;
	}
	stream->position= position+1;

	return result;
}

/* 0x00195820: the bits of the buffer after the last one read keep what they
had, in the last byte; the bytes after it, in the last word, too */
void bitstream_read_bits(
	struct bitstream *stream,
	void *buffer,
	long bit_count)
{
	long position= stream->position;
	long count= bitstream_bits_available(stream, bit_count);

	if (count>0)
	{
		if (position%8==0)
		{
			/* at a whole byte: the bytes are copied */
			memcpy(buffer, (byte *)stream->data+(position>>3), (count+7)>>3);
			if (count&7)
			{
				((byte *)buffer)[count>>3]&= 0xFF>>(8-(count&7));
			}
		}
		else
		{
			long shift= position%32;
			const unsigned long *words= stream->data+(position>>5);
			unsigned long *destination= buffer;
			unsigned long *destination_end= destination+((count-1)>>5)+1;
			long last_word_bit_count= count&31;
			long last_word_index= count>>5;
			unsigned long kept= 0;

			if (last_word_bit_count>0)
			{
				long last_word_byte_count= (last_word_bit_count+7)>>3;

				if (last_word_byte_count<4)
				{
					kept= (0xFFFFFFFF<<(last_word_byte_count*8)) & destination[last_word_index];
				}
			}

			while (destination<destination_end)
			{
				*destination= *words>>SHIFT_COUNT(shift);
				words++;
				*destination++|= *words<<SHIFT_COUNT(32-shift);
			}

			if (last_word_bit_count)
			{
				destination= buffer;
				destination[last_word_index]=
					((0xFFFFFFFF>>SHIFT_COUNT(32-last_word_bit_count)) & destination[last_word_index]) | kept;
			}
		}
	}
	stream->position+= bit_count;

	return;
}

/* 0x001959c0 */
unsigned long bitstream_read_integer(
	struct bitstream *stream,
	long bit_count)
{
	long position= stream->position;
	long count= bitstream_bits_available(stream, bit_count);
	unsigned long result= 0;

	if (count>0)
	{
		long word_index= position>>5;
		long shift= position&31;

		if (word_index==(position+count-1)>>5)
		{
			result= stream->data[word_index]>>shift;
		}
		else
		{
			result= stream->data[word_index+1]<<SHIFT_COUNT(32-shift) | stream->data[word_index]>>shift;
		}
		result&= 0xFFFFFFFF>>SHIFT_COUNT(32-count);
	}
	stream->position+= bit_count;

	return result;
}

/* 0x001946f0: whether a stream is past the end of its data, or what is read
started with the word of a build for testing */
boolean bitstream_overflowed(
	const struct bitstream *stream)
{
	boolean overflowed= stream->position>(long)((unsigned long)stream->size<<3);

	if (stream->error)
	{
		overflowed= TRUE;
	}

	return overflowed;
}

/* 0x001947a0: a stream is to be read, from the start, with no positions
kept. If its first word is the one of a build for testing, that is noted and
it is read from after it; if not, from the start */
void bitstream_begin_reading(
	struct bitstream *stream)
{
	stream->unknownC= _bitstream_reading;
	stream->position= 0;
	stream->position_count= 0;
	stream->error= FALSE;
	if (bitstream_read_integer(stream, 32)==BITSTREAM_DEBUG_SIGNATURE)
	{
		stream->error= TRUE;
	}
	else
	{
		stream->position= 0;
		stream->error= FALSE;
	}
}

/* 0x00194830: writes a bit, if there is room for it */
void __attribute__((stdcall)) bitstream_write_bool(
	struct bitstream *stream,
	boolean value)
{
	long position= stream->position;

	if ((long)(((unsigned long)stream->size<<3)-(unsigned long)position)>=1 && value)
	{
		((byte *)stream->data)[position/8]|= (byte)(1<<SHIFT_COUNT(position%8));
	}
	stream->position= (long)((unsigned long)stream->position+1);
}

/* 0x001947e0: writes a number in so many bits. One that does not fit in them
has a line of text made for it, which nothing is done with */
void __attribute__((stdcall)) bitstream_write_integer_checked(
	unsigned long value,
	long bit_count,
	struct bitstream *stream)
{
	if (bit_count<32 && value>=1u<<SHIFT_COUNT(bit_count))
	{
		char text[256];

		text[0]= 0;
		csprintf256(text, "%u exceeds max value of %u", value, 1u<<SHIFT_COUNT(bit_count));
	}
	bitstream_write_integer(stream, value, bit_count);
}

/* 0x00194710: the last position that a stream kept is not kept any more;
and with a byte, the stream goes back to it. One that is being written then
has zeros from there on, and notes how many bits were taken back, and one
more thing taken back */
void __attribute__((stdcall)) bitstream_pop_position(
	struct bitstream *stream,
	boolean go_back)
{
	long position= stream->positions[--stream->position_count];

	if (go_back)
	{
		if (stream->unknownC==_bitstream_writing && position<stream->position)
		{
			long byte_index= position/8;
			long rest;

			stream->unknown2C= stream->position-position;
			stream->unknown30++;
			if (byte_index<stream->size)
			{
				((byte *)stream->data)[byte_index]&= (byte)((1<<SHIFT_COUNT(position%8))-1);
			}
			rest= stream->size-byte_index-1;
			if (rest>0)
			{
				memset((byte *)stream->data+byte_index+1, 0, rest);
			}
		}
		stream->position= position;
	}
}

/* 0x00194fa0: reads a text of characters of 16 bits, up to the one that
ends it, into room for so many. One that does not end in that room is cut
short there, and that is an error of the stream */
void __attribute__((stdcall)) bitstream_read_wide_string(
	struct bitstream *stream,
	word *string,
	long maximum_count)
{
	long index;

	for (index= 0; index<maximum_count; index++)
	{
		word character= (word)bitstream_read_integer(stream, 16);

		string[index]= character;
		if (character==0)
		{
			return;
		}
	}
	string[maximum_count-1]= 0;
	stream->error= TRUE;
}