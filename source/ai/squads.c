/*
SQUADS.C

the squads of the characters, and the groups of squads. Worked out from halo2ship.exe; the name of the file is a guess, by
the names that the game gives to its tables of data ("squad", "squad group").
*/

#include "cseries/cseries.h"
#include <stdint.h>

#include "cache/cache_files.h"
#include "memory/data.h"
#include "saved games/game_state.h"
#include "scenario/scenario.h"

/* ---------- globals */

/* 0x0051e9d8: the squads ("squad": 0x14F of 0x98 bytes). 0x0051e9dc: their
groups ("squad group": 0x64 of 0x38 bytes). 0x00557c74: five numbers that are
NONE when the squads start */
struct data_array *squad_data;
struct data_array *squad_group_data;
long unknown_00557c74[5];

/* ---------- code */

/* 0x00200930: the five numbers are NONE, and the two tables are made, in the
memory of the state of the game (data_new, which the game has written here) */
void squads_initialize(
	void)
{
	long index;

	for (index= 0; index<5; index++)
	{
		unknown_00557c74[index]= NONE;
	}
	squad_data= data_new("squad", 0x14F, 0x98, 0, game_state_allocation);
	squad_group_data= data_new("squad group", 0x64, 0x38, 0, game_state_allocation);
}

/* ---------- what is known of a squad, of a group, and of the scenario */

enum
{
	SQUAD_SIZE= 0x98,
	/* the group that the squad is of, or NONE; and the next squad of that
	group, a short, or NONE */
	SQUAD_GROUP_INDEX_OFFSET= 0x6C,
	SQUAD_NEXT_SQUAD_OFFSET= 0x74,

	SQUAD_GROUP_SIZE= 0x38,
	/* its first group, its first squad, the next group of the group that it
	is of, and that group; each NONE if there is none */
	SQUAD_GROUP_FIRST_CHILD_OFFSET= 4,
	SQUAD_GROUP_FIRST_SQUAD_OFFSET= 8,
	SQUAD_GROUP_NEXT_SIBLING_OFFSET= 0xC,
	SQUAD_GROUP_PARENT_OFFSET= 0x10,

	/* the scenario has its groups at 0x158 (of 0x24 bytes, with the group
	that each is of in a short at 0x20) and its squads at 0x160 (of 0x74
	bytes, with the group of each in a short at 0x26) */
	SCENARIO_SQUAD_GROUPS_OFFSET= 0x158,
	SCENARIO_SQUAD_GROUP_SIZE= 0x24,
	SCENARIO_SQUAD_GROUP_PARENT_OFFSET= 0x20,
	SCENARIO_SQUADS_OFFSET= 0x160,
	SCENARIO_SQUAD_SIZE= 0x74,
	SCENARIO_SQUAD_GROUP_OFFSET= 0x26
};

#define SQUAD_GET(index) ((byte *)squad_data->data+((index)&0xFFFF)*SQUAD_SIZE)
#define SQUAD_GROUP_GET(index) ((byte *)squad_group_data->data+((index)&0xFFFF)*SQUAD_GROUP_SIZE)

/* 0x00200c20: a squad of the scenario gets its squad of the game; it is
below */
void squad_new_from_scenario(const byte *scenario_squad);

/* 0x001e4990, of actors.c */
void *character_definition_get_unknown34(long character_index);

/* 0x0044aee8: for each of the 0x14 kinds of character, the side that it is
on when nothing else says */
const short unknown_0044aee8[0x14]=
{
	3, 7, 3, 3, 3, 3, 1, 2, 2, 4, 4, 4, 5, 5, NONE, 3, 7, 7, 7, 4
};

/* 0x00205280: a squad is put last in the list of the squads of a group. In
the game it is far from the others of this file, so it may be of another */
void squad_group_add_squad(
	long group_index,
	long squad_index)
{
	byte *group= SQUAD_GROUP_GET(group_index);
	long other_index= *(long *)(group+SQUAD_GROUP_FIRST_SQUAD_OFFSET);

	if (other_index==NONE)
	{
		*(long *)(group+SQUAD_GROUP_FIRST_SQUAD_OFFSET)= squad_index;
	}
	else
	{
		byte *squad;

		do
		{
			squad= SQUAD_GET(other_index);
			other_index= *(short *)(squad+SQUAD_NEXT_SQUAD_OFFSET);
		} while (other_index!=NONE);
		*(short *)(squad+SQUAD_NEXT_SQUAD_OFFSET)= (short)squad_index;
	}
}

