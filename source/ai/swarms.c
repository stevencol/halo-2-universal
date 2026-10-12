/*
SWARMS.C

the swarms: characters that go about as one. Worked out from halo2ship.exe; the name of the file is a guess, by
the names that the game gives to its tables of data ("swarm").
*/

#include "cseries/cseries.h"
#include "memory/data.h"
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
