/*
UNITS.C

the bipeds and the vehicles: what they carry. Worked out from halo2ship.exe;
the name of the file is a guess, by the one that the first game has.
*/

#include "units.h"

#include "objects/objects.h"

/* ---------- code */

/* 0x000cbd50: one of the weapons that a unit carries, by its place; NONE
for the place NONE */
long unit_get_weapon(
	long unit_index,
	short weapon_index)
{
	const struct object_header_datum *header;

	if (weapon_index==NONE)
	{
		return NONE;
	}
	header= (const struct object_header_datum *)object_header_data->data+(unit_index&0xFFFF);

	return ((const long *)((const byte *)header->datum+UNIT_WEAPONS_OFFSET))[weapon_index];
}

/* 0x000c7100: the last of the chain that the units make by what each has at
0x24C, from the one after a unit; NONE if the unit has none there */
long unit_get_ultimate_unknown24C(
	long unit_index)
{
	const struct object_header_datum *headers= (const struct object_header_datum *)object_header_data->data;
	long next= *(const long *)((const byte *)headers[unit_index&0xFFFF].datum+UNIT_UNKNOWN24C_OFFSET);
	long last= NONE;

	while (next!=NONE)
	{
		last= next;
		next= *(const long *)((const byte *)headers[last&0xFFFF].datum+UNIT_UNKNOWN24C_OFFSET);
	}

	return last;
}

/* an object, from its index */
#define UNIT_OBJECT_GET(object_index) \
	((const byte *)((struct object_header_datum *)object_header_data->data)[(object_index)&0xFFFF].datum)

/* 0x000c8860: the byte that a unit keeps at 0x240, as a short */
short unit_get_unknown240(
	long unit_index)
{
	return (char)UNIT_OBJECT_GET(unit_index)[0x240];
}

/* 0x000cbf40: the bit 0 of what a unit keeps at 0x134 */
boolean unit_get_unknown134_bit0(
	long unit_index)
{
	return UNIT_OBJECT_GET(unit_index)[0x134]&1;
}

/* 0x000ccb80: the number that a unit keeps at 0x238 */
long unit_get_unknown238(
	long unit_index)
{
	return *(const long *)(UNIT_OBJECT_GET(unit_index)+0x238);
}

/* 0x000ce020: the byte that a unit keeps at 0x23C, as a short */
short unit_get_unknown23C(
	long unit_index)
{
	return (char)UNIT_OBJECT_GET(unit_index)[0x23C];
}

/* 0x000e5670: the byte that an object keeps at 0x3DC. It is far from the
others in the game: of a kind of unit, it seems, by how far in the byte is */
long unit_get_unknown3DC(
	long unit_index)
{
	return UNIT_OBJECT_GET(unit_index)[0x3DC];
}

/* 0x000c6f80: a unit keeps two numbers, at 0x1EC and 0x1F0 */
void unit_set_unknown1EC(
	long unit_index,
	long unknown1EC,
	long unknown1F0)
{
	byte *unit= (byte *)UNIT_OBJECT_GET(unit_index);

	*(long *)(unit+0x1EC)= unknown1EC;
	*(long *)(unit+0x1F0)= unknown1F0;
}

/* 0x000cdff0: one of the bytes that a unit keeps from 0x23E, as a short;
zero for NONE */
short unit_get_unknown23E(
	long unit_index,
	short index)
{
	if (index==NONE)
	{
		return 0;
	}

	return (char)UNIT_OBJECT_GET(unit_index)[index+0x23E];
}

/* 0x000def30: what an object keeps at 0x348, which takes the bit 7 of its
first byte. Of a kind of unit, by how far in it is */
byte *unit_get_unknown348(
	long unit_index)
{
	byte *unit= (byte *)UNIT_OBJECT_GET(unit_index);

	unit[0x348]|= 0x80;

	return unit+0x348;
}

/* 0x000e4020: whether an object has anything in its byte at 0x39D */
long unit_get_unknown39D(
	long unit_index)
{
	return UNIT_OBJECT_GET(unit_index)[0x39D]!=0;
}

/* ---------- more of what objects keep

The functions that follow are scattered in the game (0xe70b0 to 0x26be60):
each is of the file of a kind of object that is not known yet. What an object
has at the offset that its short at 0x346 gives is one of those things */

#define UNIT_UNKNOWN346(unit) ((byte *)(unit)+*(const short *)((const byte *)(unit)+0x346))

/* 0x000e70b0: the short at 0x36 of what the object has at 0x346 */
void unit_get_unknown346_unknown36(
	short *value,
	long unit_index)
{
	const byte *unit= UNIT_OBJECT_GET(unit_index);

	*value= *(const short *)(UNIT_UNKNOWN346(unit)+0x36);
}

/* 0x000e70e0: the byte at 0xC of it */
long unit_get_unknown346_unknownC(
	long unit_index)
{
	const byte *unit= UNIT_OBJECT_GET(unit_index);

	return (char)UNIT_UNKNOWN346(unit)[0xC];
}

/* 0x000ea1c0: the number at 0x2C of it is NONE */
void __attribute__((stdcall)) unit_clear_unknown346_unknown2C(
	long unit_index,
	long unknown)
{
	const byte *unit= UNIT_OBJECT_GET(unit_index);

	(void)unknown;
	*(long *)(UNIT_UNKNOWN346(unit)+0x2C)= NONE;
}

/* 0x000f8db0: an object keeps two numbers, at 0x144 and 0x148 */
void unit_set_unknown144(
	long unit_index,
	const long *unknown)
{
	byte *unit= (byte *)UNIT_OBJECT_GET(unit_index);

	*(long *)(unit+0x144)= unknown[0];
	*(long *)(unit+0x148)= unknown[1];
}

/* 0x000faa30: the short that an object keeps at 0x130 is at least a given
one */
void unit_raise_unknown130(
	long unit_index,
	short value)
{
	byte *unit= (byte *)UNIT_OBJECT_GET(unit_index);

	if (value>*(short *)(unit+0x130))
	{
		*(short *)(unit+0x130)= value;
	}
}

/* 0x00101f20: the number that an object keeps at 0x154, if it has the bit 0
of its byte at 0x12C; if not, NONE */
long unit_get_unknown154(
	long unit_index)
{
	const byte *unit= UNIT_OBJECT_GET(unit_index);

	if (unit[0x12C]&1)
	{
		return *(const long *)(unit+0x154);
	}

	return NONE;
}

/* 0x00119250: what an object keeps at 0x12C, which takes the bit 6 of its
first byte */
byte *unit_get_unknown12C(
	long unit_index)
{
	byte *unit= (byte *)UNIT_OBJECT_GET(unit_index);

	unit[0x12C]|= 0x40;

	return unit+0x12C;
}

/* 0x00207b90: an object takes the bit 16 of its flags; true. (Its two other
arguments are not used: it is given to something that calls it, it seems) */
boolean __attribute__((stdcall)) object_set_flag16(
	long object_index,
	long unknown2,
	long unknown3)
{
	(void)unknown2;
	(void)unknown3;
	*(unsigned long *)((byte *)UNIT_OBJECT_GET(object_index)+4)|= 0x10000;

	return TRUE;
}

/* 0x0026be60: whether an object whose byte at 0x3DC is 1 does not have the
bit 0 of its byte at 0x348; true for the others */
boolean unit_unknown_0026be60(
	long unit_index)
{
	const byte *unit= UNIT_OBJECT_GET(unit_index);

	if (unit[0x3DC]==1)
	{
		return !(unit[0x348]&1);
	}

	return TRUE;
}
