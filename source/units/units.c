/*
UNITS.C

the bipeds and the vehicles: what they carry. Worked out from halo2ship.exe;
the name of the file is a guess, by the one that the first game has.
*/

#include "units.h"

#include <stdint.h>

#include "cache/cache_files.h"
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

/* the definition of an object (its tag): the object starts with which */
#define UNIT_DEFINITION_GET(object) \
	((const byte *)(uintptr_t)global_tag_instances[*(const long *)(object)&0xFFFF].address)

/* 0x0009dc90: as unit_clear_unknown2A0, and true */
boolean __attribute__((stdcall)) unit_unknown_0009dc90(
	long unit_index)
{
	byte *unit= (byte *)UNIT_OBJECT_GET(unit_index);

	*(long *)(unit+0x2A0)= NONE;
	*(short *)(unit+0x2A4)= NONE;
	*(long *)(unit+0x2A8)= NONE;
	*(long *)(unit+0x2AC)= NONE;

	return TRUE;
}

/* 0x000a09b0: false for an object whose byte at 0xAB is 2 and that has the
bit 2 of its byte at 0x10A; true for the others */
boolean __attribute__((stdcall)) object_unknown_000a09b0(
	long object_index)
{
	const byte *object= UNIT_OBJECT_GET(object_index);

	return !(object[0xAB]==2 && ((object[0x10A]>>2)&1));
}

/* 0x000cc380: whether the object that a unit has at 0x248, or the unit
itself if it has none, has something at 0x12C */
long unit_unknown_000cc380(
	long unit_index)
{
	const byte *unit= UNIT_OBJECT_GET(unit_index);

	if (*(const long *)(unit+0x248)!=NONE)
	{
		unit= UNIT_OBJECT_GET(*(const long *)(unit+0x248));
	}

	return *(const long *)(unit+0x12C)!=NONE;
}

/* 0x000f0f90: the bit 7 of the number at 0x1EC of the definition of an
object */
boolean object_definition_get_unknown1EC_bit7(
	long object_index)
{
	return (*(const unsigned long *)(UNIT_DEFINITION_GET(UNIT_OBJECT_GET(object_index))+0x1EC)>>7)&1;
}

/* 0x000f12e0: whether the short at 0x1F0 of the definition of an object is
not 6 */
boolean object_definition_unknown1F0_is_not_6(
	long object_index)
{
	return *(const short *)(UNIT_DEFINITION_GET(UNIT_OBJECT_GET(object_index))+0x1F0)!=6;
}

/* 0x000fa9f0: the number that an object keeps at 0x144 is NONE, and the
short after it, if the number was a given one */
void __attribute__((stdcall)) unit_clear_unknown144(
	long unit_index,
	long unknown)
{
	byte *unit= (byte *)UNIT_OBJECT_GET(unit_index);

	if (*(long *)(unit+0x144)==unknown)
	{
		*(long *)(unit+0x144)= NONE;
		*(short *)(unit+0x148)= NONE;
	}
}

/* 0x00100fd0: whether the short at 0x1FE of the definition of an object is
above zero */
boolean object_definition_has_unknown1FE(
	long object_index)
{
	return *(const short *)(UNIT_DEFINITION_GET(UNIT_OBJECT_GET(object_index))+0x1FE)>0;
}

/* 0x00101240: the bit 3 of the number at 0x12C of the definition of an
object (by where these are in the game, the objects are the weapons) */
boolean object_definition_get_unknown12C_bit3(
	long object_index)
{
	return (*(const unsigned long *)(UNIT_DEFINITION_GET(UNIT_OBJECT_GET(object_index))+0x12C)>>3)&1;
}

/* 0x00101280: the short at 0x290 of the definition of an object */
short object_definition_get_unknown290(
	long object_index)
{
	return *(const short *)(UNIT_DEFINITION_GET(UNIT_OBJECT_GET(object_index))+0x290);
}

