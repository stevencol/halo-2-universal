/*
COMMAND_SCRIPTS.C

the scripts that tell the characters what to do. Worked out from halo2ship.exe; the name of the file is a guess, by
the names that the game gives to its tables of data ("command scripts",
"joint command scripts").
*/

#include "cseries/cseries.h"
#include "memory/data.h"
#include "saved games/game_state.h"

/* ---------- globals */

/* 0x00502408: the scripts ("command scripts": 0x28 of 0xD4 bytes).
0x00502404: those of several at once ("joint command scripts": 0xA of 0x8C
bytes) */
struct data_array *command_script_data;
struct data_array *joint_command_script_data;

/* ---------- code */

/* 0x00257d00: the two tables are made, in the memory of the state of the
game (data_new, which the game has written here) */
void command_scripts_initialize(
	void)
{
	command_script_data= data_new("command scripts", 0x28, 0xD4, 0, game_state_allocation);
	joint_command_script_data= data_new("joint command scripts", 0xA, 0x8C, 0, game_state_allocation);
}
