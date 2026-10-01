/*
GAME.C

Only the variables of the game that the code decompiled so far uses. The
names are those of Halo 1 where it has them.
*/

#include "game.h"

/* ---------- globals */

/* 0x004e6948 */
struct main_game_globals *main_game_globals;

/* 0x00510c54 */
struct game_time_globals *game_time_globals;

/* 0x00547f6e: asks the game's main loop to start the map again. The name is
a guess, after Halo 1's main_reset_map */
boolean main_reset_map_requested;
