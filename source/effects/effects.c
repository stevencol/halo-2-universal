/*
EFFECTS.C

the effects. Worked out from halo2ship.exe; the name of the file is a guess, by
the name that the game gives to its table of data ("effect", "effect location").
*/

#include "cseries/cseries.h"

#include <stdint.h>
#include <string.h>

#include "cache/cache_files.h"
#include "main/main.h"
#include "memory/data.h"
#include "objects/objects.h"
#include "saved games/game_state.h"

/* ---------- globals */

/* 0x004ea93c: the effects ("effect": 0x100 of 0x190 bytes). 0x004ea938:
where each is ("effect location": 0x200 of 0x3C bytes). 0x00510c74,
0x0051ec84, 0x0051ec88, 0x0051ec8c: the four tables of the particles, which
start and stop with the effects (particle_systems.c) */
struct data_array *effect_data;
struct data_array *effect_location_data;
struct data_array *particle_system_data;
struct data_array *particle_data;
struct data_array *particle_emitter_data;
struct data_array *particle_location_data;

/* ---------- code */

/* 0x00175bd0: the effects have no tables */
void effects_dispose(
	void)
{
	particle_location_data= NULL;
	particle_emitter_data= NULL;
	particle_data= NULL;
	particle_system_data= NULL;
	if (effect_data)
	{
		effect_data= NULL;
	}
	if (effect_location_data)
	{
		effect_location_data= NULL;
	}
}

/* 0x00175c10: for a new map, each of the six tables can be used, and is
empty (data_delete_all) */
void effects_initialize_for_new_map(
	void)
{
	effect_data->valid= TRUE;
	data_delete_all(effect_data);
	effect_location_data->valid= TRUE;
	data_delete_all(effect_location_data);
	particle_system_data->valid= TRUE;
	data_delete_all(particle_system_data);
	particle_data->valid= TRUE;
	data_delete_all(particle_data);
	particle_emitter_data->valid= TRUE;
	data_delete_all(particle_emitter_data);
	particle_location_data->valid= TRUE;
	data_delete_all(particle_location_data);
}

/* 0x00175c70: when a map goes, none of the six can be used */
void effects_dispose_from_old_map(
	void)
{
	particle_location_data->valid= FALSE;
	particle_emitter_data->valid= FALSE;
	particle_data->valid= FALSE;
	particle_system_data->valid= FALSE;
	effect_data->valid= FALSE;
	effect_location_data->valid= FALSE;
}

/* 0x00173de0, of particle_systems.c: the particles start with the effects */
void particle_systems_initialize(void);

/* 0x00175b50: the effects make their two tables, in the memory of the state
of the game (data_new, which the game has written here): 0x100 effects of
0x190 bytes and 0x200 places of 0x3C. Then the other things that start with
them (particle_systems_initialize) */
void effects_initialize(
	void)
{
	effect_data= data_new("effect", 0x100, 0x190, 0, game_state_allocation);
	effect_location_data= data_new("effect location", 0x200, 0x3C, 0, game_state_allocation);
	particle_systems_initialize();
}

/* 0x004e7408, of game.c: the two numbers that the random numbers come from */
extern unsigned long *global_random_seed_reference;

/* 0x00177c20: which of the two numbers that random numbers come from an
effect uses: the first, which every console has the same, for a definition
that has the bit 2 of its flags; the second if not */
unsigned long *effect_definition_get_random_seed(
	long definition_index)
{
	const unsigned long *definition=
		(const unsigned long *)(uintptr_t)global_tag_instances[definition_index&0xFFFF].address;

	if ((*definition>>2)&1)
	{
		return global_random_seed_reference;
	}

	return global_random_seed_reference+1;
}

/* 0x0016ddf0: the tag that a tag names at 4. It is far from the others of
this file in the game, and may be of another */
void *tag_get_unknown4_tag(
	long tag_index)
{
	const byte *tag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;

	return (void *)(uintptr_t)global_tag_instances[*(const long *)(tag+4)&0xFFFF].address;
}

