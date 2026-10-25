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

/* 0x0004cbb0: whether the element (0x1C bytes) of a tag that starts with a
count (0x04) and its elements (0x08) has, in the long at 0x0C, a number
above 0xF; not for NONE or an element that is not there */
long tag_unknown_0004cbb0(
	long tag_index,
	unsigned long index)
{
	if (tag_index!=NONE)
	{
		const byte *tag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;

		if (index<*(const unsigned long *)(tag+0x04)
			&& *(const long *)(*(const byte *const *)(tag+0x08)+index*0x1C+0x0C)>>4!=0)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* 0x0004dc60: an entry (0x0C bytes) of one of two lists of something (at
0x54, or at 0x64 and then the other-th of its shorts) names an element (0x38
bytes) of the list at 0x44; if that has the bit 0x02, what it points to
gets the entry's short twice, times four if its text starts with a dot and
times three if not, and the text is returned. Otherwise nothing */
byte *__attribute__((stdcall)) unknown_0004dc60(
	long index,
	const byte *data,
	boolean first,
	long other)
{
	const byte *entry;
	const unsigned short *value;
	const byte *element;

	if (first)
	{
		entry= *(const byte *const *)(data+0x54)+index*0x0C;
		value= *(const unsigned short *const *)(entry+0x08);
	}
	else
	{
		entry= *(const byte *const *)(data+0x64)+index*0x0C;
		value= *(const unsigned short *const *)(entry+0x08)+other;
	}
	element= *(const byte *const *)(data+0x44)+*(const unsigned short *)(entry+0x02)*0x38;
	if (*element&0x02)
	{
		byte *text= *(byte *const *)(*(const byte *const *)(element+0x34)+0x04)+0x20;

		if (text[0]=='.')
		{
			((long *)(text+0x04))[0]= *value<<2;
			((long *)(text+0x04))[1]= *value<<2;
		}
		else
		{
			((long *)(text+0x04))[0]= *value*3;
			((long *)(text+0x04))[1]= *value*3;
		}
		return text;
	}

	return NULL;
}

/* 0x00173b00: a short goes first in the list of an index: what was first
before is its next (0x440, NONE for the first one, by the bits at 0x00) */
void effect_unknown_00173b00(
	short index,
	short value,
	byte *data)
{
	unsigned long *bits= (unsigned long *)data+(index>>5);

	if (!(*bits&(1UL<<(index&0x1F))))
	{
		((short *)(data+0x440))[value]= NONE;
		((short *)(data+0x40))[index]= value;
		*bits|= 1UL<<(index&0x1F);
		return;
	}
	((short *)(data+0x440))[value]= ((const short *)(data+0x40))[index];
	((short *)(data+0x40))[index]= value;
}

/* 0x00176330: something of 0x38 bytes from a point: the point at 0x00 and
0x1C, global_down3d at 0x0C, global_up3d at 0x28 and two names (0x070000C0
and 0x020000CA) */
void effect_unknown_00176330(
	unsigned long *data,
	const unsigned long *point)
{
	const unsigned long *down= (const unsigned long *)global_down3d;
	const unsigned long *up= (const unsigned long *)global_up3d;

	data[3]= down[0];
	data[4]= down[1];
	data[5]= down[2];
	data[0]= point[0];
	data[1]= point[1];
	data[2]= point[2];
	data[6]= 0x070000C0;
	data[10]= up[0];
	data[11]= up[1];
	data[12]= up[2];
	data[7]= point[0];
	data[8]= point[1];
	data[9]= point[2];
	data[13]= 0x020000CA;
}

/* 0x00468710 (player_profile.c) */
extern const unsigned long *player_color_default;

/* 0x00178240: something gets two reals at 0x6C and 0x70, a colour at 0x24
(the default for none) and a vector at 0x3C (none: 0.0 at 0x40 and 0x44) */
void __attribute__((stdcall)) effect_unknown_00178240(
	byte *data,
	const unsigned long *color,
	const unsigned long *vector,
	real first,
	real second)
{
	*(real *)(data+0x6C)= first;
	if (!color)
	{
		color= player_color_default;
	}
	*(real *)(data+0x70)= second;
	((unsigned long *)(data+0x24))[0]= color[0];
	((unsigned long *)(data+0x24))[1]= color[1];
	((unsigned long *)(data+0x24))[2]= color[2];
	if (vector)
	{
		((unsigned long *)(data+0x3C))[0]= vector[0];
		((unsigned long *)(data+0x3C))[1]= vector[1];
		((unsigned long *)(data+0x3C))[2]= vector[2];
		return;
	}
	*(unsigned long *)(data+0x40)= 0;
	*(unsigned long *)(data+0x44)= 0;
}

/* 0x0017b750: 32 longs get a value */
void effect_unknown_0017b750(
	unsigned long value,
	unsigned long *values)
{
	long index;

	for (index= 0; index<32; index++)
	{
		values[index]= value;
	}
}

/* the index of an element (0x54 bytes) of the particle systems, with its salt */
#define PARTICLE_SYSTEM_INDEX(element) \
	((long)*(const short *)(element)<<16 | (long)((element)-(const byte *)particle_system_data->data)/0x54)
#define PARTICLE_SYSTEM_GET(index) ((byte *)particle_system_data->data+((index)&0xFFFF)*0x54)

/* 0x00175070: a particle system goes last in a list (its first and last are
said; each has the one before at 0x3C and the one after at 0x38) */
void __attribute__((stdcall)) particle_system_list_add(
	byte *element,
	long *last,
	long *first)
{
	long index= PARTICLE_SYSTEM_INDEX(element);

	*(long *)(element+0x38)= NONE;
	if (*first==NONE)
	{
		*first= index;
	}
	if (*last!=NONE)
	{
		byte *other= PARTICLE_SYSTEM_GET(*last);

		*(long *)(other+0x38)= index;
		*(long *)(element+0x3C)= PARTICLE_SYSTEM_INDEX(other);
		*last= index;
		return;
	}
	*last= index;
	*(long *)(element+0x3C)= NONE;
}

/* 0x00175100: it is taken out of the list */
void __attribute__((stdcall)) particle_system_list_remove(
	byte *element,
	long *last,
	long *first)
{
	long index= PARTICLE_SYSTEM_INDEX(element);

	if (*(const long *)(element+0x38)!=NONE)
	{
		*(long *)(PARTICLE_SYSTEM_GET(*(const long *)(element+0x38))+0x3C)= *(const long *)(element+0x3C);
	}
	if (*(const long *)(element+0x3C)!=NONE)
	{
		*(long *)(PARTICLE_SYSTEM_GET(*(const long *)(element+0x3C))+0x38)= *(const long *)(element+0x38);
	}
	if (*last==index)
	{
		*last= *(const long *)(element+0x38);
	}
	if (*first==index)
	{
		*first= *(const long *)(element+0x3C);
	}
	*(long *)(element+0x38)= NONE;
	*(long *)(element+0x3C)= NONE;
}

/* 0x00177590: a valid effect (its index and salt right) is taken out of the
entries of the object that it names at 0x48 (those of the object's short at
0x11E, of kind 2, with the effect's index) */
void effect_unknown_00177590(
	long effect_index)
{
	const byte *effect;
	byte *object;
	byte *entries;
	unsigned long count;
	unsigned long index;

	if (effect_index==NONE || (effect_index&0xFFFF)>=effect_data->first_unallocated)
	{
		return;
	}
	effect= (const byte *)effect_data->data+(effect_index&0xFFFF)*effect_data->size;
	if (!*(const short *)effect || *(const short *)effect!=effect_index>>16)
	{
		return;
	}
	object= (byte *)object_try_and_get_and_verify_type(*(const long *)(effect+0x48), 0xFFFFFFFF);
	if (!object)
	{
		return;
	}
	entries= object+*(const short *)(object+0x11E);
	count= (unsigned long)(long)*(const short *)(object+0x11C)>>3;
	for (index= 0; (long)index<(long)count; index++)
	{
		if (*(const long *)(entries+index*8+4)==effect_index && entries[index*8]==2)
		{
			*(long *)(entries+index*8+4)= NONE;
			return;
		}
	}
}

/* 0x00248d50: the shorts at 0x02 of the particle emitters (0x4C bytes) of
something, from the one at its 0x04, each to the next at 0x08, added */
long particle_unknown_00248d50(
	const byte *data)
{
	long emitter_index= *(const long *)(data+0x04);
	long total= 0;

	while (emitter_index!=NONE)
	{
		const byte *emitter= (const byte *)particle_emitter_data->data+(emitter_index&0xFFFF)*0x4C;

		total+= *(const unsigned short *)(emitter+0x02);
		emitter_index= *(const long *)(emitter+0x08);
	}

	return total;
}
