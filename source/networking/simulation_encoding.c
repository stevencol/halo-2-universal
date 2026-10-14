/*
SIMULATION_ENCODING.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

Small functions that read from the network, or write to it, a piece of what
the consoles tell each other of the game: points, and numbers of a few bits;
and three that give the name of a kind of event. Most are named by the tables
that say how each kind of event and of thing is sent. They are of several
files in the game, which are not known: the name of this one is made up, and
so are the names of its functions.
*/

#include "cseries/cseries.h"

#include <string.h>

#include "math/real_math.h"
#include "networking/bitstream.h"
#include "scenario/scenario.h"

/* ---------- constants */

enum
{
	/* in the structure of the map: the box that the map is in */
	STRUCTURE_BSP_WORLD_BOUNDS_OFFSET= 0x34
};

/* 0x00440214: a box from -10 to 10 on each side, for the points that are
sent as they are from something else, and not as they are in the map */
static const real_bounds simulation_relative_bounds[3]=
{
	{-10.0f, 10.0f}, {-10.0f, 10.0f}, {-10.0f, 10.0f}
};

/* ---------- code */

/* the box that the map is in */
static inline __attribute__((always_inline)) const real_bounds *simulation_world_bounds(
	void)
{
	return (const real_bounds *)((const byte *)global_structure_bsp+STRUCTURE_BSP_WORLD_BOUNDS_OFFSET);
}

/* three numbers of some bits each, read as a point of a box */
static inline __attribute__((always_inline)) void simulation_read_point_inline(
	struct bitstream *stream,
	const real_bounds *bounds,
	long bits,
	real_point3d *point)
{
	long quantized[3];
	long axis;

	for (axis= 0; axis<3; axis++)
	{
		quantized[axis]= bitstream_read_integer(stream, bits);
	}
	dequantize_real_point3d(quantized, bounds, bits, point);
}

/* 0x00086e40: a point of the map is read, of some bits for each number */
void __attribute__((stdcall)) simulation_read_world_point(
	struct bitstream *stream,
	real_point3d *point,
	long bits)
{
	simulation_read_point_inline(stream, simulation_world_bounds(), bits, point);
}

/* 0x00086f50: a point of the box from -10 to 10 is read */
void __attribute__((stdcall)) simulation_read_relative_point(
	long bits,
	struct bitstream *stream,
	real_point3d *point)
{
	simulation_read_point_inline(stream, simulation_relative_bounds, bits, point);
}

/* 0x00086f10: a point of the box from -10 to 10 is written, with some bits
for each number */
void __attribute__((stdcall)) simulation_write_relative_point(
	struct bitstream *stream,
	long bits,
	const real_point3d *point)
{
	long quantized[3];
	long axis;

	quantize_real_point3d(point, simulation_relative_bounds, bits, quantized);
	for (axis= 0; axis<3; axis++)
	{
		bitstream_write_integer(stream, quantized[axis], bits);
	}
}

/* 0x00086e90: whether two points of the map would be sent almost as the
same: none of their three numbers, with some bits each, is more than one
away from the other's */
boolean __attribute__((stdcall)) simulation_world_points_are_close(
	long bits,
	const real_point3d *first,
	const real_point3d *second)
{
	long first_quantized[3];
	long second_quantized[3];
	long axis;

	quantize_real_point3d(first, simulation_world_bounds(), bits, first_quantized);
	quantize_real_point3d(second, simulation_world_bounds(), bits, second_quantized);
	for (axis= 0; axis<3; axis++)
	{
		long difference= first_quantized[axis]-second_quantized[axis];

		if (difference<0)
		{
			difference= -difference;
		}
		if (difference>1)
		{
			return FALSE;
		}
	}

	return TRUE;
}

/* ---------- functions that the tables name

Those that read take three things: one that they do not use, where what is
read goes, and the stream */

/* 0x0009ae80: a short of 5 bits; whether it is from 0 to 15 */
boolean __attribute__((stdcall)) simulation_decode_unknown_0009ae80(
	long unknown,
	short *value,
	struct bitstream *stream)
{
	short read= (short)bitstream_read_integer(stream, 5);

	(void)unknown;
	*value= read;

	return read>=0 && read<16;
}

/* 0x0009f240: a short of 1 bit; whether it is 0 or 1 */
boolean __attribute__((stdcall)) simulation_decode_unknown_0009f240(
	long unknown,
	short *value,
	struct bitstream *stream)
{
	short read= (short)bitstream_read_integer(stream, 1);

