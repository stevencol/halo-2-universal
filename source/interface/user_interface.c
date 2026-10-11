/*
USER_INTERFACE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What the rest of the game asks of the interface. The names are guesses.
*/

#include "user_interface.h"

#include "game/game.h"
#include "interface/user_interface_controller.h"

/* ---------- prototypes */

/* 0x00148e6d: whether something of the interface is so of a gamepad.
0x00147f4f: what the interface is at, as a number */
boolean __attribute__((stdcall)) unknown_00148e6d(long controller_index);
long unknown_00147f4f(void);

/* ---------- code */

/* 0x00147da5: whether something of the interface is so of the gamepad whose
player has one of the things of the voice; not if no gamepad has it */
boolean user_interface_unknown_00147da5(
	long voice_index)
{
	long controller_index= user_interface_controller_find_voice_index(voice_index);

	return controller_index!=NONE ? unknown_00148e6d(controller_index) : FALSE;
}

/* 0x0014a224: whether, in a game of the third kind, the interface is at 6
or at 9 */
boolean user_interface_unknown_0014a224(
	void)
{
	if (game_in_progress() && main_game_globals->options.game_mode==_game_mode_3)
	{
		long at= unknown_00147f4f();

		return at==6 || at==9;
	}

	return FALSE;
}
