/*
LIGHTS.C

the lights of the world. Worked out from halo2ship.exe; the name of the file
is a guess, by the name that the game gives to its table of data ("lights").
*/

#include "cseries/cseries.h"

#include <string.h>

#include "cache/cache_files.h"
#include "game/game.h"
#include "math/real_math.h"
#include "memory/data.h"
#include "objects/objects.h"
#include "objects/cluster_partitions.h"
#include "saved games/game_state.h"
#include "scenario/scenario.h"

/* ---------- constants */

enum
{
	MAXIMUM_LIGHTS= 0x15E,
	LIGHT_SIZE= 0x110,

	/* of what light_globals points to */
	LIGHT_GLOBALS_SIZE= 0xC,

	/* of the first thing of light_cluster_partition: 0x200 numbers */
	LIGHT_UNKNOWN_SIZE= 0x800,

	/* of a light: a byte of bits, and two numbers that are NONE in a light
	that nothing has (of what, is not known) */
	LIGHT_FLAGS_OFFSET= 2,
	LIGHT_UNKNOWN10_OFFSET= 0x10,
	LIGHT_UNKNOWN4C_OFFSET= 0x4C,
	LIGHT_UNKNOWN58_OFFSET= 0x58
};

/* ---------- structures */

/* which lights are in each cluster of the structure bsp is kept in a
struct cluster_partition (cluster_partitions.h) */

/* ---------- globals */

/* 0x004e030c: the lights ("lights": 0x15E of 0x110 bytes). 0x005107e8: 0xC
bytes of the state of the game (the name is a guess): a byte, a number at 4
and a byte at 8 that is set when the lights start. 0x004e0310: see the
structure */
struct data_array *light_data;
byte *light_globals;

/* 0x004e0308: a number that a new light keeps, less one (a count of the
times that something has been done to the lights, perhaps) */
long unknown_004e0308;
struct cluster_partition light_cluster_partition;

/* ---------- prototypes */

void __attribute__((stdcall)) cluster_partition_initialize(struct cluster_partition *unknown, const char *name,
	long unknown3);
void __attribute__((stdcall)) unknown_000c2d00(long light_index);
void __attribute__((stdcall)) unknown_000c0c80(long light_index);
void __attribute__((stdcall)) unknown_000c3260(long light_index, boolean unknown);

/* ---------- code */

/* 0x000bffa0: the table is made, in the memory of the state of the game
(data_new, which the game has written here), and the 0xC bytes are taken
there too (game_state_malloc, written here as well). If there is a table,
cluster_partition_initialize fills the three things */
void lights_initialize(
	void)
{
	light_data= data_new("lights", MAXIMUM_LIGHTS, LIGHT_SIZE, 0, game_state_allocation);
	light_globals= game_state_malloc(LIGHT_GLOBALS_SIZE);
	light_globals[8]= TRUE;
	if (light_data)
	{
		cluster_partition_initialize(&light_cluster_partition, "light", 0);
	}
}

/* 0x000c0040: for a new map the table is good to use, with nothing in it,
and so are the two of the three things, whose 0x200 numbers are all NONE */
void lights_initialize_for_new_map(
	void)
{
	data_make_valid(light_data);
	light_globals[8]= TRUE;
	memset(light_cluster_partition.unknown0, 0xFF, LIGHT_UNKNOWN_SIZE);
	data_make_valid(light_cluster_partition.unknown8);
	data_make_valid(light_cluster_partition.unknown4);
	light_globals[0]= FALSE;
	*(long *)(light_globals+4)= 0;
}

/* 0x000c00a0: unknown_000c2d00 is done for each light whose two numbers are
NONE. The game has the walk over the table written here, as in the two that
follow */
void lights_unknown_000c00a0(
	void)
{
	long absolute_index= NONE;

	for (;;)
	{
		const byte *light;
		long light_index;

		absolute_index= data_next_absolute_index(light_data, absolute_index+1);
		if (absolute_index==NONE)
		{
			break;
		}
		light= (const byte *)light_data->data+absolute_index*light_data->size;
		light_index= (*(const short *)light<<16)|absolute_index;
		if (*(const long *)(light+LIGHT_UNKNOWN4C_OFFSET)==NONE && *(const long *)(light+LIGHT_UNKNOWN58_OFFSET)==NONE)
		{
			unknown_000c2d00(light_index);
		}
	}
}

