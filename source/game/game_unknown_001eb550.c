/*
GAME_UNKNOWN_001EB550.C

a part of the game: it keeps 0x1C bytes in the state of the game, three
reals, a vector and a number.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "cseries/cseries.h"
#include "math/real_math.h"
#include "saved games/game_state.h"

#include <string.h>

/* ---------- constants */

enum
{
	GAME_UNKNOWN_001EB550_SIZE= 0x1C,
	/* the bits of the first real and of the third, as the game has them:
	about 4.17 and 0.0011. The second is 1 */
	GAME_UNKNOWN_001EB550_REAL0_BITS= 0x40857AF5,
	GAME_UNKNOWN_001EB550_REAL1_BITS= 0x3F800000,
	GAME_UNKNOWN_001EB550_REAL2_BITS= 0x3A902DE0
};

/* ---------- globals */

/* 0x0051e9c4: what it keeps */
unsigned long *unknown_0051e9c4;

/* ---------- private code */

/* what it keeps is as it starts: the three reals, no vector, and nothing
for the number: the game has it written in each of the two that do it */
static inline __attribute__((always_inline)) void game_unknown_001eb550_reset(
	unsigned long *unknown)
{
	unknown[0]= GAME_UNKNOWN_001EB550_REAL0_BITS;
	unknown[1]= GAME_UNKNOWN_001EB550_REAL1_BITS;
	unknown[2]= GAME_UNKNOWN_001EB550_REAL2_BITS;
	unknown[6]= 0;
	memcpy(&unknown[3], global_zero_vector3d, 3*sizeof(unsigned long));
}

/* ---------- code */

/* 0x001eb550: what it keeps takes its memory of the state of the game, and
is as it starts. The game has game_state_malloc written here */
void game_unknown_001eb550_initialize(
	void)
{
	unsigned long *unknown= game_state_malloc(GAME_UNKNOWN_001EB550_SIZE);

	unknown_0051e9c4= unknown;
	game_unknown_001eb550_reset(unknown);
}

/* 0x001eb5e0: it has no memory */
void game_unknown_001eb550_dispose(
	void)
{
	unknown_0051e9c4= NULL;
}

/* 0x001eb5f0: for a new map, it is as it starts */
void game_unknown_001eb550_initialize_for_new_map(
	void)
{
	game_unknown_001eb550_reset(unknown_0051e9c4);
}
