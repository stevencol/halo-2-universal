/*
USER_INTERFACE_CONTROLLER.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What the game keeps of the player of each gamepad. The names are guesses.
*/

#include "user_interface_controller.h"

#include "saved games/saved_game_files.h"

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

/* whether what starts something of a gamepad's player (unknown4E0) is
anything: its first eight bytes, which are most likely who someone is in the
service of games */
static inline __attribute__((always_inline)) boolean user_interface_controller_unknown4E0_valid(
	const struct user_interface_controller *controller)
{
	unsigned long identifier[2];

	memcpy(identifier, controller->unknown4E0, sizeof(identifier));

	return (identifier[0]|identifier[1])!=0;
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

/* 0x0018fc08: a gamepad's player has no profile any more: what the game
kept of its file is let go of, and the profile is nothing */
void user_interface_controller_remove_profile(
	long index)
{
	struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);

	if (controller->flags&FLAG(_user_interface_controller_profile_bit))
	{
		if (controller->saved_game!=NONE)
		{
			cached_saved_game_file_delete(controller->saved_game);
		}
		memset(&controller->profile, 0, sizeof(controller->profile));
		controller->flags&= ~FLAG(_user_interface_controller_profile_bit);
		controller->saved_game= NONE;
	}
}

/* 0x0018fc44: the profile of a gamepad's player, and its file; nothing and
NONE if it has none */
void user_interface_controller_get_profile(
	long index,
	struct player_profile *profile,
	long *saved_game)
{
	const struct user_interface_controller *controller= user_interface_controller_get(index);

	if (controller && (controller->flags&FLAG(_user_interface_controller_profile_bit)))
	{
		*profile= controller->profile;
		*saved_game= controller->saved_game;
	}
	else
	{
		memset(profile, 0, sizeof(*profile));
		*saved_game= NONE;
	}
}

/* 0x0018fc75: whether a file is the one of the profile of a gamepad's
player; not if the file has the bit of 21 */
boolean __attribute__((stdcall)) user_interface_controllers_have_saved_game(
	long saved_game)
{
	boolean found= FALSE;
	long index= 0;

	while (index!=NONE)
	{
		const struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);

		if (controller->flags&FLAG(_user_interface_controller_profile_bit))
		{
			found= !(((unsigned long)controller->saved_game>>_saved_game_unknown21_bit)&1) &&
				controller->saved_game==saved_game;
		}
		index= user_interface_controller_after(index);
		if (found)
		{
			break;
		}
	}

	return found;
}

/* 0x0018fd7c: something of a gamepad's player */
long user_interface_controller_get_unknown1FC(
	long index)
{
	return USER_INTERFACE_CONTROLLER(index)->unknown1FC;
}

/* 0x0018ff47: who a gamepad's player is in the service of games */
void user_interface_controller_get_online_user(
	long index,
	struct online_user *user)
{
	*user= USER_INTERFACE_CONTROLLER(index)->user;
}

/* 0x0018ff64: whether a gamepad's player has a task of the service of
games */
boolean user_interface_controller_has_tasks(
	long index)
{
	const struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);

	return controller->tasks[0]!=NONE || controller->tasks[1]!=NONE;
}

/* 0x0018ff88: something of a gamepad's player, of 0x6A2 bytes; whether it is
anything. Nothing if it is not */
boolean user_interface_controller_get_unknown4E0(
	long index,
	byte *unknown)
{
	const struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);

	if (user_interface_controller_unknown4E0_valid(controller))
	{
		memcpy(unknown, controller->unknown4E0, sizeof(controller->unknown4E0));

		return TRUE;
	}
	memset(unknown, 0, sizeof(controller->unknown4E0));

	return FALSE;
}

/* 0x0018ffc3: something else of a gamepad's player, of 0x92 bytes, which is
only good when the thing before it is */
boolean user_interface_controller_get_unknownB82(
	long index,
	byte *unknown)
{
	const struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);

	if (user_interface_controller_unknown4E0_valid(controller))
	{
		memcpy(unknown, controller->unknownB82, sizeof(controller->unknownB82));

		return TRUE;
	}
	memset(unknown, 0, sizeof(controller->unknownB82));

	return FALSE;
}

/* 0x001900be: how many gamepads have a player that is signed in to the
service of games and is not a guest */
short user_interface_controllers_count_online_not_guests(
	void)
{
	short count= 0;
	long index;

	for (index= 0; index!=NONE; index= user_interface_controller_after(index))
	{
		if (((USER_INTERFACE_CONTROLLER(index)->flags>>_user_interface_controller_online_bit)&1) &&
			!user_interface_controller_is_guest(index))
		{
			count++;
		}
	}

	return count;
}

/* 0x001900ff: how many guests the player of a gamepad has: the other
gamepads whose player is signed in as the same one. None if it is a guest
itself */
short __attribute__((stdcall)) user_interface_controller_count_guests(
	long index)
{
	short count= 0;

	if (!user_interface_controller_is_guest(index))
	{
		long other;

		for (other= 0; other!=NONE; other= user_interface_controller_after(other))
		{
			const struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(other);
			const struct online_user *user= &USER_INTERFACE_CONTROLLER(index)->user;

			if (other!=index && ((controller->flags>>_user_interface_controller_online_bit)&1) &&
				user->xuid[0]==controller->user.xuid[0] && user->xuid[1]==controller->user.xuid[1])
			{
				count++;
			}
		}
	}

	return count;
}

/* 0x001901fc: the gamepads whose player has a profile, as bits */
short user_interface_controllers_get_profile_mask(
	void)
{
	short mask= 0;
	long index= 0;

	do
	{
		if ((USER_INTERFACE_CONTROLLER(index)->flags>>_user_interface_controller_profile_bit)&1)
		{
			mask|= 1<<index;
		}
		index= user_interface_controller_next(index);
	} while (index!=NONE);

	return mask;
}

/* 0x0019022f: the gamepads whose player is signed in to the service of
games, as bits */
short user_interface_controllers_get_online_mask(
	void)
{
	short mask= 0;
	long index= 0;

	do
	{
		if ((USER_INTERFACE_CONTROLLER(index)->flags>>_user_interface_controller_online_bit)&1)
		{
			mask|= 1<<index;
		}
		index= user_interface_controller_next(index);
	} while (index!=NONE);

	return mask;
}

/* 0x0019028d: whether the player of any gamepad is signed in to the service
of games */
boolean user_interface_controllers_any_online(
	void)
{
	long index;

	for (index= 0; index!=NONE; index= user_interface_controller_after(index))
	{
		if ((USER_INTERFACE_CONTROLLER(index)->flags>>_user_interface_controller_online_bit)&1)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* 0x001902c1: the name of the profile of a gamepad's player; NULL if it has
none */
wchar *user_interface_controller_get_name(
	long index)
{
	struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);

	return ((controller->flags>>_user_interface_controller_profile_bit)&1) ? controller->name : NULL;
}

/* 0x001902de: something of the profile of a gamepad's player; NONE if it has
none */
long user_interface_controller_get_profile_unknownE4(
	long index)
{
	const struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);

	return ((controller->flags>>_user_interface_controller_profile_bit)&1) ? controller->profile.unknownE4 : NONE;
}

/* 0x001907bf: whether something is 1, or a number is 250 or more */
boolean __attribute__((stdcall)) user_interface_unknown_001907bf(
	char unknown,
	word number)
{
	return unknown==1 || number>=USER_INTERFACE_UNKNOWN_001907BF_MINIMUM;
}
