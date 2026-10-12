/*
UNITS.C

the bipeds and the vehicles: what they carry. Worked out from halo2ship.exe;
the name of the file is a guess, by the one that the first game has.
*/

#include "units.h"

#include "game/game.h"
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

void __attribute__((stdcall)) unknown_000b58c0(long entity_index, long unknown);

/* 0x000a7a30: unknown_000b58c0 is done with the entity of the simulation of
an object (at 0xD4), if it has one */
void object_unknown_000a7a30(
	long object_index,
	long unknown)
{
	long entity_index= *(const long *)(UNIT_OBJECT_GET(object_index)+0xD4);

	if (entity_index!=NONE)
	{
		unknown_000b58c0(entity_index, unknown);
	}
}

/* 0x000bc190: what an object is known by: the numbers that it has at 0xC4
and 0xC8, and its short at 0xC2 */
void object_get_unknownC4(
	long object_index,
	byte *unknown)
{
	const byte *object= UNIT_OBJECT_GET(object_index);

	*(long *)(unknown+4)= *(const long *)(object+0xC8);
	*(long *)unknown= *(const long *)(object+0xC4);
	*(short *)(unknown+8)= *(const short *)(object+0xC2);
}

/* 0x000c8a10: the bit 4 of the byte at 0x348 of an object of the type 0;
false for the others */
boolean unit_unknown_000c8a10(
	long unit_index)
{
	const byte *unit= UNIT_OBJECT_GET(unit_index);

	if (unit[0xAA]==0)
	{
		return (unit[0x348]>>4)&1;
	}

	return FALSE;
}

/* 0x000cb7e0: the three numbers that a unit keeps at 0x168 */
void unit_get_unknown168(
	long *unknown,
	long unit_index)
{
	const long *source= (const long *)(UNIT_OBJECT_GET(unit_index)+0x168);

	unknown[0]= source[0];
	unknown[1]= source[1];
	unknown[2]= source[2];
}

/* 0x000cd620: the first of the four places for weapons of a unit (from
0x218) that has none, or NONE if all have one */
short unit_get_free_weapon_index(
	long unit_index)
{
	const long *weapons= (const long *)(UNIT_OBJECT_GET(unit_index)+0x218);
	short index;

	for (index= 0; index<4; index++)
	{
		if (weapons[index]==NONE)
		{
			return index;
		}
	}

	return NONE;
}

/* 0x000cd660: whether both of the bytes that say which weapons a unit has in
its hands (0x212 and 0x213) say one */
boolean unit_has_two_weapons_in_hand(
	long unit_index)
{
	const byte *unit= UNIT_OBJECT_GET(unit_index);

	return unit[0x212]!=0xFF && unit[0x213]!=0xFF;
}

/* 0x000ceea0: the number that a unit keeps at 0x2A8 is NONE, if it was a
given one */
void __attribute__((stdcall)) unit_clear_unknown2A8(
	long unit_index,
	long unknown)
{
	byte *unit= (byte *)UNIT_OBJECT_GET(unit_index);

	if (*(long *)(unit+0x2A8)==unknown)
	{
		*(long *)(unit+0x2A8)= NONE;
	}
}

/* 0x000e4300: the short that an object keeps at 0x34C is NONE, and the two
bytes after it are zero */
void unit_clear_unknown34C(
	long unit_index)
{
	byte *unit= (byte *)UNIT_OBJECT_GET(unit_index);

	*(short *)(unit+0x34C)= NONE;
	unit[0x34E]= 0;
	unit[0x34F]= 0;
}

/* 0x004de2d0: a table of objects by a short that each keeps at 0xAC: NONE
where there is none */
long *unknown_004de2d0;

/* 0x000b8540: an object is deleted */
void __attribute__((stdcall)) unknown_000b8540(long object_index);

