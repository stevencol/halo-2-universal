/*
SQUADS.C

the squads of the characters, and the groups of squads. Worked out from halo2ship.exe; the name of the file is a guess, by
the names that the game gives to its tables of data ("squad", "squad group").
*/

#include "cseries/cseries.h"
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

/* 0x00200c20: a squad of the scenario gets its squad of the game */
void unknown_00200c20(const byte *scenario_squad);

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
them; each squad of the scenario gets its squad (unknown_00200c20) and each
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
		unknown_00200c20(*(const byte *const *)(scenario+SCENARIO_SQUADS_OFFSET+4)+(short)index*SCENARIO_SQUAD_SIZE);
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