/* 0x000c0110: each light whose two numbers are NONE and that has the bits 3
and 1 is given to cluster_partition_remove, with what it has at 0x10, and loses the
bit 3 */
void lights_unknown_000c0110(
	void)
{
	long absolute_index= NONE;

	for (;;)
	{
		const byte *datum;
		long light_index;

		absolute_index= data_next_absolute_index(light_data, absolute_index+1);
		if (absolute_index==NONE)
		{
			break;
		}
		datum= (const byte *)light_data->data+absolute_index*light_data->size;
		light_index= (*(const short *)datum<<16)|absolute_index;
		if (*(const long *)(datum+LIGHT_UNKNOWN4C_OFFSET)==NONE && *(const long *)(datum+LIGHT_UNKNOWN58_OFFSET)==NONE &&
			(datum[LIGHT_FLAGS_OFFSET]&8))
		{
			byte *light= (byte *)light_data->data+(light_index&0xFFFF)*LIGHT_SIZE;

			if (light[LIGHT_FLAGS_OFFSET]&2)
			{
				cluster_partition_remove(&light_cluster_partition, light_index, (long *)(light+LIGHT_UNKNOWN10_OFFSET));
				light[LIGHT_FLAGS_OFFSET]&= ~8;
			}
		}
	}
}

/* 0x000c01c0: unknown_000c3260 is done, with TRUE, for each light whose
first number is NONE */
void lights_unknown_000c01c0(
	void)
{
	long absolute_index= NONE;

	for (;;)
	{
		const byte *light;
		long light_index;

		absolute_index= data_next_absolute_index(light_data, absolute_index+1);
		if (absolute_index==NONE)
		{
			break;
		}
		light= (const byte *)light_data->data+absolute_index*light_data->size;
		light_index= (*(const short *)light<<16)|absolute_index;
		if (*(const long *)(light+LIGHT_UNKNOWN4C_OFFSET)==NONE)
		{
			unknown_000c3260(light_index, TRUE);
		}
	}
}

/* 0x000c0230: a new light, of a definition (a tag): its index, or NONE if
there is no room, or if it is not to be made: a definition with the bit 0
and NONE at 0x94 makes none, and one whose short at 0xA4 is 2 makes one only
in the kinds of game 4 and 5. What the four other arguments are is not known:
they are kept at 0x4C, 0x54, 0x50 and 0x56. The light takes the bit 0 when
its definition does not have it */
long __attribute__((stdcall)) light_new(
	long definition_index,
	long unknown4C,
	short unknown54,
	long unknown50,
	short unknown56)
{
	const byte *definition= (const byte *)(uintptr_t)global_tag_instances[definition_index&0xFFFF].address;
	long game_mode= main_game_globals->options.game_mode;
	long light_index= NONE;
	boolean unknown_mode= game_mode==_game_mode_5 || game_mode==_game_mode_4;
	boolean unknown2= *(const short *)(definition+0xA4)==2;

	if ((!(definition[0]&1) || *(const long *)(definition+0x94)!=NONE) && (unknown_mode || !unknown2))
	{
		light_index= datum_new(light_data);
		if (light_index!=NONE)
		{
			byte *light= (byte *)light_data->data+(light_index&0xFFFF)*LIGHT_SIZE;

			*(long *)(light+4)= definition_index;
			*(long *)(light+LIGHT_UNKNOWN4C_OFFSET)= unknown4C;
			*(short *)(light+LIGHT_FLAGS_OFFSET)= 0;
			*(short *)(light+0x54)= unknown54;
			*(long *)(light+0x50)= unknown50;
			*(long *)(light+0x14)= NONE;
			*(long *)(light+8)= NONE;
			*(short *)(light+0x56)= unknown56;
			*(long *)(light+LIGHT_UNKNOWN58_OFFSET)= NONE;
			*(short *)(light+0x5C)= NONE;
			*(float *)(light+0xD0)= 1.0f;
			*(float *)(light+0xCC)= 1.0f;
			*(short *)(light+LIGHT_FLAGS_OFFSET)= !(definition[0]&1);
			light[LIGHT_FLAGS_OFFSET]&= ~0x10;
			*(long *)(light+LIGHT_UNKNOWN10_OFFSET)= NONE;
			unknown_000c0c80(light_index);
			*(long *)(light+0xC)= unknown_004e0308-1;
			*(long *)(light+0x10C)= NONE;
		}
	}

	return light_index;
}