	(void)unknown;
	*value= read;

	return read>=0 && read<2;
}

/* 0x0009fa90: a short of 2 bits; whether it is 1 or 2 */
boolean __attribute__((stdcall)) simulation_decode_unknown_0009fa90(
	long unknown,
	short *value,
	struct bitstream *stream)
{
	short read= (short)bitstream_read_integer(stream, 2);

	(void)unknown;
	*value= read;

	return read>0 && read<3;
}

/* 0x0009ec30: a number of 5 bits; always true */
boolean __attribute__((stdcall)) simulation_decode_unknown_0009ec30(
	long unknown,
	long *value,
	struct bitstream *stream)
{
	(void)unknown;
	*value= bitstream_read_integer(stream, 5);

	return TRUE;
}

/* 0x0009fd90: a number of 2 bits; always true */
boolean __attribute__((stdcall)) simulation_decode_unknown_0009fd90(
	long unknown,
	long *value,
	struct bitstream *stream)
{
	(void)unknown;
	*value= bitstream_read_integer(stream, 2);

	return TRUE;
}

/* 0x0009ceb0: 16 bits; whether the stream had them and they are not NONE */
boolean __attribute__((stdcall)) simulation_decode_unknown_0009ceb0(
	long unknown,
	short *value,
	struct bitstream *stream)
{
	(void)unknown;
	bitstream_read_bits(stream, value, 16);
	if (stream->position>stream->size*8 || *value==NONE)
	{
		return FALSE;
	}

	return TRUE;
}

/* 0x0009cfa0: a bit that says whether 32 bits follow; if they do, they are
read and the flags that say what was read take their bit 0. Always true.
The first and the third thing are not used */
boolean __attribute__((stdcall)) simulation_decode_unknown_0009cfa0(
	long unknown1,
	unsigned long *flags,
	long unknown3,
	void *value,
	struct bitstream *stream)
{
	(void)unknown1;
	(void)unknown3;
	if (bitstream_read_bool(stream))
	{
		bitstream_read_bits(stream, value, 32);
		*flags|= 1;
	}

	return TRUE;
}

/* 0x000b29a0: a number of 32 bits. This one takes the stream first and where
the number goes last; whether the stream had it, nothing went wrong and it
is not less than zero */
boolean __attribute__((stdcall)) simulation_decode_unknown_000b29a0(
	struct bitstream *stream,
	long unknown,
	long *value)
{
	long read= bitstream_read_integer(stream, 32);
	boolean past_end;

	(void)unknown;
	*value= read;
	past_end= stream->position>stream->size*8;
	if (stream->error || past_end || read<0)
	{
		return FALSE;
	}

	return TRUE;
}

/* 0x0009ca70: the name of a kind of event, in a text of a size; the first
three things are not used */
char *__attribute__((stdcall)) simulation_describe_damage_section_response(
	long unknown1,
	long unknown2,
	long unknown3,
	unsigned long size,
	char *buffer)
{
	(void)unknown1;
	(void)unknown2;
	(void)unknown3;

	return csnzprintf(buffer, size, "damage section response");
}

/* 0x000a51f0: the same for another kind */
char *__attribute__((stdcall)) simulation_describe_game_engine(
	long unknown1,
	long unknown2,
	long unknown3,
	unsigned long size,
	char *buffer)
{
	(void)unknown1;
	(void)unknown2;
	(void)unknown3;

	return csnzprintf(buffer, size, "game engine");
}

/* 0x000a5780: and another */
char *__attribute__((stdcall)) simulation_describe_game_engine_request_boot_player(
	long unknown1,
	long unknown2,
	long unknown3,
	unsigned long size,
	char *buffer)
{
	(void)unknown1;
	(void)unknown2;
	(void)unknown3;

	return csnzprintf(buffer, size, "game engine request boot player");
}

/* 0x0009dcd0: the 0x28 bytes that something has at 0x68 are taken from
somewhere */
void __attribute__((stdcall)) simulation_unknown68_set(
	long unknown,
	byte *thing,
	const byte *source)
{
	(void)unknown;
	memcpy(thing+0x68, source, 0x28);
}

/* 0x000a0410: and are given to somewhere; always true */
boolean __attribute__((stdcall)) simulation_unknown68_get(
	long unknown,
	const byte *thing,
	byte *destination)
{
	(void)unknown;
	memcpy(destination, thing+0x68, 0x28);

	return TRUE;
}