/* 0x001012c0: the bit 0 of the byte at 0x12F of the definition of an object;
false for no object */
boolean object_definition_get_unknown12F_bit0(
	long object_index)
{
	if (object_index!=NONE)
	{
		return UNIT_DEFINITION_GET(UNIT_OBJECT_GET(object_index))[0x12F]&1;
	}

	return FALSE;
}

/* 0x00101300: the bit 28 of the number at 0x12C of the definition of an
object */
boolean object_definition_get_unknown12C_bit28(
	long object_index)
{
	return (*(const unsigned long *)(UNIT_DEFINITION_GET(UNIT_OBJECT_GET(object_index))+0x12C)>>28)&1;
}

/* ---------- of the weapons, by where they are in the game (0x101e80 to
0x105be0); the file of the weapons is not started */

/* 0x00101e80: whether a weapon has nothing in its bytes at 0x20C and 0x218
and its number at 0x178 is zero */
long weapon_unknown_00101e80(
	long weapon_index)
{
	const byte *weapon= UNIT_OBJECT_GET(weapon_index);

	return !weapon[0x20C] && !weapon[0x218] && *(const long *)(weapon+0x178)==0;
}

/* 0x001029d0: whether the short that starts one of the things of 0x10 bytes
that a weapon has from 0x224 is 2 */
long weapon_unknown_001029d0(
	long weapon_index,
	short index)
{
	return *(const short *)(UNIT_OBJECT_GET(weapon_index)+index*0x10+0x224)==2;
}

/* 0x00102f70: one of the two things of 0x34 bytes that a weapon has from
0x1A4 (its triggers, it seems) loses the bit 6 of its byte at 4, unless its
byte at 1 is 1 */
void weapon_unknown_00102f70(
	long weapon_index,
	short index)
{
	if (index>=0 && index<2)
	{
		byte *unknown= (byte *)UNIT_OBJECT_GET(weapon_index)+index*0x34+0x1A4;

		if (unknown[1]!=1)
		{
			unknown[4]&= ~0x40;
		}
	}
}

/* 0x00105be0: four things that a weapon keeps from 0x250 are as they start:
NONE, two shorts that are NONE, and a real that is zero */
void weapon_clear_unknown250(
	long weapon_index)
{
	byte *weapon= (byte *)UNIT_OBJECT_GET(weapon_index);

	*(short *)(weapon+0x254)= NONE;
	*(short *)(weapon+0x256)= NONE;
	*(float *)(weapon+0x258)= 0.0f;
	*(long *)(weapon+0x250)= NONE;
}

/* 0x0010a0b0: the short at 0xC0 of the definition of an object */
short object_definition_get_unknownC0(
	long object_index)
{
	return *(const short *)(UNIT_DEFINITION_GET(UNIT_OBJECT_GET(object_index))+0xC0);
}

/* 0x0010cf50: whether an object has the bit 0 of its byte at 0x12C and not
the bit 1 */
boolean object_unknown_0010cf50(
	long object_index)
{
	byte flags= UNIT_OBJECT_GET(object_index)[0x12C];

	return (flags&1) && !((flags>>1)&1);
}

/* 0x0010f5f0: the number at 0x7C of what an object has at the offset that
its short at 0x12A gives, if three things of it are not NONE; NONE if not */
long object_unknown_0010f5f0(
	long object_index)
{
	const byte *object= UNIT_OBJECT_GET(object_index);
	const byte *unknown= object+*(const short *)(object+0x12A);

	if (*(const long *)(unknown+0x68)!=NONE && *(const long *)unknown!=NONE && *(const short *)(unknown+6)!=NONE)
	{
		return *(const long *)(unknown+0x7C);
	}

	return NONE;
}