/* 0x002009d0: the groups and the squads of the game, which are those of the
scenario in the same order, are put in their lists: no group has groups or
squads, and each takes the group that the scenario says that it is of; then
each group is put last among the groups of its group, and each squad last
among the squads of its group */
void squads_unknown_002009d0(
	void)
{
	const byte *scenario= (const byte *)global_scenario;
	long index;

	for (index= 0; (short)index<*(const long *)(scenario+SCENARIO_SQUAD_GROUPS_OFFSET); index++)
	{
		byte *group= SQUAD_GROUP_GET(index);

		*(long *)(group+SQUAD_GROUP_FIRST_CHILD_OFFSET)= NONE;
		*(long *)(group+SQUAD_GROUP_FIRST_SQUAD_OFFSET)= NONE;
		*(long *)(group+SQUAD_GROUP_NEXT_SIBLING_OFFSET)= NONE;
		*(long *)(group+SQUAD_GROUP_PARENT_OFFSET)= *(const short *)(*(const byte *const *)(scenario+
			SCENARIO_SQUAD_GROUPS_OFFSET+4)+(index&0xFFFF)*SCENARIO_SQUAD_GROUP_SIZE+SCENARIO_SQUAD_GROUP_PARENT_OFFSET);
	}
	for (index= 0; (short)index<*(const long *)(scenario+SCENARIO_SQUADS_OFFSET); index++)
	{
		byte *squad= SQUAD_GET(index);

		*(short *)(squad+SQUAD_NEXT_SQUAD_OFFSET)= NONE;
		*(long *)(squad+SQUAD_GROUP_INDEX_OFFSET)= *(const short *)(*(const byte *const *)(scenario+
			SCENARIO_SQUADS_OFFSET+4)+(index&0xFFFF)*SCENARIO_SQUAD_SIZE+SCENARIO_SQUAD_GROUP_OFFSET);
	}
	for (index= 0; (short)index<*(const long *)(scenario+SCENARIO_SQUAD_GROUPS_OFFSET); index++)
	{
		long parent_index= *(long *)(SQUAD_GROUP_GET(index)+SQUAD_GROUP_PARENT_OFFSET);

		if (parent_index!=NONE)
		{
			byte *other= SQUAD_GROUP_GET(parent_index);
			long other_index= *(long *)(other+SQUAD_GROUP_FIRST_CHILD_OFFSET);

			if (other_index==NONE)
			{
				*(long *)(other+SQUAD_GROUP_FIRST_CHILD_OFFSET)= (short)index;
			}
			else
			{
				do
				{
					other= SQUAD_GROUP_GET(other_index);
					other_index= *(long *)(other+SQUAD_GROUP_NEXT_SIBLING_OFFSET);
				} while (other_index!=NONE);
				*(long *)(other+SQUAD_GROUP_NEXT_SIBLING_OFFSET)= (short)index;
			}
		}
	}
	for (index= 0; (short)index<*(const long *)(scenario+SCENARIO_SQUADS_OFFSET); index++)
	{
		long group_index= *(long *)(SQUAD_GET(index)+SQUAD_GROUP_INDEX_OFFSET);

		if (group_index!=NONE)
		{
			squad_group_add_squad(group_index, (short)index);
		}
	}
}

/* 0x00200b30: for a new map the two tables are good to use, with nothing in
them; each squad of the scenario gets its squad (squad_new_from_scenario) and each
group of the scenario its group, which is of nothing yet; and they are put in
their lists */
void squads_initialize_for_new_map(
	void)
{
	const byte *scenario= (const byte *)global_scenario;
	long index;

	data_make_valid(squad_data);
	data_make_valid(squad_group_data);
	for (index= 0; (short)index<*(const long *)(scenario+SCENARIO_SQUADS_OFFSET); index++)
	{
		squad_new_from_scenario(*(const byte *const *)(scenario+SCENARIO_SQUADS_OFFSET+4)+(short)index*SCENARIO_SQUAD_SIZE);
	}
	for (index= 0; (short)index<*(const long *)(scenario+SCENARIO_SQUAD_GROUPS_OFFSET); index++)
	{
		const byte *scenario_group= *(const byte *const *)(scenario+SCENARIO_SQUAD_GROUPS_OFFSET+4)+
			(short)index*SCENARIO_SQUAD_GROUP_SIZE;
		byte *group= SQUAD_GROUP_GET(datum_new(squad_group_data));

		*(long *)(group+SQUAD_GROUP_NEXT_SIBLING_OFFSET)= NONE;
		*(long *)(group+SQUAD_GROUP_FIRST_CHILD_OFFSET)= NONE;
		*(long *)(group+SQUAD_GROUP_FIRST_SQUAD_OFFSET)= NONE;
		*(short *)(group+0x14)= NONE;
		*(short *)(group+0x24)= NONE;
		*(short *)(group+0x26)= NONE;
		group[0x28]= 0;
		*(short *)(group+0x2A)= NONE;
		*(short *)(group+0x2C)= NONE;
		*(short *)(group+0x34)= 0;
		group[0x23]= 0;
		*(long *)(group+SQUAD_GROUP_PARENT_OFFSET)= *(const short *)(scenario_group+SCENARIO_SQUAD_GROUP_PARENT_OFFSET);
		*(long *)(group+SQUAD_GROUP_NEXT_SIBLING_OFFSET)= NONE;
	}
	squads_unknown_002009d0();
}

