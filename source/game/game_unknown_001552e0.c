/*
GAME_UNKNOWN_001552E0.C

a part of the game: it keeps 0x50C bytes of its own, and a byte and 0x78 bytes in
the state of the game.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "cseries/cseries.h"
#include "saved games/game_state.h"

#include <string.h>

/* ---------- globals */

/* 0x004e8c38: what it keeps of its own. 0x004e8c34: the byte. 0x00510c6c,
of game.c: the 0x78 bytes */
byte unknown_004e8c38[0x50C];
byte *unknown_004e8c34;
struct unknown_00510c6c;
extern struct unknown_00510c6c *unknown_00510c6c;

/* ---------- code */

/* 0x001552e0: what it keeps of its own is nothing; the byte takes four
bytes of the state of the game and is clear, and the 0x78 bytes take theirs.
The game has game_state_malloc written here, twice */
void game_unknown_001552e0_initialize(
	void)
{
	byte *unknown;

	memset(unknown_004e8c38, 0, sizeof(unknown_004e8c38));
	unknown= game_state_malloc(4);
	*unknown= 0;
	unknown_004e8c34= unknown;
	unknown_00510c6c= game_state_malloc(0x78);
}

/* 0x00155490: when a map goes, what it keeps of its own is nothing, and the
byte is clear */
void game_unknown_001552e0_dispose_from_old_map(
	void)
{
	memset(unknown_004e8c38, 0, sizeof(unknown_004e8c38));
	*unknown_004e8c34= 0;
}