/* 0x000c0520: a light of the scenario is made, of a definition (a tag, which
the game gives in eax) */
void unknown_000c0520(long definition_index, long placement_index, boolean unknown_mode, const byte *placement);

/* 0x000c09c0: each light that the scenario places (it has them at 0xE8, of
0x6C bytes each, which start with which of its kinds of light it is, a short)
is made, if its kind (the scenario has them at 0xF0, of 0x28 bytes) has a tag.
It is told whether the kind of game is 4 or 5, as light_new looks at */
void lights_place_scenario_lights(
	long game_mode)
{
	boolean unknown_mode= game_mode==_game_mode_5 || game_mode==_game_mode_4;
	const byte *scenario= (const byte *)global_scenario;
	long placement_index;

	for (placement_index= 0; placement_index<*(const long *)(scenario+0xE8); placement_index++)
	{
		const byte *placement= *(const byte *const *)(scenario+0xEC)+placement_index*0x6C;
		long type= *(const short *)placement;

		if (type!=NONE && type>=0 && type<*(const long *)(scenario+0xF0))
		{
			long definition_index= *(const long *)(*(const byte *const *)(scenario+0xF4)+type*0x28+4);

			if (definition_index!=NONE)
			{
				unknown_000c0520(definition_index, placement_index, unknown_mode, placement);
				scenario= (const byte *)global_scenario;
			}
		}
	}
}

/* 0x000c0350: a new light, of a definition (a tag), at a place and turned
some way (a point and two directions), with a size: its index, or NONE if
there is no room. With an object the place is said from the object (and from
one of its markers, a short), and the object gets the bit 6 of its flags;
with none it is a place of the world, and the light keeps where its first
direction gets to from it as well. A light of a definition whose real at 0xB0
is above zero takes the bit 4 */
long light_new_at(
	long definition_index,
	long object_index,
	short marker_index,
	const real_vector3d *up,
	const real_point3d *position,
	const real_vector3d *forward,
	real scale)
{
	long light_index= datum_new(light_data);

	if (light_index!=NONE)
	{
		byte *light= (byte *)light_data->data+(light_index&0xFFFF)*LIGHT_SIZE;
		const byte *definition= (const byte *)(uintptr_t)global_tag_instances[definition_index&0xFFFF].address;

		*(long *)(light+4)= definition_index;
		*(short *)(light+LIGHT_FLAGS_OFFSET)= 0;
		*(long *)(light+0x14)= (long)game_time_globals->time;
		*(long *)(light+0x50)= 0;
		*(short *)(light+0x56)= 0;
		*(long *)(light+8)= NONE;
		*(long *)(light+LIGHT_UNKNOWN4C_OFFSET)= NONE;
		*(short *)(light+0x54)= NONE;
		*(float *)(light+0xD0)= scale;
		*(float *)(light+0xCC)= scale;
		*(short *)(light+LIGHT_FLAGS_OFFSET)= 1;
		if (*(const float *)(definition+0xB0)>0.0f)
		{
			*(short *)(light+LIGHT_FLAGS_OFFSET)= 0x11;
		}
		else
		{
			*(short *)(light+LIGHT_FLAGS_OFFSET)= 1;
		}
		*(long *)(light+LIGHT_UNKNOWN10_OFFSET)= NONE;
		if (object_index==NONE)
		{
			real_point3d *end= (real_point3d *)(light+0x90);

			*(real_point3d *)(light+0x84)= *position;
			end->x= position->x+forward->i;
			end->y= position->y+forward->j;
			end->z= position->z+forward->k;
			*(real_vector3d *)(light+0xAC)= *forward;
			*(real_vector3d *)(light+0xB8)= *up;
			*(long *)(light+LIGHT_UNKNOWN58_OFFSET)= NONE;
			*(short *)(light+0x5C)= NONE;
		}
		else
		{
			byte *object= (byte *)((struct object_header_datum *)object_header_data->data)[object_index&0xFFFF].datum;

			*(real_point3d *)(light+0x60)= *position;
			*(real_vector3d *)(light+0x6C)= *forward;
			*(real_vector3d *)(light+0x78)= *up;
			*(short *)(light+0x5C)= marker_index;
			*(long *)(light+LIGHT_UNKNOWN58_OFFSET)= object_index;
			*(unsigned long *)(object+4)|= 0x40;
		}
		unknown_000c0c80(light_index);
		*(long *)(light+0xC)= unknown_004e0308-1;
		*(long *)(light+0x10C)= NONE;
	}

	return light_index;
}

