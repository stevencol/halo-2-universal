/*
LIGHTS.C

the lights of the world. Worked out from halo2ship.exe; the name of the file
is a guess, by the name that the game gives to its table of data ("lights").
*/

#include "cseries/cseries.h"

#include <string.h>

#include "cache/cache_files.h"
#include "game/game.h"
#include "memory/data.h"
#include "saved games/game_state.h"

/* ---------- constants */

enum
{
	MAXIMUM_LIGHTS= 0x15E,
	LIGHT_SIZE= 0x110,

	/* of what light_globals points to */
	LIGHT_GLOBALS_SIZE= 0xC,

	/* of the first thing of unknown_004e0310: 0x200 numbers */
	LIGHT_UNKNOWN_SIZE= 0x800,

	/* of a light: a byte of bits, and two numbers that are NONE in a light
	that nothing has (of what, is not known) */
	LIGHT_FLAGS_OFFSET= 2,
	LIGHT_UNKNOWN10_OFFSET= 0x10,
	LIGHT_UNKNOWN4C_OFFSET= 0x4C,
	LIGHT_UNKNOWN58_OFFSET= 0x58
};

/* ---------- structures */

/* what unknown_001cabc0 fills for the lights, with the name "light": 0x800
bytes of the state of the game and two tables of data. It may be where the
lights are in the world, cluster by cluster, but that is a guess */
struct light_unknown_004e0310
{
	long *unknown0;
	struct data_array *unknown4;
	struct data_array *unknown8;
};

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
struct light_unknown_004e0310 unknown_004e0310;

/* ---------- prototypes */

void __attribute__((stdcall)) unknown_001cabc0(struct light_unknown_004e0310 *unknown, const char *name,
	long unknown3);
void unknown_001cae40(struct light_unknown_004e0310 *unknown, long light_index, byte *unknown10);
void __attribute__((stdcall)) unknown_000c2d00(long light_index);
void __attribute__((stdcall)) unknown_000c0c80(long light_index);
void __attribute__((stdcall)) unknown_000c3260(long light_index, boolean unknown);

/* ---------- code */

/* 0x000bffa0: the table is made, in the memory of the state of the game
(data_new, which the game has written here), and the 0xC bytes are taken
there too (game_state_malloc, written here as well). If there is a table,
unknown_001cabc0 fills the three things */
void lights_initialize(
	void)
{
	light_data= data_new("lights", MAXIMUM_LIGHTS, LIGHT_SIZE, 0, game_state_allocation);
	light_globals= game_state_malloc(LIGHT_GLOBALS_SIZE);
	light_globals[8]= TRUE;
	if (light_data)
	{
		unknown_001cabc0(&unknown_004e0310, "light", 0);
	}
}

/* 0x000c0040: for a new map the table is good to use, with nothing in it,
and so are the two of the three things, whose 0x200 numbers are all NONE */
void lights_initialize_for_new_map(
	void)
{
	data_make_valid(light_data);
	light_globals[8]= TRUE;
	memset(unknown_004e0310.unknown0, 0xFF, LIGHT_UNKNOWN_SIZE);
	data_make_valid(unknown_004e0310.unknown8);
	data_make_valid(unknown_004e0310.unknown4);
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
and 1 is given to unknown_001cae40, with what it has at 0x10, and loses the
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
				unknown_001cae40(&unknown_004e0310, light_index, light+LIGHT_UNKNOWN10_OFFSET);
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
