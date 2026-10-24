/*
UNITS.C

the bipeds and the vehicles: what they carry. Worked out from halo2ship.exe;
the name of the file is a guess, by the one that the first game has.
*/

#include "units.h"

#include <stdint.h>
#include <string.h>

#include "cache/cache_files.h"
#include "game/game.h"
#include "objects/objects.h"
#include "scenario/scenario.h"

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

/* 0x000bf090: an object with a short at 0xAC is taken out of unknown_004de2d0
(as many as the scenario says at 0x48), and its short is NONE */
void unknown_004de2d0_remove(
	long object_index)
{
	byte *object= (byte *)((const struct object_header_datum *)object_header_data->data)[object_index&0xFFFF].datum;
	short index;

	if (*(const short *)(object+0xAC)==NONE)
	{
		return;
	}
	*(short *)(object+0xAC)= NONE;
	for (index= 0; index<*(const long *)((const byte *)global_scenario+0x48); index++)
	{
		if (unknown_004de2d0[index]==object_index)
		{
			unknown_004de2d0[index]= NONE;
		}
	}
}

/* whether an object is of the first two kinds (its byte at 0xAA), the bit
of its kind in 3 as the processor shifts it */
#define UNIT_OBJECT_IS_UNIT(object) ((1UL<<(*((const byte *)(object)+0xAA)&0x1F))&3)

/* what a unit has at its short at 0x12A: whether it is set (its longs at 0x68
and 0x00 and its short at 0x06 are not NONE) and has a name (0x7C) */
#define UNIT_UNKNOWN12A(unit) ((const byte *)(unit)+*(const short *)((const byte *)(unit)+0x12A))
#define UNIT_UNKNOWN12A_IS(data, name) \
	(*(const long *)((data)+0x68)!=NONE && *(const long *)(data)!=NONE && *(const short *)((data)+0x06)!=NONE \
		&& *(const unsigned long *)((data)+0x7C)==(name))

/* 0x00467430: five names */
unsigned long unknown_00467430[5];

