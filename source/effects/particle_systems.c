/*
PARTICLE_SYSTEMS.C

the particles of the effects. Worked out from halo2ship.exe; the name of the
file is a guess, by the names that the game gives to its four tables of data
("particle_system", "particles", "particle_emitter", "particle_location").
*/

#include "cseries/cseries.h"
#include "memory/data.h"
#include "saved games/game_state.h"
#include <string.h>

/* ---------- globals */

/* of effects.c, which empties them for a new map: 0x00510c74 (0x80 of 0x54
bytes), 0x0051ec84 (0x400 of 0x40), 0x0051ec88 (0x100 of 0x4C) and 0x0051ec8c
(0x100 of 0x34) */
extern struct data_array *particle_system_data;
extern struct data_array *particle_data;
extern struct data_array *particle_emitter_data;
extern struct data_array *particle_location_data;

/* ---------- code */

/* 0x00173de0: the particles make their four tables, in the memory of the
state of the game (data_new, which the game has written here) */
void particle_systems_initialize(
	void)
{
	particle_system_data= data_new("particle_system", 0x80, 0x54, 0, game_state_allocation);
	particle_data= data_new("particles", 0x400, 0x40, 0, game_state_allocation);
	particle_emitter_data= data_new("particle_emitter", 0x100, 0x4C, 0, game_state_allocation);
	particle_location_data= data_new("particle_location", 0x100, 0x34, 0, game_state_allocation);
}

/* seven small functions of things that are not known. The first three are
just before the particles in the game; the other four are far (0x002470e0),
but what calls them (0x00247470) calls one of the first too, so they are
kept together. Whether any is of this file is not known */

/* 0x00173b60: a short is put in a ring of 0x100 of them, which starts at 8;
the number at 4 says where the next goes */
void unknown_ring_add_short(
	byte *ring,
	short value)
{
	long index= *(const long *)(ring+4);

	*(long *)(ring+4)= index+1==0x100 ? 0 : index+1;
	*(short *)(ring+8+index*2)= value;
}

/* 0x00173b80: the bit 2 of the byte that something has at 0x16 */
long unknown_get_unknown16_bit2(
	const byte *unknown)
{
	return (unknown[0x16]>>2)&1;
}

/* 0x00173b90: the bit 10 of the short that something has at 0xC */
long unknown_get_unknownC_bit10(
	const byte *unknown)
{
	return (*(const unsigned short *)(unknown+0xC)>>10)&1;
}

/* 0x002470e0: where something has what it has at 0x9C */
byte *unknown_get_unknown9C(
	byte *unknown)
{
	return unknown+0x9C;
}

/* 0x002470f0: and at 0xA8 */
byte *unknown_get_unknownA8(
	byte *unknown)
{
	return unknown+0xA8;
}

/* 0x00247120: the bit 7 of the byte at 0x16 */
long unknown_get_unknown16_bit7(
	const byte *unknown)
{
	return (unknown[0x16]>>7)&1;
}

/* 0x00247130: the bit 9 of the short at 0xC */
long unknown_get_unknownC_bit9(
	const byte *unknown)
{
	return (*(const unsigned short *)(unknown+0xC)>>9)&1;
}

/* 0x00173350: in a chain of entries (0x6C bytes, each to the next by its
short at 0x08), from one, the first whose short at 0x0A is a number; NONE
if none */
short particle_systems_unknown_00173350(
	short index,
	const byte *entries,
	short value)
{
	while (index!=NONE)
	{
		const byte *entry= entries+index*0x6C;

		if (*(const short *)(entry+0x0A)==value)
		{
			break;
		}
		index= *(const short *)(entry+0x08);
	}

	return index;
}

/* 0x00172bb0: one more entry (0x6C bytes, from 0x04; how many at 0x00) of a
table: a number and a short, NONE for the rest, the 16 bytes at 0xA0 of
something and its pairs (as many as at 0x198, from 0x19C) copied to the
end of those of the table (from 0x6C08; how many at 0x6C04); its index */
short __attribute__((stdcall)) particle_systems_unknown_00172bb0(
	byte *table,
	const byte *data,
	short kind,
	long number)
{
	short index= (short)*(const long *)table;
	byte *entry= table+4+index*0x6C;
	long count= *(const long *)(data+0x198);
	long first= *(const long *)(table+0x6C04);

	*(long *)table= *(const long *)table+1;
	*(long *)entry= number;
	*(short *)(entry+0x04)= NONE;
	*(short *)(entry+0x06)= NONE;
	*(short *)(entry+0x0A)= NONE;
	*(long *)(entry+0x28)= NONE;
	*(short *)(entry+0x08)= kind;
	memcpy(entry+0x0C, data+0xA0, 16);
	*(long *)(table+0x6C04)= count+first;
	*(byte **)(entry+0x20)= table+0x6C08+first*8;
	memcpy(table+0x6C08+first*8, data+0x19C, (unsigned long)count*8);
	*(short *)(entry+0x1C)= (short)count;
	*(float *)(entry+0x24)= 0.0f;
	memset(entry+0x2C, 0, 0x40);

	return index;
}
