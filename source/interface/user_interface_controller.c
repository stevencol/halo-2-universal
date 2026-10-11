/*
USER_INTERFACE_CONTROLLER.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What the game keeps of the player of each gamepad. The names are guesses.
*/

#include "user_interface_controller.h"

#include <string.h>

/* ---------- code */

/* the gamepad after one, as the game has it written out where it does not
call user_interface_controller_next: NONE after the last, and after what is
not a gamepad */
static inline __attribute__((always_inline)) long user_interface_controller_after(
	long index)
{
	return (index>=0 && index<MAXIMUM_GAMEPADS-1) ? index+1 : NONE;
}

/* 0x0018f576: what the game keeps of a gamepad's player; NULL for NONE */
struct user_interface_controller *user_interface_controller_get(
	long index)
{
	return index!=NONE ? USER_INTERFACE_CONTROLLER(index) : NULL;
}

/* 0x00190262: the gamepad after one, and the first after NONE; NONE after
the last, and after what is not a gamepad */
long user_interface_controller_next(
	long index)
{
	switch (index)
	{
	case NONE:
		return 0;
	case 0:
		return 1;
	case 1:
		return 2;
	case 2:
		return 3;
	}

	return NONE;
}

/* 0x001900a5: whether a gamepad's player is, in the service of games, the
guest of another */
boolean user_interface_controller_is_guest(
	long index)
{
	return (USER_INTERFACE_CONTROLLER(index)->user.flags&ONLINE_USER_GUEST_MASK)!=0;
}

/* 0x0018f8ce: the gamepad whose player has a profile and one of the things
of the voice; NONE if there is none, or for NONE */
long user_interface_controller_find_voice_index(
	long voice_index)
{
	long index;

	if (voice_index==NONE)
	{
		return NONE;
	}
	for (index= 0; index!=NONE; index= user_interface_controller_after(index))
	{
		const struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);

		if (((controller->flags>>_user_interface_controller_profile_bit)&1) && controller->voice_index==voice_index)
		{
			return index;
		}
	}

	return NONE;
}

/* 0x0018f911: how many gamepads have a player with a profile */
short user_interface_controllers_count_profiles(
	void)
{
	short count= 0;
	long index= 0;

	do
	{
		if ((USER_INTERFACE_CONTROLLER(index)->flags>>_user_interface_controller_profile_bit)&1)
		{
			count++;
		}
		index= user_interface_controller_next(index);
	} while (index!=NONE);

	return count;
}

/* 0x0018f93a: how many gamepads have a player with a profile that is not a
guest */
short user_interface_controllers_count_profiles_not_guests(
	void)
{
	short count= 0;
	long index= 0;

	do
	{
		if (((USER_INTERFACE_CONTROLLER(index)->flags>>_user_interface_controller_profile_bit)&1) &&
			!user_interface_controller_is_guest(index))
		{
			count++;
		}
		index= user_interface_controller_next(index);
	} while (index!=NONE);

	return count;
}

/* 0x0018fa4d: the first gamepad whose player has a profile, or NONE; with 1,
guests are not looked at */
long __attribute__((stdcall)) user_interface_controllers_first_profile(
	long not_guests)
{
	long index= 0;

	do
	{
		boolean guest= FALSE;

		if (not_guests==1)
		{
			guest= user_interface_controller_is_guest(index);
		}
		if (((USER_INTERFACE_CONTROLLER(index)->flags>>_user_interface_controller_profile_bit)&1) && !guest)
		{
			return index;
		}
		index= user_interface_controller_next(index);
	} while (index!=NONE);

	return NONE;
}

/* 0x0018fa94: of the players that are signed in to the service of games, as
the libraries give them, the first that is someone, has a name, signed in
well and whose gamepad does not have a guest; NONE if there is none */
long __attribute__((stdcall)) user_interface_controllers_first_online_user(
	const struct online_user *users)
{
	long index= 0;

	do
	{
		const struct online_user *user= &users[index];

		if ((user->xuid[0]|user->xuid[1])!=0 && user->name[0]!=0 && user->result>=0 &&
			!user_interface_controller_is_guest(index))
		{
			return index;
		}
		index= user_interface_controller_next(index);
	} while (index!=NONE);

	return NONE;
}

/* 0x0018fada: the gamepad whose player has a profile, if there is only one;
NONE if not */
long user_interface_controllers_get_only_profile(
	void)
{
	long only= NONE;
	long count= 0;
	long index= 0;

	do
	{
		if (USER_INTERFACE_CONTROLLER(index)->flags&FLAG(_user_interface_controller_profile_bit))
		{
			count++;
			only= index;
		}
		index= user_interface_controller_next(index);
	} while (index!=NONE);

	return count==1 ? only : NONE;
}

/* 0x0018fb0d: the file of the only profile that the gamepads' players have;
NONE if there is none, or more than one */
long user_interface_controllers_get_only_saved_game(
	void)
{
	long index= user_interface_controllers_get_only_profile();

	if (index!=NONE)
	{
		const struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);

		if (controller->flags&FLAG(_user_interface_controller_profile_bit))
		{
			return controller->saved_game;
		}
	}

	return NONE;
}