/* 0x000c7070: whether the name at 0x7C of what a unit has at its short at
0x12A is one of unknown_00467430 */
boolean unit_unknown_000c7070(
	long unit_index)
{
	const byte *unit= UNIT_OBJECT_GET(unit_index);
	unsigned long name= *(const unsigned long *)(UNIT_UNKNOWN12A(unit)+0x7C);
	const unsigned long *entry;

	for (entry= unknown_00467430; entry<unknown_00467430+5; entry++)
	{
		if (*entry==name)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* 0x000c8f60: the first child of an object that is a unit with the short
at 0x1FC; NONE if there is none */
long __attribute__((stdcall)) unit_unknown_000c8f60(
	long object_index,
	short number)
{
	long child_index= *(const long *)(UNIT_OBJECT_GET(object_index)+0x10);

	while (child_index!=NONE)
	{
		const byte *child= UNIT_OBJECT_GET(child_index);

		if (UNIT_OBJECT_IS_UNIT(child) && *(const short *)(child+0x1FC)==number)
		{
			break;
		}
		child_index= *(const long *)(child+0x0C);
	}

	return child_index;
}

/* 0x000cbe60: the first weapon of a unit that is not in either of its hands
(0x212 and 0x213); NONE if there is none */
long unit_unknown_000cbe60(
	long unit_index)
{
	const byte *unit= UNIT_OBJECT_GET(unit_index);
	long index;

	for (index= 0; index<4; index++)
	{
		if (index!=(signed char)unit[0x212] && index!=(signed char)unit[0x213]
			&& ((const long *)(unit+UNIT_WEAPONS_OFFSET))[index]!=NONE)
		{
			return ((const long *)(unit+UNIT_WEAPONS_OFFSET))[index];
		}
	}

	return NONE;
}

/* 0x000cc3c0: whether what a unit has at its short at 0x12A is set and has
the name 0x0600008C */
boolean unit_unknown_000cc3c0(
	long unit_index)
{
	return UNIT_UNKNOWN12A_IS(UNIT_UNKNOWN12A(UNIT_OBJECT_GET(unit_index)), 0x0600008C);
}

/* 0x000cc410: the same, with the name 0x0D000042 or 0x0C000043 */
long unit_unknown_000cc410(
	long unit_index)
{
	const byte *data= UNIT_UNKNOWN12A(UNIT_OBJECT_GET(unit_index));

	return UNIT_UNKNOWN12A_IS(data, 0x0D000042) || UNIT_UNKNOWN12A_IS(data, 0x0C000043);
}

/* the first long of an element (0xB0 bytes) of the block at 0x1C8 of the
definition of a unit */
#define UNIT_DEFINITION_UNKNOWN1C8(definition, index) \
	(*(const unsigned long *)(*(const byte *const *)((definition)+0x1CC)+(index)*0xB0))

/* 0x000cc750: the bit 2 of an element of the block at 0x1C8 of the
definition of a unit; FALSE for one that is not there */
boolean unit_unknown_000cc750(
	long unit_index,
	short index)
{
	const byte *definition= UNIT_DEFINITION_GET(UNIT_OBJECT_GET(unit_index));

	return index>=0 && index<*(const long *)(definition+0x1C8) && (UNIT_DEFINITION_UNKNOWN1C8(definition, index)>>2)&1;
}

/* 0x000cc7b0: the bit 3 */
boolean unit_unknown_000cc7b0(
	long unit_index,
	short index)
{
	const byte *definition= UNIT_DEFINITION_GET(UNIT_OBJECT_GET(unit_index));

	return index>=0 && index<*(const long *)(definition+0x1C8) && (UNIT_DEFINITION_UNKNOWN1C8(definition, index)>>3)&1;
}

/* 0x000cccd0: one of the bytes from 0x23E of a unit gets a number more, its
index goes in the bytes at 0x23C and 0x23D, and the byte is returned */
short __attribute__((stdcall)) unit_unknown_000cccd0(
	long unit_index,
	short index,
	signed char amount)
{
	signed char *unit= (signed char *)UNIT_OBJECT_GET(unit_index);

	unit[index+0x23E]+= amount;
	unit[0x23D]= (signed char)index;
	unit[0x23C]= (signed char)index;
	return unit[index+0x23E];
}

/* 0x000cd5c0: whether a unit has a weapon of a tag */
boolean __attribute__((stdcall)) unit_unknown_000cd5c0(
	long unit_index,
	long tag_index)
{
	const long *weapons= (const long *)(UNIT_OBJECT_GET(unit_index)+UNIT_WEAPONS_OFFSET);
	long index;

	for (index= 0; index<4; index++)
	{
		if (weapons[index]!=NONE && *(const long *)UNIT_OBJECT_GET(weapons[index])==tag_index)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* 0x000cfe40: the first weapon of a unit whose definition has the bit 3 of
its long at 0x12C; NONE if none has */
short unit_unknown_000cfe40(
	long unit_index)
{
	const long *weapons= (const long *)(UNIT_OBJECT_GET(unit_index)+UNIT_WEAPONS_OFFSET);
	short index;

	for (index= 0; index<4; index++)
	{
		if (weapons[index]!=NONE
			&& (*(const unsigned long *)(UNIT_DEFINITION_GET(UNIT_OBJECT_GET(weapons[index]))+0x12C)>>3)&1)
		{
			return index;
		}
	}

	return NONE;
}

/* 0x000d05c0: the next of a list of objects (each has the next at 0x0C;
the next of the iterator is at 0x0C) that is a unit: its index goes at
0x04 of the iterator and its short at 0x1FC at 0x08, and it is returned;
nothing after the last */
byte *unit_iterator_next(
	byte *iterator)
{
	while (*(const long *)(iterator+0x0C)!=NONE)
	{
		long object_index= *(const long *)(iterator+0x0C);
		byte *object= (byte *)UNIT_OBJECT_GET(object_index);

		*(long *)(iterator+0x0C)= *(const long *)(object+0x0C);
		if (UNIT_OBJECT_IS_UNIT(object))
		{
			*(long *)(iterator+0x04)= object_index;
			*(short *)(iterator+0x08)= *(const short *)(object+0x1FC);
			return object;
		}
	}

	return NULL;
}

/* 0x000d0930: a unit gets a point (or vector) at 0x15C and at 0x180 */
void unit_unknown_000d0930(
	long unit_index,
	const unsigned long *point)
{
	unsigned long *unit= (unsigned long *)UNIT_OBJECT_GET(unit_index);

	unit[0x15C/4]= point[0];
	unit[0x15C/4+1]= point[1];
	unit[0x15C/4+2]= point[2];
	unit[0x180/4]= point[0];
	unit[0x180/4+1]= point[1];
	unit[0x180/4+2]= point[2];
}

/* 0x000c48f0: whether the parent of an object is a unit whose definition
has the bit 6 in the element of the block at 0x1C8 that the object says at
0x1FC; FALSE for NONE or without one */
boolean unit_unknown_000c48f0(
	long object_index)
{
	const byte *object;
	const byte *parent;

	if (object_index==NONE)
	{
		return FALSE;
	}
	object= UNIT_OBJECT_GET(object_index);
	if (*(const long *)(object+0x14)==NONE)
	{
		return FALSE;
	}
	parent= UNIT_OBJECT_GET(*(const long *)(object+0x14));
	if (!UNIT_OBJECT_IS_UNIT(parent))
	{
		return FALSE;
	}

	return (UNIT_DEFINITION_UNKNOWN1C8(UNIT_DEFINITION_GET(parent), *(const short *)(object+0x1FC))>>6)&1;
}

/* the tag that the definition of an object names at 0x38; NULL for NONE */
#define UNIT_DEFINITION_UNKNOWN38(definition) \
	(*(const long *)((definition)+0x38)==NONE ? NULL \
		: (const byte *)(uintptr_t)global_tag_instances[*(const long *)((definition)+0x38)&0xFFFF].address)

/* 0x000d5990: whether a kind of object is one of 0, 1, 2, 3, 4, 7, 8, 11 and
12 (the bits of 0x199F, by the shift of the processor); for 6, whether the
tag that the definition names at 0x38 has a block at 0x60 with something in
it (not for a definition that is not loaded) */
boolean unit_unknown_000d5990(
	byte kind,
	long tag_index)
{
	unsigned long bit= 1UL<<(kind&0x1F);
	const byte *definition;
	const byte *tag;

	if (bit&0x199F)
	{
		return TRUE;
	}
	if (!(bit&0x40))
	{
		return FALSE;
	}
	definition= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;
	if (!definition || !(tag= UNIT_DEFINITION_UNKNOWN38(definition)))
	{
		return FALSE;
	}

	return *(const long *)(tag+0x60)>0;
}

/* 0x000d5a70: a number by the bytes at 0x30 and 0x2F of something: 2, 1, 0
or NONE */
long unit_unknown_000d5a70(
	const byte *data)
{
	switch ((signed char)data[0x30])
	{
	case 0:
		break;
	case 1:
		return 0;
	case 2:
		return 2;
	default:
		return NONE;
	}
	switch ((signed char)data[0x2F])
	{
	case 0:
		return 1;
	case 1:
		if (*(const short *)(data+0x2C)==NONE || (1UL<<(data[0x2E]&0x1F))&0x181F)
		{
			return 0;
		}
		return 1;
	default:
		return NONE;
	}
}

/* 0x000d5b60: the long at 0x64 of the tag that the definition of an object
names at 0x38, if its long at 0x60 is above zero; 0 if not or for NONE */
long unit_unknown_000d5b60(
	long object_index)
{
	const byte *tag;

	if (object_index==NONE)
	{
		return 0;
	}
	tag= UNIT_DEFINITION_UNKNOWN38(UNIT_DEFINITION_GET(UNIT_OBJECT_GET(object_index)));
	if (!tag || *(const long *)(tag+0x60)<=0)
	{
		return 0;
	}

	return *(const long *)(tag+0x64);
}

/* 0x0047d8e0: a short that is NONE when the game starts */
short unknown_0047d8e0;

/* 0x000d6660: something of 0x88 bytes is made, with a number at 0x00, NONE
in most of what it has, the structure bsp at 0x22 and 1.0 three times */
void __attribute__((stdcall)) unit_unknown_000d6660(
	byte *data,
	long number)
{
	memset(data, 0, 0x88);
	*(long *)(data+0x00)= number;
	*(short *)(data+0x7C)= unknown_0047d8e0;
	*(long *)(data+0x0C)= NONE;
	*(long *)(data+0x08)= NONE;
	*(short *)(data+0x10)= NONE;
	*(long *)(data+0x14)= NONE;
	*(long *)(data+0x1C)= NONE;
	*(short *)(data+0x20)= NONE;
	*(short *)(data+0x22)= global_structure_bsp_index;
	*(long *)(data+0x18)= NONE;
	*(real *)(data+0x54)= 1.0f;
	*(real *)(data+0x58)= 1.0f;
	*(real *)(data+0x5C)= 1.0f;
	*(short *)(data+0x7E)= NONE;
	data[0x84]= 0;
}

/* 0x000d74b0: whether any of the elements (0x60 bytes) of the block at 0x70
of something points at 0x40 to something with a byte set at 0x40 */
boolean unit_unknown_000d74b0(
	const byte *data)
{
	const byte *element= *(const byte *const *)(data+0x70);
	long index;

	for (index= 0; index<*(const long *)(data+0x74); index++)
	{
		if ((*(const byte *const *)(element+index*0x60+0x40))[0x40])
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* 0x000d8ae0: the short at 0x10 of an element (0x14 bytes) of the block at
0x5C of the tag that the definition of an object names at 0x38;
unknown_0047d8e0 for NONE or without the tag */
void unit_unknown_000d8ae0(
	long object_index,
	short *result,
	long index)
{
	const byte *tag;

	if (object_index!=NONE
		&& (tag= UNIT_DEFINITION_UNKNOWN38(UNIT_DEFINITION_GET(UNIT_OBJECT_GET(object_index)))))
	{
		*result= *(const short *)(*(const byte *const *)(tag+0x5C)+index*0x14+0x10);
		return;
	}
	*result= unknown_0047d8e0;
}

/* 0x000dd300: a number by a name (they look like string ids): 0 for
0x0400000C, 0x0900000E and 0x0A00000F; 1 for 0x0800001E, 0x09000015,
0x09000016, 0x0A000014, 0x0A000017 and 0x0D000042; 2 for the others */
byte unit_unknown_000dd300(
	long name)
{
	switch (name)
	{
	case 0x0400000C:
	case 0x0900000E:
	case 0x0A00000F:
		return 0;
	case 0x0800001E:
	case 0x09000015:
	case 0x09000016:
	case 0x0A000014:
	case 0x0A000017:
	case 0x0D000042:
		return 1;
	default:
		return 2;
	}
}

/* 0x000ddf60: whether a unit has NONE at 0x35C and 0x360 and something at
0x354 that has the bit 0x01 in the byte at 0x04 of its entry (8 bytes) of
the list at 0x2C of the collision bsp */
boolean unit_unknown_000ddf60(
	long unit_index)
{
	const byte *unit= UNIT_OBJECT_GET(unit_index);
	long index= *(const long *)(unit+0x354);

	if (*(const long *)(unit+0x35C)!=NONE || index==NONE || *(const long *)(unit+0x360)!=NONE)
	{
		return FALSE;
	}

	return (*(const byte *const *)((const byte *)global_collision_bsp+0x2C))[index*8+4]&1;
}

/* 0x000dfd70: the six longs from 0x350 of a unit are NONE */
void unit_unknown_000dfd70(
	long unit_index)
{
	long *unit= (long *)UNIT_OBJECT_GET(unit_index);

	unit[0x354/4]= NONE;
	unit[0x358/4]= NONE;
	unit[0x35C/4]= NONE;
	unit[0x360/4]= NONE;
	unit[0x364/4]= NONE;
	unit[0x350/4]= NONE;
}

/* 0x000e0dc0: whether a unit has 6 at 0x3DC, something at 0x3F8 and the bit
13 of its short at 0x348 */
boolean unit_unknown_000e0dc0(
	long unit_index)
{
	const byte *unit= UNIT_OBJECT_GET(unit_index);

	return unit[0x3DC]==6 && unit[0x3F8] && (*(const unsigned short *)(unit+0x348)>>13)&1;
}

/* 0x000e58e0: whether a unit has something at 0x3E8 and either the bit 1
or 2 of its byte at 0xC0, or the same at 0x3E4 */
boolean unit_unknown_000e58e0(
	long unit_index)
{
	const byte *unit= UNIT_OBJECT_GET(unit_index);
	long value= *(const long *)(unit+0x3E8);

	return value!=NONE && ((unit[0xC0]>>1)&1 || (unit[0xC0]>>2)&1 || value==*(const long *)(unit+0x3E4));
}

/* 0x000e5980: 3 with the bit 2 of the byte at 0x10A of an object; else 2
with the bit 4 of the long at 0x264 of its definition and 1 without */
long unit_unknown_000e5980(
	long object_index)
{
	const byte *object= UNIT_OBJECT_GET(object_index);

	if ((object[0x10A]>>2)&1)
	{
		return 3;
	}

	return ((*(const unsigned long *)(UNIT_DEFINITION_GET(object)+0x264)>>4)&1)+1;
}

/* 0x000e6800: FALSE if something has the bit 0x01 of its byte at 0x12 and
neither 0x01 nor 0x08 of that at 0x11 */
boolean unit_unknown_000e6800(
	const byte *data)
{
	return !((data[0x12]&1) && !(data[0x11]&9));
}

/* 0x000e6fe0: a bit of the bits after the long at 0x04 of what a unit has
at its short at 0x346 */
long unit_unknown_000e6fe0(
	long unit_index,
	long bit)
{
	const unsigned long *bits= (const unsigned long *)(UNIT_UNKNOWN346(UNIT_OBJECT_GET(unit_index))+0x04);

	return (bits[bit>>5]&(1UL<<(bit&0x1F)))!=0;
}

/* 0x000ee8a0: whether the weapon in a hand of a unit (0x212 or 0x213) has
the bit 1 and not the bit 2 of its byte at 0x16C */
boolean __attribute__((stdcall)) unit_unknown_000ee8a0(
	long unit_index,
	long hand)
{
	const byte *unit= UNIT_OBJECT_GET(unit_index);
	short weapon= (signed char)unit[0x212+hand];
	long weapon_index;
	byte flags;

	if (weapon==NONE || (weapon_index= ((const long *)(unit+UNIT_WEAPONS_OFFSET))[weapon])==NONE)
	{
		return FALSE;
	}
	flags= UNIT_OBJECT_GET(weapon_index)[0x16C];

	return (flags>>1)&1 && !((flags>>2)&1);
}

/* 0x000f06c0: the shorts at 0xBC0 of sixteen entries of 0xD8 bytes of
something are NONE */
void unit_unknown_000f06c0(
	byte *data)
{
	long index;

	for (index= 0; index<16; index++)
	{
		*(short *)(data+index*0xD8+0xBC0)= NONE;
	}
}

/* 0x000f1320: whether an object has the bit 2 of its byte at 0x10A and its
definition the short 5 at 0x1F0, or 4 with 1 at 0x218 */
boolean unit_unknown_000f1320(
	long object_index)
{
	const byte *object= UNIT_OBJECT_GET(object_index);
	const byte *definition= UNIT_DEFINITION_GET(object);
	short kind;

	if (!((object[0x10A]>>2)&1))
	{
		return FALSE;
	}
	kind= *(const short *)(definition+0x1F0);

	return (kind==4 && *(const short *)(definition+0x218)==1) || kind==5;
}

/* 0x000f1c80: a unit gets the long at 0x70 of something; and its byte at
0x387 is cleared unless one of the entries (0xD8 bytes, as many as the long
at 0x44 of what it points to at 0x04) has its byte at 0xB90 set */
void unit_unknown_000f1c80(
	long unit_index,
	const byte *data)
{
	byte *unit= (byte *)UNIT_OBJECT_GET(unit_index);
	long count;
	long index;

	*(long *)(unit+0x37C)= *(const long *)(data+0x70);
	count= *(const long *)(*(const byte *const *)(data+0x04)+0x44);
	for (index= 0; index<count; index++)
	{
		if (data[index*0xD8+0xB90])
		{
			return;
		}
	}
	unit[0x387]= 0;
}

/* 0x000f5d10: a bit (0 to 31) of the long at 0x3B4 of a unit is set or
cleared; FALSE for another */
boolean __attribute__((stdcall)) unit_unknown_000f5d10(
	long unit_index,
	long bit,
	boolean set)
{
	unsigned long *flags= (unsigned long *)(UNIT_OBJECT_GET(unit_index)+0x3B4);

	if (bit<0 || bit>=32)
	{
		return FALSE;
	}
	if (set)
	{
		*flags|= 1UL<<bit;
	}
	else
	{
		*flags&= ~(1UL<<bit);
	}
	return TRUE;
}

/* 0x00100390: the bit 0x04 of the byte at 0x1A8 of one of the entries
(0x34 bytes) of a weapon, if its definition has that many at 0x2D0 */
boolean weapon_unknown_00100390(
	long weapon_index,
	long index)
{
	byte *weapon= (byte *)UNIT_OBJECT_GET(weapon_index);

	if (*(const long *)(UNIT_DEFINITION_GET(weapon)+0x2D0)<=index)
	{
		return FALSE;
	}
	weapon[index*0x34+0x1A8]|= 0x04;
	return TRUE;
}

/* 0x001003e0: the same with the bit 0x20 */
boolean weapon_unknown_001003e0(
	long weapon_index,
	long index)
{
	byte *weapon= (byte *)UNIT_OBJECT_GET(weapon_index);

	if (*(const long *)(UNIT_DEFINITION_GET(weapon)+0x2D0)<=index)
	{
		return FALSE;
	}
	weapon[index*0x34+0x1A8]|= 0x20;
	return TRUE;
}

/* 0x00100880: whether one of the entries (0x10 bytes, from 0x220) of a
weapon, as many as its definition has at 0x2C0, has a short at 0x04 from 1
to 6; of all of them for NONE, or of only one */
boolean weapon_unknown_00100880(
	long weapon_index,
	long which)
{
	const byte *weapon= UNIT_OBJECT_GET(weapon_index);
	long count= *(const long *)(UNIT_DEFINITION_GET(weapon)+0x2C0);
	long index;

	for (index= 0; index<count; index++)
	{
		if (which==NONE || which==index)
		{
			short value= *(const short *)(weapon+(short)index*0x10+0x224);

			if (value>0 && value<=6)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

/* 0x00100f00: FALSE when the real at 0x184 of a weapon is 1.0 or more, or
when its definition has entries at 0x2C0 whose first has a short above 0
at 0x0A and the weapon has 0 in its shorts at 0x22C and 0x22A */
boolean weapon_unknown_00100f00(
	long weapon_index)
{
	const byte *weapon= UNIT_OBJECT_GET(weapon_index);
	const byte *definition= UNIT_DEFINITION_GET(weapon);

	if (*(const real *)(weapon+0x184)>=1.0f)
	{
		return FALSE;
	}

	return *(const long *)(definition+0x2C0)<=0 || *(const short *)(*(const byte *const *)(definition+0x2C4)+0x0A)<=0
		|| *(const short *)(weapon+0x22C)!=0 || *(const short *)(weapon+0x22A)!=0;
}

/* 0x00101160: whether any of the bytes from 0x20C (each 0x0C bytes, as many
as the definition has at 0x2C8) of a weapon is 3, 5, 6 or 7, or 2 with a
short at 0x294 of its definition */
boolean weapon_unknown_00101160(
	long weapon_index)
{
	const byte *weapon= UNIT_OBJECT_GET(weapon_index);
	const byte *definition= UNIT_DEFINITION_GET(weapon);
	boolean result= FALSE;
	long index;

	for (index= 0; index<*(const long *)(definition+0x2C8); index++)
	{
		byte state= weapon[index*0x0C+0x20C];

		if (state==3 || state==7 || state==5 || state==6 || (state==2 && *(const short *)(definition+0x294)))
		{
			result= TRUE;
		}
	}

	return result;
}

/* 0x001011d0: whether any of those bytes is 0 with 5 at the short at 0x06
of its element (0x40 bytes) of the block at 0x2C8 of the definition */
boolean weapon_unknown_001011d0(
	long weapon_index)
{
	const byte *weapon= UNIT_OBJECT_GET(weapon_index);
	const byte *definition= UNIT_DEFINITION_GET(weapon);
	boolean result= FALSE;
	long index;

	for (index= 0; index<*(const long *)(definition+0x2C8); index++)
	{
		if (!weapon[index*0x0C+0x20C] && *(const short *)(*(const byte *const *)(definition+0x2CC)+index*0x40+0x06)==5)
		{
			result= TRUE;
		}
	}

	return result;
}

/* 0x00101340: the bit 18 of the long at 0x12C of the definition of an
object; FALSE for NONE */
boolean weapon_unknown_00101340(
	long weapon_index)
{
	if (weapon_index==NONE)
	{
		return FALSE;
	}

	return (*(const unsigned long *)(UNIT_DEFINITION_GET(UNIT_OBJECT_GET(weapon_index))+0x12C)>>18)&1;
}

/* 0x00101440: TRUE when the long at 0x178 of a weapon is from 5 to 10, and
else the bit 6 of the long at 0x12C of its definition; FALSE for NONE */
boolean weapon_unknown_00101440(
	long weapon_index)
{
	const byte *weapon;
	long value;

	if (weapon_index==NONE)
	{
		return FALSE;
	}
	weapon= UNIT_OBJECT_GET(weapon_index);
	value= *(const long *)(weapon+0x178);
	if (value>=5 && value<=10)
	{
		return TRUE;
	}

	return (*(const unsigned long *)(UNIT_DEFINITION_GET(weapon)+0x12C)>>6)&1;
}

/* 0x00101640: whether the long at 0x12C of the definition of an object has
the bit 22 or 23 */
boolean weapon_unknown_00101640(
	long weapon_index)
{
	unsigned long flags= *(const unsigned long *)(UNIT_DEFINITION_GET(UNIT_OBJECT_GET(weapon_index))+0x12C);

	return (flags>>22)&1 || (flags>>23)&1;
}

/* 0x00101b10: a name (it looks like a string id) of one of the first two
entries (0xEC bytes) of the block at 0x2D0 of the definition of a weapon, at
0x30; 0x0F0000DB or 0x110000DC when it has none there, and 0 for another */
long weapon_unknown_00101b10(
	long index,
	long weapon_index)
{
	const byte *definition;
	long name;

	if (index<0 || index>=2)
	{
		return 0;
	}
	definition= UNIT_DEFINITION_GET(UNIT_OBJECT_GET(weapon_index));
	name= index ? 0x110000DC : 0x0F0000DB;
	if (index<*(const long *)(definition+0x2D0))
	{
		long value= *(const long *)(*(const byte *const *)(definition+0x2D4)+index*0xEC+0x30);

		if (value && value!=NONE)
		{
			name= value;
		}
	}

	return name;
}

/* 0x00101f50: which of the weapons of the unit that has a weapon (0x154,
with the bit 0 of its byte at 0x12C) it is; NONE if none */
long weapon_unknown_00101f50(
	long weapon_index)
{
	const byte *weapon= UNIT_OBJECT_GET(weapon_index);
	long unit_index= (weapon[0x12C]&1) ? *(const long *)(weapon+0x154) : NONE;
	long index;

	if (unit_index==NONE)
	{
		return NONE;
	}
	for (index= 0; index<4; index++)
	{
		if (((const long *)(UNIT_OBJECT_GET(unit_index)+UNIT_WEAPONS_OFFSET))[index]==weapon_index)
		{
			return index;
		}
	}

	return NONE;
}

/* 0x00106400: whether the first element (0x40 bytes) of the block at 0x2C8
of the definition of a weapon has 2 at 0x06 and 1 at 0x18, when asked or
when the first byte of 0x20C of the weapon is 2 */
boolean __attribute__((stdcall)) weapon_unknown_00106400(
	long weapon_index,
	boolean always)
{
	const byte *weapon= UNIT_OBJECT_GET(weapon_index);
	const byte *definition= UNIT_DEFINITION_GET(weapon);
	const byte *element;

	if (*(const long *)(definition+0x2C8)<=0)
	{
		return FALSE;
	}
	element= *(const byte *const *)(definition+0x2CC);

	return (always || weapon[0x20C]==2) && *(const short *)(element+0x06)==2 && *(const short *)(element+0x18)==1;
}

/* what an object has at the offset that its short at 0x33E gives */
#define UNIT_UNKNOWN33E(object) ((const byte *)(object)+*(const short *)((const byte *)(object)+0x33E))

/* 0x0010db60: whether what an object has at its short at 0x33E has a short
at 0x36, and a real of 0.0001 or more at 0x40 or a byte from 1 to 6 at 0x02 */
long unit_unknown_0010db60(
	long object_index)
{
	const byte *data= UNIT_UNKNOWN33E(UNIT_OBJECT_GET(object_index));
	boolean enough= *(const real *)(data+0x40)>=0.0001f;
	boolean kind= data[0x02]>0 && data[0x02]<7;

	return *(const short *)(data+0x36)!=NONE && (enough || kind);
}

/* 0x0010edd0: whether it has something in the high short at 0x5C and one of
the thirteen bytes from 0x6E set */
boolean unit_unknown_0010edd0(
	long object_index)
{
	const byte *data= UNIT_UNKNOWN33E(UNIT_OBJECT_GET(object_index));
	long index;

	if ((short)(*(const unsigned long *)(data+0x5C)>>16)==NONE)
	{
		return FALSE;
	}
	for (index= 0; index<13; index++)
	{
		if (data[index+0x6E])
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* whether three things from an offset of what an object has at its short at
0x33E (its long, the short 6 after and the byte 0x12 after) say that it is
set: not NONE, the bit 0x01 of the last and not 0x01 or 0x08 of the one
before it */
#define UNIT_UNKNOWN33E_SET(data, offset) \
	(*(const long *)((data)+(offset))!=NONE && *(const short *)((data)+(offset)+0x06)!=NONE \
		&& ((data)[(offset)+0x12]&1) && !((data)[(offset)+0x11]&9))

/* 0x0010f820: whether the one at 0x9C or that at 0xBC is set */
boolean unit_unknown_0010f820(
	long object_index)
{
	const byte *data= UNIT_UNKNOWN33E(UNIT_OBJECT_GET(object_index));

	return UNIT_UNKNOWN33E_SET(data, 0x9C) || UNIT_UNKNOWN33E_SET(data, 0xBC);
}

/* 0x0010f890: whether the one at 0xDC is set, with the bit 7 of the first
byte */
boolean unit_unknown_0010f890(
	long object_index)
{
	const byte *data= UNIT_UNKNOWN33E(UNIT_OBJECT_GET(object_index));

	return UNIT_UNKNOWN33E_SET(data, 0xDC) && (data[0]>>7)&1;
}

/* 0x0010f630: the longs at 0x70 and 0x7C of what a unit has at its short at
0x12A when it is set; NONE and FALSE if not */
boolean unit_unknown_0010f630(
	long unit_index,
	long *first,
	long *second)
{
	const byte *data= UNIT_UNKNOWN12A(UNIT_OBJECT_GET(unit_index));

	if (*(const long *)(data+0x68)!=NONE && *(const long *)data!=NONE && *(const short *)(data+0x06)!=NONE)
	{
		*first= *(const long *)(data+0x70);
		*second= *(const long *)(data+0x7C);
		return TRUE;
	}
	*first= NONE;
	*second= NONE;
	return FALSE;
}

/* where the bits of something are: from its first long, after the bytes
that the long at 0x0C and the short at 0x06 of what it points to at 0x04,
and its first byte, say, and the longs for as many bits as its byte at 0x08
(rounded down to four bytes) */
#define UNIT_BITS_OFFSET(data) \
	((((long)*(const byte *)((data)+0x08)+0x1F)>>3&~3L) + *(const short *)(*(const byte *const *)((data)+0x04)+0x06) \
		+ *(const long *)(*(const byte *const *)((data)+0x04)+0x0C) + *(const signed char *)*(const byte *const *)((data)+0x04) \
		+ *(const long *)(data))

/* 0x0010ee70: that place */
long unit_unknown_0010ee70(
	const byte *data)
{
	return UNIT_BITS_OFFSET(data);
}

/* 0x0010eea0: one of the bits after it */
long unit_unknown_0010eea0(
	short bit,
	const byte *data)
{
	long place= ((((long)data[0x08]+0x1F)>>3&~3L) + (bit>>5)*4 + *(const short *)(*(const byte *const *)(data+0x04)+0x06)
		+ *(const long *)(*(const byte *const *)(data+0x04)+0x0C) + *(const signed char *)*(const byte *const *)(data+0x04));

	return (*(const unsigned long *)(*(const byte *const *)data+place)&(1UL<<(bit&0x1F)))!=0;
}

/* 0x00110fc0: whether the name at 0xA4 of what an object has at its short at
0x33E is 0x0A000066, 0x09000008, 0x09000009 or 0x0F00067F */
boolean unit_unknown_00110fc0(
	long object_index)
{
	long name= *(const long *)(UNIT_UNKNOWN33E(UNIT_OBJECT_GET(object_index))+0xA4);

	return name==0x0A000066 || (name>=0x09000008 && name<=0x09000009) || name==0x0F00067F;
}

/* 0x00113da0: whether the byte at 0x89 of that is not set, or the byte at
0x8B has the bit 0x02 and not 0x01 */
long unit_unknown_00113da0(
	long object_index)
{
	const byte *data= UNIT_UNKNOWN33E(UNIT_OBJECT_GET(object_index));

	return !data[0x89] || (!(data[0x8B]&1) && (data[0x8B]&2));
}

/* 0x00113df0: whether it is set and the byte at 0x8B has both */
long unit_unknown_00113df0(
	long object_index)
{
	const byte *data= UNIT_UNKNOWN33E(UNIT_OBJECT_GET(object_index));

	return data[0x89] && (data[0x8B]&1) && (data[0x8B]&2);
}

/* 0x00114440: the long at 0x14 of the tag that an object names at the
offset that its short at 0x342 gives; 0 for NONE */
long unit_unknown_00114440(
	long object_index)
{
	const byte *object= UNIT_OBJECT_GET(object_index);
	long tag_index= *(const long *)(object+*(const short *)(object+0x342));

	if (tag_index==NONE)
	{
		return 0;
	}

	return *(const long *)((const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address+0x14);
}

/* 0x00118f90: something of 0x30 bytes is made, with two names (0x06000086
and 0x0400000C), global_forward3d at 0x18 and global_up3d at 0x24 */
void unit_unknown_00118f90(
	unsigned long *data)
{
	memset(data, 0, 0x30);
	data[0]= 0x06000086;
	data[1]= 0x0400000C;
	data[6]= ((const unsigned long *)global_forward3d)[0];
	data[7]= ((const unsigned long *)global_forward3d)[1];
	data[8]= ((const unsigned long *)global_forward3d)[2];
	data[9]= ((const unsigned long *)global_up3d)[0];
	data[10]= ((const unsigned long *)global_up3d)[1];
	data[11]= ((const unsigned long *)global_up3d)[2];
}

/* 0x00119bc0: whether an object does not have the bit 2 of its byte at 0x10A
and has the bit 5 or 4 of that at 0x12C */
boolean unit_unknown_00119bc0(
	long object_index)
{
	const byte *object= UNIT_OBJECT_GET(object_index);

	return !((object[0x10A]>>2)&1) && ((object[0x12C]>>5)&1 || (object[0x12C]>>4)&1);
}

/* 0x0011b930: whether what a unit has at its short at 0x12A is set with the
name 0x0E0000C2; FALSE for NONE */
boolean unit_unknown_0011b930(
	long unit_index)
{
	if (unit_index==NONE)
	{
		return FALSE;
	}

	return UNIT_UNKNOWN12A_IS(UNIT_UNKNOWN12A(UNIT_OBJECT_GET(unit_index)), 0x0E0000C2);
}

/* 0x001d9320: a counter (its byte at 0x00, up to the one at 0x01) goes one
up with the bit 0x01 of its byte at 0x03, or one down without it, and gets
the bit 0x02 when it reaches the end; FALSE when it has no end or already
reached it */
boolean unit_counter_update(
	signed char *counter)
{
	signed char maximum= counter[1];

	if (!maximum || (counter[3]&2))
	{
		return FALSE;
	}
	if (counter[3]&1)
	{
		counter[0]++;
		if (counter[0]>=maximum)
		{
			counter[0]= maximum;
			counter[3]|= 2;
		}
	}
	else
	{
		counter[0]--;
		if (counter[0]<=0)
		{
			counter[0]= 0;
			counter[3]|= 2;
		}
	}

	return TRUE;
}

/* 0x001d9430: what is left of it, in seconds: up to the end going up, or
down to 0 going down; 0.0 without an end */
real unit_counter_seconds(
	const signed char *counter)
{
	if (!counter[1])
	{
		return 0.0f;
	}
	if (counter[3]&1)
	{
		return (real)(counter[1]-counter[0])*game_time_globals->seconds_per_tick;
	}

	return (real)counter[0]*game_time_globals->seconds_per_tick;
}

/* whether an object is of the kind 0 and has 6 at 0x3DC */
#define UNIT_KIND0_STATE6(header) \
	(((1UL<<((header)->type&0x1F))&1) && ((const byte *)(header)->datum)[0x3DC]==6)

/* 0x001dac00: whether it is not */
boolean unit_unknown_001dac00(
	long object_index)
{
	const struct object_header_datum *header= (const struct object_header_datum *)object_header_data->data+(object_index&0xFFFF);

	return !UNIT_KIND0_STATE6(header);
}

/* 0x001dac40: whether neither of two objects is */
long unit_unknown_001dac40(
	long first_index,
	long second_index)
{
	const struct object_header_datum *headers= (const struct object_header_datum *)object_header_data->data;

	return !UNIT_KIND0_STATE6(&headers[first_index&0xFFFF]) && !UNIT_KIND0_STATE6(&headers[second_index&0xFFFF]);
}

/* 0x001e67a0: something gets at 0xAC the point at 0x10 of another (or
global_up3d, without its byte at 0x1D), two bytes at 0xB8 and 1 at 0x04 */
void __attribute__((stdcall)) unit_unknown_001e67a0(
	const byte *source,
	byte *data,
	byte first,
	byte second)
{
	const unsigned long *point= source[0x1D] ? (const unsigned long *)(source+0x10) : (const unsigned long *)global_up3d;

	((unsigned long *)(data+0xAC))[0]= point[0];
	((unsigned long *)(data+0xAC))[1]= point[1];
	((unsigned long *)(data+0xAC))[2]= point[2];
	data[0xB8]= first;
	data[0xB9]= second;
	data[0x04]= 1;
}

/* 0x001e6850: something gets a byte at 0xBC, two points at 0xC0 and 0xCC,
a real at 0xD8 and 1 at 0x04 */
void __attribute__((stdcall)) unit_unknown_001e6850(
	byte *data,
	const unsigned long *second,
	byte value,
	real number,
	const unsigned long *first)
{
	data[0xBC]= value;
	((unsigned long *)(data+0xC0))[0]= first[0];
	((unsigned long *)(data+0xC0))[1]= first[1];
	((unsigned long *)(data+0xC0))[2]= first[2];
	((unsigned long *)(data+0xCC))[0]= second[0];
	((unsigned long *)(data+0xCC))[1]= second[1];
	((unsigned long *)(data+0xCC))[2]= second[2];
	*(real *)(data+0xD8)= number;
	data[0x04]= 1;
}

/* 0x001e6980: the oldest of four entries (8 bytes: a time, a short and a
byte) gets the game's time, a short and a byte */
void __attribute__((stdcall)) unit_unknown_001e6980(
	long *entries,
	short value,
	byte flags)
{
	long oldest= entries[0];
	long which= 0;

	if (entries[2]<oldest)
	{
		which= 1;
		oldest= entries[2];
	}
	if (entries[4]<oldest)
	{
		which= 2;
		oldest= entries[4];
	}
	if (entries[6]<oldest)
	{
		which= 3;
	}
	entries[which*2]= game_time_globals->time;
	*(short *)&entries[which*2+1]= value;
	*((unsigned short *)&entries[which*2+1]+1)= flags;
}

/* 0x001e6a00: a bit of two longs is set or cleared by the bits 0x01 and
0x02 of a value */
void unit_unknown_001e6a00(
	unsigned long value,
	byte bit,
	unsigned long *bits)
{
	unsigned long mask= 1UL<<(bit&0x1F);

	if (value&1)
	{
		bits[0]|= mask;
	}
	else
	{
		bits[0]&= ~mask;
	}
	if ((value>>1)&1)
	{
		bits[1]|= mask;
	}
	else
	{
		bits[1]&= ~mask;
	}
}

/* 0x004686a4: a byte that is TRUE when the game starts */
byte unknown_004686a4= TRUE;

/* 0x00113260: whether a unit whose definition does not have the bit 11 of
its long at 0xBC has what it keeps at its short at 0x12A set, with
unknown_004686a4, the bit 15 of its long at 0x148 or the bit 0x01 of its
byte at 0x137, and one of its bytes at 0x1F6 and 0x1F7 not NONE */
boolean unit_unknown_00113260(
	long unit_index)
{
	const byte *unit= UNIT_OBJECT_GET(unit_index);
	const byte *data= UNIT_UNKNOWN12A(unit);

	return !((*(const unsigned long *)(UNIT_DEFINITION_GET(unit)+0xBC)>>11)&1)
		&& *(const long *)(data+0x68)!=NONE && *(const long *)data!=NONE && *(const short *)(data+0x06)!=NONE
		&& unknown_004686a4
		&& ((signed char)unit[0x149]<0 || (unit[0x137]&1))
		&& ((signed char)unit[0x1F6]!=NONE || (signed char)unit[0x1F7]!=NONE);
}

/* 0x0014ccf0: a real of something by a name (they look like string ids): 1.0,
-1.0 or 0.0 by its bits at 0x07, or its bytes at 0x05 and 0x06 in 64ths and
32nds; 0.0 for other names */
boolean unit_unknown_0014ccf0(
	long name,
	const byte *data,
	float *value)
{
	switch (name)
	{
	case 0x180005FC:
		*value= (data[7]&1) ? 0.0f : 1.0f;
		break;
	case 0x0D0005FF:
		*value= 1.0f;
		break;
	case 0x170005FA:
		*value= (float)data[5]*0.015625f;
		break;
	case 0x170005FB:
		*value= (float)data[6]*0.03125f;
		break;
	case 0x1D0005FD:
		*value= (data[7]&2) ? 1.0f : -1.0f;
		break;
	case 0x1D0005FE:
		*value= (data[7]&4) ? 1.0f : -1.0f;
		break;
	default:
		*value= 0.0f;
		break;
	}

	return TRUE;
}