/* 0x0010f8f0: the number at 8 of what an object has at 0x9C of the offset
that its short at 0x33E gives, if two things of it are not NONE */
long object_unknown_0010f8f0(
	long object_index)
{
	const byte *object= UNIT_OBJECT_GET(object_index);
	const byte *unknown= object+*(const short *)(object+0x33E)+0x9C;

	if (*(const long *)unknown!=NONE && *(const short *)(unknown+6)!=NONE)
	{
		return *(const long *)(unknown+8);
	}

	return NONE;
}

/* 0x00118fe0: the bit 0 of the byte that an object has at 0x12C is set or
cleared; it is always cleared for an object that has the bit 2 of its byte
at 0x10A */
void object_set_unknown12C_bit0(
	long object_index,
	boolean value)
{
	byte *object= (byte *)UNIT_OBJECT_GET(object_index);

	if ((object[0x10A]>>2)&1)
	{
		value= FALSE;
	}
	if (value)
	{
		object[0x12C]|= 1;
	}
	else
	{
		object[0x12C]&= ~1;
	}
}

/* 0x0011a960: the bit 18 of the number at 8 of what an object has at 0x346,
for an object that there is and that does not have the bit 2 of its byte at
0x10A; false for the others */
boolean unit_unknown_0011a960(
	long unit_index)
{
	if (unit_index!=NONE)
	{
		const byte *unit= UNIT_OBJECT_GET(unit_index);

		if (!((unit[0x10A]>>2)&1))
		{
			return (*(const unsigned long *)(UNIT_UNKNOWN346(unit)+8)>>18)&1;
		}
	}

	return FALSE;
}

/* 0x002013c0: whether an object has neither that bit 2 nor that bit 18 */
boolean unit_unknown_002013c0(
	long unit_index)
{
	const byte *unit= UNIT_OBJECT_GET(unit_index);

	if ((unit[0x10A]>>2)&1)
	{
		return FALSE;
	}
	if ((*(const unsigned long *)(UNIT_UNKNOWN346(unit)+8)>>18)&1)
	{
		return FALSE;
	}

	return TRUE;
}

/* 0x0011ba10: the number at 0x124 of the definition of an object */
long object_definition_get_unknown124(
	long object_index)
{
	return *(const long *)(UNIT_DEFINITION_GET(UNIT_OBJECT_GET(object_index))+0x124);
}

/* 0x0020bb70: whether the short at 0x1F0 of the definition of an object is 6 */
boolean object_definition_unknown1F0_is_6(
	long object_index)
{
	return *(const short *)(UNIT_DEFINITION_GET(UNIT_OBJECT_GET(object_index))+0x1F0)==6;
}

/* 0x0015b1e0: an object has 0xFF for its byte at 0xAF, and, unless this
console only predicts, one of the things of 8 bytes of a table starts with
NONE */
void object_unknown_0015b1e0(
	long object_index,
	long *table,
	long index)
{
	byte *object= (byte *)UNIT_OBJECT_GET(object_index);

	if (main_game_globals->options.simulation!=_game_simulation_predicted)
	{
		table[index*2]= NONE;
	}
	object[0xAF]= 0xFF;
}