#define TAG_GET(tag_index) ((const byte *)(uintptr_t)global_tag_instances[(tag_index)&0xFFFF].address)

/* 0x00200c20: a squad of the scenario gets its squad of the game, with
nothing in it but two bits of its flags (the bits 8 and 9 of the flags of the
squad of the scenario, at 0x20), its group, and its side: the one that the
scenario says (a byte, at 0x24), or if that is zero, the one of the unit of
its character (the scenario has its characters at 0x178; a character has its
unit at 0x10, or what else it is at 0x18, and both have their side at 0xC0),
or if that is zero too, the one of the kind of its character */
void squad_new_from_scenario(
	const byte *scenario_squad)
{
	byte *squad= SQUAD_GET(datum_new(squad_data));
	long index;

	*(short *)(squad+2)= 0;
	*(short *)(squad+2)= (short)((*(const unsigned long *)(scenario_squad+0x20)>>8)&1);
	if (*(const unsigned long *)(scenario_squad+0x20)&0x200)
	{
		squad[2]|= 2;
	}
	else
	{
		squad[2]&= ~2;
	}
	*(long *)(squad+0x68)= NONE;
	*(long *)(squad+0x70)= NONE;
	*(long *)(squad+0x78)= NONE;
	*(short *)(squad+0x7C)= 0;
	squad[0x26]= 0;
	*(short *)(squad+0x2A)= NONE;
	*(short *)(squad+0x62)= NONE;
	*(short *)(squad+0x64)= NONE;
	*(long *)(squad+0x80)= NONE;
	*(long *)(squad+0x2C)= NONE;
	*(long *)(squad+0x5C)= NONE;
	*(short *)(squad+0x24)= 0;
	squad[0x60]= 0;
	*(long *)(squad+SQUAD_GROUP_INDEX_OFFSET)= *(const short *)(scenario_squad+SCENARIO_SQUAD_GROUP_OFFSET);
	*(short *)(squad+SQUAD_NEXT_SQUAD_OFFSET)= NONE;
	*(short *)(squad+0x7E)= NONE;
	for (index= 0; index<5; index++)
	{
		*(long *)(squad+0x84+4*index)= NONE;
	}
	squad[0x76]= scenario_squad[0x24];
	if (!scenario_squad[0x24])
	{
		short character= *(const short *)(scenario_squad+0x36);
		const byte *scenario= (const byte *)global_scenario;

		if (character>=0 && character<*(const long *)(scenario+0x178))
		{
			const byte *reference= *(const byte *const *)(scenario+0x17C)+character*8;

			if (*(const long *)(reference+4)!=NONE)
			{
				const byte *definition= TAG_GET(*(const long *)(reference+4));

				if (*(const long *)(definition+0x10)!=NONE)
				{
					squad[0x76]= TAG_GET(*(const long *)(definition+0x10))[0xC0];
				}
				else if (*(const long *)(definition+0x18)!=NONE)
				{
					squad[0x76]= TAG_GET(*(const long *)(definition+0x18))[0xC0];
				}
				if (!squad[0x76])
				{
					const byte *general= character_definition_get_unknown34(*(const long *)(reference+4));

					if (general)
					{
						short type= *(const short *)(general+4);

						if (type>=0 && type<0x14)
						{
							squad[0x76]= (byte)unknown_0044aee8[type];
						}
					}
				}
			}
		}
	}
}

/* 0x00203330: one of the things of 0x7C bytes that the scenario has at 0x244,
the one that something names with its short at 0x2A; NULL for NONE. What the
thing is is not known: the function is far from the others of this file */
const byte *unknown_get_scenario_unknown244(
	const byte *unknown)
{
	short index= *(const short *)(unknown+0x2A);

	if (index==NONE)
	{
		return NULL;
	}

	return *(const byte *const *)((const byte *)global_scenario+0x244)+index*0x7C;
}
