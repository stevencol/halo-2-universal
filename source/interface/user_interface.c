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

/* 0x00148262: a part of the interface, by two numbers; NULL if there is
none */
const byte *unknown_00148262(long unknown, long unknown2);

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

/* 0x00149278: whether one of the things of a part of the interface is there
and is at 7, 8 or 0xF0 */
boolean user_interface_screen_unknown_00149278(
	const byte *screen)
{
	if (screen)
	{
		long state= *(const long *)(screen+USER_INTERFACE_SCREEN_STATE_OFFSET);

		return (state>=_user_interface_screen_state7 && state<=_user_interface_screen_state8) ||
			state==_user_interface_screen_state_f0;
	}

	return FALSE;
}

/* 0x001492d6: whether either of the two things of a part of the interface
is a given one, and is at what the function before this one looks for */
boolean user_interface_unknown_001492d6(
	long unknown,
	long unknown2,
	long identifier)
{
	const byte *channel= unknown_00148262(unknown, unknown2);
	boolean found= FALSE;

	if (channel)
	{
		long index;

		for (index= 0; index<NUMBER_OF_USER_INTERFACE_CHANNEL_SCREENS; index++)
		{
			const byte *screen= *(const byte *const *)(channel+USER_INTERFACE_CHANNEL_SCREENS_OFFSET+
				index*sizeof(const byte *));

			if (user_interface_screen_unknown_00149278(screen) &&
				*(const long *)(screen+USER_INTERFACE_SCREEN_IDENTIFIER_OFFSET)==identifier)
			{
				found= TRUE;
			}
		}
	}

	return found;
}

/* 0x00149318: whether the interface shows any of three things; the change
of game has to wait if so. All three are looked for */
boolean user_interface_unknown_00149318(
	void)
{
	static const long screens[NUMBER_OF_USER_INTERFACE_UNKNOWN_00149318_SCREENS]= { 0x3A, 0x23, 0x38 };
	boolean found= FALSE;
	long index;

	for (index= 0; index<NUMBER_OF_USER_INTERFACE_UNKNOWN_00149318_SCREENS; index++)
	{
		if (user_interface_unknown_001492d6(4, 1, screens[index]))
		{
			found= TRUE;
		}
	}

	return found;
}

/* 0x0054e7fc: a time of the interface */
unsigned long unknown_0054e7fc;
extern unsigned long unknown_0054d5b8;

/* 0x0014a250: whether user_interface_unknown_0014a224 says no and 200 have
passed since that time */
boolean user_interface_unknown_0014a250(
	void)
{
	return !user_interface_unknown_0014a224() && unknown_0054d5b8-unknown_0054e7fc>=200;
}