/* 0x001794a0: whether the place of an object in the table has the bit 0 of
its flags and another object is not as object_unknown_000b9ce0 says */
boolean object_unknown_001794a0(
	long object_index,
	long other_object_index)
{
	if (((const struct object_header_datum *)object_header_data->data)[object_index&0xFFFF].flags&1)
	{
		if (!object_unknown_000b9ce0(other_object_index))
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* 0x001b8c80: the last of the objects of the type 1, from an object up
through those that each is on, before one that is not of that type; NONE if
the first is not */
long object_unknown_001b8c80(
	long object_index)
{
	long result= NONE;

	while (object_index!=NONE)
	{
		const byte *object= UNIT_OBJECT_GET(object_index);

		if (object[0xAA]!=1)
		{
			break;
		}
		result= object_index;
		object_index= *(const long *)(object+0x14);
	}

	return result;
}

/* 0x0051e9a0, of havok.c: its number counts the objects that have the bit
0 of their short at 0xC0 */
extern long *unknown_0051e9a0;

/* 0x001c3980: an object takes the bit 0 of its short at 0xC0, and is counted,
if it did not have it. It is among those of the physics in the game */
void object_unknown_001c3980(
	long object_index)
{
	byte *object= (byte *)UNIT_OBJECT_GET(object_index);

	if (!(object[0xC0]&1))
	{
		*(short *)(object+0xC0)|= 1;
		(*unknown_0051e9a0)++;
	}
}

/* 0x0011c080: whether the bits that the game has at 0x1138 have the one
that something names with its short at 4 */
long game_unknown1138_test(
	const byte *unknown)
{
	long bit= *(const short *)(unknown+4);

	return (*(const unsigned long *)((const byte *)main_game_globals+0x1138+(bit>>5)*4)&(1<<(bit&0x1F)))!=0;
}

/* 0x0051e9c4, of game_unknown_001eb550.c: it starts with a real number */
extern unsigned long *unknown_0051e9c4;

/* 0x000fa170: the real number at 0x164 of a tag, times that number, with the
sign turned */
float tag_get_unknown164_scaled(
	long tag_index)
{
	const byte *tag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;

	return 0.0f-*(const float *)(tag+0x164)**(const float *)unknown_0051e9c4;
}

/* 0x00510c6c, of game.c; 0x004c1bd0, of game_unknown_0002b4a0.c */
struct unknown_00510c6c;
extern struct unknown_00510c6c *unknown_00510c6c;
extern long unknown_004c1bd0;

/* 0x0016c2b0: the unit that unknown_00510c6c names at 0x3C, if it has its
first byte and 4 for its short at 2, and the unit is there; NONE if not */
long unknown_00510c6c_get_unit(
	void)
{
	const byte *unknown= (const byte *)unknown_00510c6c;

	if (unknown[0] && *(const short *)(unknown+2)==4)
	{
		long unit_index= *(const long *)(unknown+0x3C);

		if (object_try_and_get_and_verify_type(unit_index, 3))
		{
			return unit_index;
		}
	}

	return NONE;
}

/* 0x0016c6f0: the first of the matrices that an object has at the place that
its short at 0x116 says (those of its nodes, most likely); the matrix that
changes nothing if the object is not there */
void object_get_first_node_matrix(
	real_matrix4x3 *matrix,
	long object_index)
{
	if (object_try_and_get_and_verify_type(object_index, 0xFFFFFFFF))
	{
		const byte *object= UNIT_OBJECT_GET(object_index);

		*matrix= *(const real_matrix4x3 *)(object+*(const short *)(object+0x116));
	}
	else
	{
		*matrix= *global_identity4x3;
	}
}

/* 0x00175f50: whether a biped that has the bit 2 of its byte at 0x10A has a
time at 0x2E0 that is more than a second old */
boolean biped_unknown_00175f50(
	long object_index)
{
	const byte *biped= object_try_and_get_and_verify_type(object_index, 1);

	if (biped && ((biped[0x10A]>>2)&1))
	{
		long time= *(const long *)(biped+0x2E0);

		if (time!=NONE && game_time_globals->ticks_per_second+time<(long)game_time_globals->time)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* 0x001518f0: whether the object that something names at 4 is of one of
some types */
boolean unknown_object_is_of_types(
	const byte *unknown,
	unsigned long type_mask)
{
	long object_index= *(const long *)(unknown+4);

	if (object_get_and_verify_type(object_index, 0xFFFFFFFF))
	{
		byte type= ((const struct object_header_datum *)object_header_data->data)[object_index&0xFFFF].type;

		return ((1u<<(type&31))&type_mask)!=0;
	}

	return FALSE;
}

/* 0x0003d7c0: whether object_unknown_000b9ce0 says no of an object; and, to
where the caller says, if it says where, whether then the object is
unknown_004c1bd0 */
boolean object_unknown_0003d7c0(
	long object_index,
	boolean *is_unknown_004c1bd0)
{
	boolean result= FALSE;
	boolean value= FALSE;

	if (!object_unknown_000b9ce0(object_index))
	{
		value= object_index==unknown_004c1bd0;
		result= TRUE;
	}
	if (is_unknown_004c1bd0)
	{
		*is_unknown_004c1bd0= value;
	}

	return result;
}

/* 0x000fa9a0: the first object of the type 5 that there is; whether there
is one */
boolean __attribute__((stdcall)) objects_get_first_of_type5(
	long *object_index)
{
	struct object_iterator iterator;

	iterator.signature= OBJECT_ITERATOR_SIGNATURE;
	iterator.type_mask= 0x20;
	iterator.header_flags= 0;
	iterator.absolute_index= 0;
	iterator.index= NONE;
	if (object_iterator_next(&iterator))
	{
		*object_index= iterator.index;

		return TRUE;
	}

	return FALSE;
}

/* 0x000bb760: one of the 0x280 numbers of the table of unknown_004de2d0;
NONE if it is not one of them */
long unknown_004de2d0_get(
	short index)
{
	if (index>=0 && index<0x280)
	{
		return unknown_004de2d0[index];
	}

	return NONE;
}

/* 0x00118f70: the bit 1 of the byte that an object has at 0x12C */
boolean object_get_unknown12C_bit1(
	long object_index)
{
	return (UNIT_OBJECT_GET(object_index)[0x12C]>>1)&1;
}

/* 0x00114480: the number that an object has at the place that its short at
0x342 says */
long object_get_unknown342_first(
	long object_index)
{
	const byte *object= UNIT_OBJECT_GET(object_index);

	return *(const long *)(object+*(const short *)(object+0x342));
}

/* 0x004ba014: some bits of something */
unsigned long unknown_004ba014;

/* 0x00030e00: zero if unknown_004ba014 has its bits 0 and 1 and the caller
says yes; NONE if not. It is after object_get_unknown30 in the game; whether
it is of this file is not known */
long __attribute__((stdcall)) unknown_004ba014_unknown_00030e00(
	boolean value)
{
	if ((unknown_004ba014&1) && ((unknown_004ba014>>1)&1) && value)
	{
		return 0;
	}

	return NONE;
}
/* 0x00030ca0: sets or clears a bit of unknown_004ba014 */
void unknown_004ba014_set_bit(
	boolean value,
	byte bit)
{
	if (value)
	{
		unknown_004ba014|= 1<<bit;
	}
	else
	{
		unknown_004ba014&= ~(1<<bit);
	}
}

/* 0x00030cd0: NONE if the first byte is set; if not, 2 if the second is
not, 0 if unknown_004ba014 has its bit 4 and not its bit 5, and 1 if not */
long __attribute__((stdcall)) unknown_004ba014_unknown_00030cd0(
	boolean first,
	boolean second)
{
	if (first)
	{
		return NONE;
	}
	if (!second)
	{
		return 2;
	}

	return ((unknown_004ba014>>4)&1) && !((unknown_004ba014>>5)&1) ? 0 : 1;
}

/* 0x004858c4: points to a byte whose bit 0x20 is examined */
const byte *unknown_004858c4;

/* 0x00030da0: one of five numbers, by the bits of unknown_004ba014, the bit
0x20 of what unknown_004858c4 points to and what the caller says. It is
before object_get_unknown30's neighbours in the game */
long __attribute__((stdcall)) unknown_004ba014_unknown_00030da0(
	boolean first,
	boolean second)
{
	if (unknown_004ba014&1)
	{
		return 1;
	}
	if (*unknown_004858c4&0x20)
	{
		return 4;
	}
	if (((unknown_004ba014>>1)&1) && first)
	{
		return 0;
	}
	if (((unknown_004ba014>>2)&1) && second)
	{
		return 2;
	}

	return 3;
}
