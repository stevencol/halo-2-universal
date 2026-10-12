/*
SWARMS.C

the swarms: characters that go about as one. Worked out from halo2ship.exe; the name of the file is a guess, by
the names that the game gives to its tables of data ("swarm").
*/

#include "cseries/cseries.h"
#include <stdint.h>

#include "math/real_math.h"
#include "memory/data.h"
#include "objects/objects.h"
#include "saved games/game_state.h"

/* ---------- constants */

enum
{
	/* of what unknown_005044c4 points to: five things of 0x50 bytes, each
	with a number that is NONE when there is none and a short at 0x4C */
	SWARM_UNKNOWN_COUNT= 5,
	SWARM_UNKNOWN_SIZE= 0x50,
	SWARM_UNKNOWN_UNKNOWN4C_OFFSET= 0x4C
};

/* ---------- globals */

/* 0x005044c8: the swarms ("swarm": 0x20 of 0x34 bytes). 0x005044c4: the
five things */
struct data_array *swarm_data;
byte *unknown_005044c4;

/* ---------- code */

/* 0x0028d930: the table is made, in the memory of the state of the game
(data_new, which the game has written here), and the five things take their
memory there (game_state_malloc, written here too), each of them with none */
void swarms_initialize(
	void)
{
	byte *unknown;
	long index;

	swarm_data= data_new("swarm", 0x20, 0x34, 0, game_state_allocation);
	unknown= game_state_malloc(SWARM_UNKNOWN_COUNT*SWARM_UNKNOWN_SIZE);
	unknown_005044c4= unknown;
	for (index= 0; index<SWARM_UNKNOWN_COUNT; index++)
	{
		*(short *)(unknown+SWARM_UNKNOWN_UNKNOWN4C_OFFSET)= 0;
		*(long *)unknown= NONE;
		unknown+= SWARM_UNKNOWN_SIZE;
	}
}

/* 0x0028d9d0: when a map goes, each object of the type 12 (the creatures, it
seems) whose number at 0x134 is zero forgets its swarm: what it has at the
offset that its short at 0x13A gives gets NONE for three numbers. And the
table cannot be used */
void swarms_dispose_from_old_map(
	void)
{
	struct object_iterator iterator;
	byte *object;

	iterator.signature= OBJECT_ITERATOR_SIGNATURE;
	iterator.type_mask= 0x1000;
	iterator.header_flags= 0;
	iterator.absolute_index= 0;
	iterator.index= NONE;
	while ((object= object_iterator_next(&iterator))!=NULL)
	{
		if (*(long *)(object+0x134)==0)
		{
			byte *unknown= (byte *)((uintptr_t)object+*(short *)(object+0x13A));

			if (unknown)
			{
				*(long *)(unknown+8)= NONE;
				*(long *)(unknown+4)= NONE;
				*(long *)(unknown+0xC)= NONE;
			}
		}
	}
	swarm_data->valid= FALSE;
}

/* 0x00468788, of sound_manager.c: a point that is all zeros */
extern const real_point3d *unknown_00468788;

/* 0x0028da50: something of 0x80 bytes or more starts with nothing: a short
and a byte that are zero, six numbers and a short that are NONE, and two
points that are the point of zeros. It may be what a creature keeps of its
swarm, by where the function is; that is a guess */
void swarm_unknown_0028da50(
	byte *unknown)
{
	*(short *)unknown= 0;
	*(long *)(unknown+4)= NONE;
	*(long *)(unknown+0x10)= NONE;
	*(long *)(unknown+0xC)= NONE;
	*(long *)(unknown+0x20)= NONE;
	*(long *)(unknown+8)= NONE;
	*(real_point3d *)(unknown+0x14)= *unknown_00468788;
	*(long *)(unknown+0x50)= NONE;
	unknown[0x56]= 0;
	*(short *)(unknown+0x66)= 0;
	*(short *)(unknown+0x64)= NONE;
	*(real_point3d *)(unknown+0x74)= *unknown_00468788;
}

/* 0x004f55d0, of ai.c: what the minds keep; its second byte says whether
there are minds in this map */
extern byte *ai_globals;

/* 0x00290c50: a walk over the swarms starts, if there are minds in the map:
the table, and three numbers that are NONE */
void swarm_iterator_new(
	byte *iterator)
{
	if (ai_globals[1])
	{
		*(struct data_array **)(iterator+4)= swarm_data;
		*(long *)(iterator+0xC)= NONE;
		*(long *)(iterator+8)= NONE;
		*(long *)(iterator+0x10)= NONE;
	}
}
