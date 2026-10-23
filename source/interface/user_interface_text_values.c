/*
USER_INTERFACE_TEXT_VALUES.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

Texts that the screens of the interface ask for: each function is given the
number of a text and the place to write it to, and writes the name or the
description of a game of the list, the name of the profile or of the player
of the service of a gamepad, or a text of the interface. The name of the
file is a guess, and so are those of its functions.
*/

#include "cseries/cseries.h"
#include "interface/user_interface_controller.h"
#include "networking/network_life_cycle.h"
#include "networking/online_friends.h"
#include "networking/online_tasks.h"
#include "saved games/player_profile.h"
#include "text/unicode.h"

/* ---------- constants */

enum
{
	/* how long the name and the description of a game of the list are */
	USER_INTERFACE_TEXT_HOPPER_NAME_LENGTH= 16,
	USER_INTERFACE_TEXT_HOPPER_DESCRIPTION_LENGTH= 128,

	/* the numbers of the texts of the names of the profiles of the four
	gamepads, and of the names of their players of the service */
	USER_INTERFACE_TEXT_PROFILE_NAME0= 0xE40F,
	USER_INTERFACE_TEXT_PROFILE_NAME1= 0xE410,
	USER_INTERFACE_TEXT_PROFILE_NAME2= 0xE411,
	USER_INTERFACE_TEXT_PROFILE_NAME3= 0xE412,
	USER_INTERFACE_TEXT_ONLINE_NAME0= 0xE429,
	USER_INTERFACE_TEXT_ONLINE_NAME1= 0xE42A,
	USER_INTERFACE_TEXT_ONLINE_NAME2= 0xE42B,

	/* how many characters of the name of a player of the service are made
	wide */
	USER_INTERFACE_TEXT_ONLINE_NAME_LENGTH= 0x100,

	/* a text of the interface that 0x0022d67c asks for */
	USER_INTERFACE_TEXT_UNKNOWN_060000B8= 0x060000B8
};

/* ---------- prototypes */

/* 0x001944c0, 0x00194540 (network_hoppers.c) */
boolean __attribute__((stdcall)) network_hoppers_get_name(long index, wchar *name);
boolean __attribute__((stdcall)) network_hoppers_get_description(long index, wchar *description);

/* 0x0023620d: writes a text of the interface, by its number */
void unknown_0023620d(long text_index, wchar *text);

/* ---------- globals */

/* 0x0055c2e0: the game of the list of the last name or description */
long unknown_0055c2e0;

/* 0x0054e5e0: a text, of two-byte letters (to the next thing that is
known) */
wchar unknown_0054e5e0[0xF6];

/* ---------- code */

/* 0x0022d52e: the name of the game of the list of the network game (or of
the last one, if it has none); nothing if there is none */
void __attribute__((stdcall)) user_interface_text_hopper_name(
	long text_index,
	wchar *text)
{
	wchar name[USER_INTERFACE_TEXT_HOPPER_NAME_LENGTH];
	long index= network_life_cycle_session_get_unknown49C8();

	(void)text_index;
	if (index!=NONE)
	{
		unknown_0055c2e0= index;
	}
	if (network_hoppers_get_name(unknown_0055c2e0, name))
	{
		network_motd_text_copy(text, name);
	}
}

/* 0x0022d566: the same with its description */
void __attribute__((stdcall)) user_interface_text_hopper_description(
	long text_index,
	wchar *text)
{
	wchar description[USER_INTERFACE_TEXT_HOPPER_DESCRIPTION_LENGTH];
	long hopper_index= network_life_cycle_session_get_unknown49C8();

	(void)text_index;
	if (hopper_index!=NONE)
	{
		unknown_0055c2e0= hopper_index;
	}
	if (network_hoppers_get_description(unknown_0055c2e0, description))
	{
		network_motd_text_copy(text, description);
	}
}

/* 0x0022d67c: the text 0x060000B8 of the interface */
void __attribute__((stdcall)) user_interface_text_unknown_0022d67c(
	long text_index,
	wchar *text)
{
	(void)text_index;
	unknown_0023620d(USER_INTERFACE_TEXT_UNKNOWN_060000B8, text);
}

/* 0x0022d68f: the name of the profile of the gamepad of the text; the text
of 0x0054e5e0 for a text of no gamepad */
void __attribute__((stdcall)) user_interface_text_profile_name(
	long text_index,
	wchar *text)
{
	struct player_profile profile;
	long saved_game;
	long index;

	switch (text_index)
	{
	case USER_INTERFACE_TEXT_PROFILE_NAME0:
		index= 0;
		break;
	case USER_INTERFACE_TEXT_PROFILE_NAME1:
		index= 1;
		break;
	case USER_INTERFACE_TEXT_PROFILE_NAME2:
		index= 2;
		break;
	case USER_INTERFACE_TEXT_PROFILE_NAME3:
		index= 3;
		break;
	default:
		network_motd_text_copy(text, unknown_0054e5e0);
		return;
	}
	user_interface_controller_get_profile(index, &profile, &saved_game);
	network_motd_text_copy(text, profile.name);
}

/* 0x0022d6ee: the name of the player of the service of the gamepad of the
text, made wide; the fourth gamepad for any text that is not of the first
three */
void __attribute__((stdcall)) user_interface_text_online_name(
	long text_index,
	wchar *text)
{
	struct online_user user;
	long controller_index;

	switch (text_index)
	{
	case USER_INTERFACE_TEXT_ONLINE_NAME0:
		controller_index= 0;
		break;
	case USER_INTERFACE_TEXT_ONLINE_NAME1:
		controller_index= 1;
		break;
	case USER_INTERFACE_TEXT_ONLINE_NAME2:
		controller_index= 2;
		break;
	default:
		controller_index= 3;
		break;
	}
	user_interface_controller_get_online_user(controller_index, &user);
	ascii_string_to_wchar_string(user.name, text, USER_INTERFACE_TEXT_ONLINE_NAME_LENGTH);
}
