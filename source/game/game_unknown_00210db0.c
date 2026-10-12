/*
GAME_UNKNOWN_00210DB0.C

a part of the game: sixteen numbers of the state of the game, that are NONE when a
structure bsp comes.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "cseries/cseries.h"

#include <string.h>

#include "memory/data.h"
#include "objects/objects.h"
#include "saved games/game_state.h"
#include "scenario/scenario.h"

/* ---------- constants */

enum
{
	GAME_UNKNOWN_00210DB0_COUNT= 16
};

/* ---------- globals */

/* 0x004f93a4: the sixteen numbers */
long *unknown_004f93a4;

/* ---------- prototypes */

void game_unknown_00210db0_unknown_00210f00(void);
void game_unknown_00210db0_unknown_00210e20(void);
void game_unknown_00210db0_unknown_00210e80(void);

/* ---------- code */

/* 0x00210db0: the numbers take their memory in the state of the game
(game_state_malloc, which the game has written here) */
void game_unknown_00210db0_initialize(
	void)
{
	unknown_004f93a4= game_state_malloc(GAME_UNKNOWN_00210DB0_COUNT*sizeof(long));
}

/* 0x00210df0: the fifth of the functions of the part: for a new structure
bsp the numbers are NONE, and game_unknown_00210db0_unknown_00210f00 and game_unknown_00210db0_unknown_00210e20 are done */
void game_unknown_00210db0_initialize_for_new_structure_bsp(
	void)
{
	long index;

	for (index= 0; index<GAME_UNKNOWN_00210DB0_COUNT; index++)
	{
		unknown_004f93a4[index]= NONE;
	}
	game_unknown_00210db0_unknown_00210f00();
	game_unknown_00210db0_unknown_00210e20();
}

/* 0x00210e10: the sixth: when the structure bsp goes, game_unknown_00210db0_unknown_00210e80 is
done */
void game_unknown_00210db0_dispose_from_old_structure_bsp(
	void)
{
	game_unknown_00210db0_unknown_00210e80();
}

/* 0x00210e80: each object of the type 7 has NONE for its short at 0x1DA and
zero for the one at 0x1DC. The object is taken from the table by the index
that the walk leaves, not from what the walk returns */
void game_unknown_00210db0_unknown_00210e80(
	void)
{
	struct object_iterator iterator;

	iterator.signature= OBJECT_ITERATOR_SIGNATURE;
	iterator.type_mask= 0x80;
	iterator.header_flags= 0;
	iterator.absolute_index= 0;
	iterator.index= NONE;
	while (object_iterator_next(&iterator)!=NULL)
	{
		byte *object= (byte *)((struct object_header_datum *)object_header_data->data)[iterator.index&0xFFFF].datum;

		*(short *)(object+0x1DA)= NONE;
		*(short *)(object+0x1DC)= 0;
	}
}

/* 0x00210e20: the sixteen numbers are 512 bits, one for each thing of the
block that the structure bsp has at 0x5C (so many, of 0x24 bytes each): the
bit of each thing that has the bit 3 of its byte at 0x18 is cleared. What
the block is of is not known */
void game_unknown_00210db0_unknown_00210e20(
	void)
{
	const byte *structure_bsp= global_structure_bsp;
	long index;

	for (index= 0; index<*(const long *)(structure_bsp+0x5C); index++)
	{
		const byte *element= *(const byte *const *)(structure_bsp+0x60)+index*0x24;

		if (element[0x18]&8)
		{
			BIT_VECTOR_AND_FLAG((unsigned long *)unknown_004f93a4, index);
		}
	}
}

/* 0x00210f00: the objects of the type 7 forget their two shorts
(game_unknown_00210db0_unknown_00210e80), and then take them from a list that
the structure bsp points to at 0x220: so many things of 0xC bytes, each with
what an object is known by (a number, a short, and two bytes, which the
object has from 0xA4; the short counts only when the second byte is zero) and
the two shorts. The first object that is the one of a thing takes them */
void game_unknown_00210db0_unknown_00210f00(
	void)
{
	const byte *list;
	long index;

	game_unknown_00210db0_unknown_00210e80();
	list= *(const byte *const *)((const byte *)global_structure_bsp+0x220);
	for (index= 0; index<*(const long *)list; index++)
	{
		const byte *element= *(const byte *const *)(list+4)+index*0xC;
		struct object_iterator iterator;

		iterator.signature= OBJECT_ITERATOR_SIGNATURE;
		iterator.type_mask= 0x80;
		iterator.header_flags= 0;
		iterator.absolute_index= 0;
		iterator.index= NONE;
		while (object_iterator_next(&iterator)!=NULL)
		{
			byte *object= (byte *)((struct object_header_datum *)object_header_data->data)[iterator.index&0xFFFF].datum;
			boolean same= element[6]==object[0xAA] && element[7]==object[0xAB] &&
				*(const long *)element==*(const long *)(object+0xA4);

			if (same && element[7]==0)
			{
				same= *(const short *)(element+4)==*(const short *)(object+0xA8);
			}
			if (same)
			{
				*(short *)(object+0x1DA)= *(const short *)(element+8);
				*(short *)(object+0x1DC)= *(const short *)(element+0xA);
				break;
			}
		}
	}
}