/* 0x00030c60: the three numbers that an object keeps at 0x30, and the one
after them */
void object_get_unknown30(
	long *unknown,
	long object_index,
	long *unknown3C)
{
	const byte *object= UNIT_OBJECT_GET(object_index);
	const long *source= (const long *)(object+0x30);

	unknown[0]= source[0];
	unknown[1]= source[1];
	unknown[2]= source[2];
	*unknown3C= *(const long *)(object+0x3C);
}

/* 0x000a0dd0: the short that an object keeps at 0x130 is at least 2; false */
boolean __attribute__((stdcall)) object_unknown_000a0dd0(
	long object_index)
{
	byte *object= (byte *)UNIT_OBJECT_GET(object_index);

	if (*(short *)(object+0x130)<2)
	{
		*(short *)(object+0x130)= 2;
	}

	return FALSE;
}

/* 0x000a7670: whether an object has an entity of the simulation, in a
console that only predicts what another rules; false in the others */
boolean object_unknown_000a7670(
	long object_index)
{
	if (main_game_globals->options.simulation==_game_simulation_predicted)
	{
		return *(const long *)(UNIT_OBJECT_GET(object_index)+0xD4)!=NONE;
	}

	return FALSE;
}

/* 0x000b8c00: the things of 0x34 bytes that an object has at the offset that
its short at 0x116 gives, and how many they are: the short at 0x114 is how
many bytes they take */
byte *object_get_unknown116_and_count(
	long object_index,
	long *count)
{
	byte *object= (byte *)UNIT_OBJECT_GET(object_index);

	*count= (long)((unsigned long)(long)*(const short *)(object+0x114)/0x34);

	return object+*(const short *)(object+0x116);
}

/* 0x000b9ce0: whether an object, or one of those that it is on, up to the
last, has the bit 4 of the flags of its place in the table or the bit 0 of
its own */
boolean object_unknown_000b9ce0(
	long object_index)
{
	for (;;)
	{
		const struct object_header_datum *header=
			&((const struct object_header_datum *)object_header_data->data)[object_index&0xFFFF];
		const byte *object= (const byte *)header->datum;

		if ((header->flags&0x10) || (object[4]&1))
		{
			return TRUE;
		}
		object_index= *(const long *)(object+0x14);
		if (object_index==NONE)
		{
			return FALSE;
		}
	}
}

/* 0x000bf050: an object takes a place of the table of unknown_004de2d0, if
no object has it: the object keeps which at 0xAC */
void object_unknown_000bf050(
	short index,
	long object_index)
{
	byte *object= (byte *)UNIT_OBJECT_GET(object_index);
	long *place= &unknown_004de2d0[index];

	if (*place==NONE)
	{
		*place= object_index;
		*(short *)(object+0xAC)= index;
	}
}

/* 0x000ccd20: the object that a unit has at 0x238 is deleted, and the unit
has none */
void unit_delete_unknown238(
	long unit_index)
{
	byte *unit= (byte *)UNIT_OBJECT_GET(unit_index);

	if (*(long *)(unit+0x238)!=NONE)
	{
		unknown_000b8540(*(long *)(unit+0x238));
		*(long *)(unit+0x238)= NONE;
	}
}

/* 0x000d0980: four things that a unit keeps from 0x2A0 are NONE */
void __attribute__((stdcall)) unit_clear_unknown2A0(
	long unit_index)
{
	byte *unit= (byte *)UNIT_OBJECT_GET(unit_index);

	*(long *)(unit+0x2A0)= NONE;
	*(short *)(unit+0x2A4)= NONE;
	*(long *)(unit+0x2A8)= NONE;
	*(long *)(unit+0x2AC)= NONE;
}

/* 0x000d0dc0: the number at 0x38 of what an object has at the offset that
its short at 0x33E gives is NONE, if it was a given one */
void unit_clear_unknown33E_unknown38(
	long unit_index,
	long unknown)
{
	byte *unit= (byte *)UNIT_OBJECT_GET(unit_index);
	byte *unknown30= unit+*(const short *)(unit+0x33E)+0x30;

	if (*(long *)(unknown30+8)==unknown)
	{
		*(long *)(unknown30+8)= NONE;
	}
}