enum
{
	/* of what the three functions that follow fill: a number that says what
	it is like (0 to 3, at least), and what goes with it */
	LIGHT_UNKNOWN_000C1930_SIZE= 0x7C
};

/* 0x000c1930: what a light is like, from its definition: its kind (a short
at 4 of the definition) and, for the kind 0, two numbers of the definition
(from 0x18); for the others, five (from 0x20) */
void light_unknown_000c1930(
	const byte *definition,
	byte *unknown)
{
	memset(unknown, 0, LIGHT_UNKNOWN_000C1930_SIZE);
	*(long *)unknown= *(const short *)(definition+4);
	if (*(const short *)(definition+4)==0)
	{
		*(long *)(unknown+4)= *(const long *)(definition+0x18);
		*(long *)(unknown+8)= *(const long *)(definition+0x1C);
	}
	else
	{
		*(long *)(unknown+4)= *(const long *)(definition+0x20);
		*(long *)(unknown+8)= *(const long *)(definition+0x24);
		*(long *)(unknown+0xC)= *(const long *)(definition+0x28);
		*(long *)(unknown+0x10)= *(const long *)(definition+0x2C);
		*(long *)(unknown+0x14)= *(const long *)(definition+0x30);
	}
}

/* 0x000c1980: the same, from where the scenario places the light: its kind
is a short at 0x3C there. For the kind 0 the two numbers are the real at 0x68
of the place, the second by what the second of the definition is of the first
(unless that second is zero); for the others, the five from 0x58 of the place */
void light_unknown_000c1980(
	const byte *placement,
	byte *unknown,
	const byte *definition)
{
	memset(unknown, 0, LIGHT_UNKNOWN_000C1930_SIZE);
	*(long *)unknown= *(const short *)(placement+0x3C);
	if (*(const short *)(placement+0x3C)==0)
	{
		*(long *)(unknown+4)= *(const long *)(placement+0x68);
		*(float *)(unknown+8)= *(const float *)(placement+0x68);
		if (*(const float *)(definition+0x1C)!=0.0f)
		{
			*(float *)(unknown+8)= *(const float *)(definition+0x1C)/ *(const float *)(definition+0x18)*
				*(const float *)(placement+0x68);
		}
	}
	else
	{
		*(long *)(unknown+4)= *(const long *)(placement+0x58);
		*(long *)(unknown+8)= *(const long *)(placement+0x5C);
		*(long *)(unknown+0xC)= *(const long *)(placement+0x60);
		*(long *)(unknown+0x10)= *(const long *)(placement+0x64);
		*(long *)(unknown+0x14)= *(const long *)(placement+0x68);
	}
}

/* 0x000c19f0: what a light is like: from where the scenario places it (the
light has which at 8), if the place has the bit 0 of its byte at 0x3E; if
not, from its definition. Then the kind 1 has zero for its third number, and
the kind 3 for its first */
void light_unknown_000c19f0(
	long light_index,
	byte *unknown)
{
	const byte *light= (const byte *)light_data->data+(light_index&0xFFFF)*LIGHT_SIZE;
	const byte *definition= (const byte *)(uintptr_t)global_tag_instances[*(const long *)(light+4)&0xFFFF].address;
	long placement_index= *(const long *)(light+8);
	const byte *placement= NULL;

	if (placement_index!=NONE)
	{
		placement= *(const byte *const *)((const byte *)global_scenario+0xEC)+placement_index*0x6C;
		if (!(placement[0x3E]&1))
		{
			placement= NULL;
		}
	}
	if (placement)
	{
		light_unknown_000c1980(placement, unknown, definition);
	}
	else
	{
		light_unknown_000c1930(definition, unknown);
	}
	if (*(long *)unknown==1)
	{
		*(float *)(unknown+0xC)= 0.0f;
	}
	else if (*(long *)unknown==3)
	{
		*(float *)(unknown+4)= 0.0f;
	}
}