/* 0x0004cb80: the short at 0x2C of one of the things of 0x5C bytes that a
tag has at 0x28; NONE for NONE or a tag that is nowhere. It is far from the
others of this file in the game, and may be of another */
long tag_get_unknown28_unknown2C(
	long tag_index,
	long index)
{
	if (tag_index!=NONE)
	{
		const byte *tag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;

		if (tag)
		{
			return *(const short *)(*(const byte *const *)(tag+0x28)+index*0x5C+0x2C);
		}
	}

	return NONE;
}

/* 0x0017afd0: whether the object of an effect (0x48) is a unit of a player
that has a number for its short at 0x28 */
boolean effect_unknown_0017afd0(
	long effect_index,
	long value)
{
	long object_index= *(const long *)((const byte *)effect_data->data+(effect_index&0xFFFF)*0x190+0x48);
	const byte *unit= object_try_and_get_and_verify_type(object_index, 3);

	if (unit)
	{
		long player_index= *(const long *)(unit+0x13C);

		if (player_index!=NONE)
		{
			const byte *player= (const byte *)player_data->data+(player_index&0xFFFF)*0x21C;

			return *(const short *)(player+0x28)==value;
		}
	}

	return FALSE;
}

/* ---------- sixteen pairs of an effect, and something of 0x60 bytes */

enum
{
	/* in an effect: sixteen pairs of numbers, free when they are 0 and
	NONE */
	EFFECT_PAIRS_OFFSET= 0x110,
	NUMBER_OF_EFFECT_PAIRS= 16
};

/* the sixteen pairs of an effect */
static inline __attribute__((always_inline)) long *effect_get_pairs(
	long effect_index)
{
	return (long *)((byte *)effect_data->data+(effect_index&0xFFFF)*0x190+EFFECT_PAIRS_OFFSET);
}

/* 0x00175ee0: something of 0x60 bytes is made: zeros, NONE in five numbers
and two shorts, 1 in two reals and a color at 0x54 and 0x58 */
void effect_unknown_00175ee0_new(
	byte *unknown)
{
	memset(unknown, 0, 0x60);
	*(long *)(unknown+0x4)= NONE;
	*(short *)(unknown+0x18)= NONE;
	*(long *)(unknown+0x8)= NONE;
	*(long *)(unknown+0x10)= NONE;
	*(long *)(unknown+0xC)= NONE;
	*(short *)(unknown+0x14)= NONE;
	*(long *)(unknown+0x34)= 0;
	*(long *)(unknown+0x38)= 0;
	*(real *)(unknown+0x40)= 1.0f;
	*(real *)(unknown+0x44)= 1.0f;
	*(short *)(unknown+0x3C)= 0;
	*(long *)(unknown+0x30)= 0;
	*(unsigned long *)(unknown+0x54)= 0xFF808080;
	*(unsigned long *)(unknown+0x58)= 0xFF808080;
	*(long *)(unknown+0x5C)= 0;
}

/* 0x00178ad0: the sixteen pairs of an effect are free */
void effect_clear_pairs(
	byte *effect)
{
	long *pairs= (long *)(effect+EFFECT_PAIRS_OFFSET);
	long index;

	for (index= 0; index<NUMBER_OF_EFFECT_PAIRS; index++)
	{
		pairs[2*index]= 0;
		pairs[2*index+1]= NONE;
	}
}

/* 0x00178af0: whether an effect has a free pair */
boolean effect_has_free_pair(
	long effect_index)
{
	const long *pairs= effect_get_pairs(effect_index);
	long index;

	for (index= 0; index<NUMBER_OF_EFFECT_PAIRS; index++)
	{
		if (pairs[2*index+1]==NONE && pairs[2*index]==0)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* 0x00178b30: two numbers go in the first free pair of an effect, if it has
one */
void __attribute__((stdcall)) effect_add_pair(
	long effect_index,
	long first,
	long second)
{
	long *pairs= effect_get_pairs(effect_index);
	long index;

	for (index= 0; index<NUMBER_OF_EFFECT_PAIRS; index++)
	{
		if (pairs[2*index+1]==NONE && pairs[2*index]==0)
		{
			pairs[2*index+1]= second;
			pairs[2*index]= first;
			return;
		}
	}
}
