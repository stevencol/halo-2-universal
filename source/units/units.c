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
