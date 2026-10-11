/*
USER_INTERFACE_CONTROLLER.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What the game keeps of the player of each gamepad. The names are guesses.
*/

#include "user_interface_controller.h"

#include "game/game.h"
#include "game/players.h"
#include "input/input_abstraction.h"
#include "main/global_preferences.h"
#include "networking/game_voice.h"
#include "saved games/saved_game_files.h"

#include <string.h>

/* ---------- globals */

/* 0x004e61cc, of game.c: which gamepads there are */
extern long unknown_004e61cc[MAXIMUM_CONTROLLERS];

/* ---------- constants */

/* 0x00444bdc, 0x00444c04: how fast a gamepad looks around, up and down and
to the sides, for each of the ten numbers that a profile can have */
static const real look_pitch_rates[PLAYER_PROFILE_LOOK_SENSITIVITY_MAXIMUM]=
{
	40.0f, 50.0f, 60.0f, 70.0f, 80.0f, 90.0f, 100.0f, 110.0f, 120.0f, 130.0f
};
static const real look_yaw_rates[PLAYER_PROFILE_LOOK_SENSITIVITY_MAXIMUM]=
{
	80.0f, 100.0f, 120.0f, 140.0f, 160.0f, 180.0f, 200.0f, 220.0f, 240.0f, 260.0f
};

/* ---------- prototypes */

/* 0x00199e2e: something of the interface, after no one is signed in.
0x000abc70: a task of the service of games starts for a player of it; NONE if
it could not. 0x002153dd: a profile is written to its file */
void __attribute__((stdcall)) user_interface_networking_unknown_00199e2e(long unknown);
long __attribute__((stdcall)) online_teams_enumerate_start(const struct online_user *user, long index);
void __attribute__((stdcall)) unknown_002153dd(long index, long saved_game, const struct player_profile *profile,
	long unknown);

/* 0x0023620d: a text of the game, by number. 0x00064cf0: the game of the
network that this console is in, as eight bytes, or NULL. 0x0019a015 and
0x001999d7: two things that the interface says. 0x001492d6: whether
something of the interface is so, of four things */
const byte *network_life_cycle_session_get_unknown4999(void);
boolean user_interface_networking_unknown_0019a015(void);
boolean user_interface_networking_unknown_001999d7(void);
boolean user_interface_unknown_001492d6(long unknown, long unknown2, long index);
void unknown_0023620d(long string, wchar *text);

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

/* whether the players of two gamepads are the same one in the service of
games */
static inline __attribute__((always_inline)) boolean user_interface_controllers_same_online_user(
	long index,
	long other)
{
	const struct online_user *user= &USER_INTERFACE_CONTROLLER(index)->user;
	const struct online_user *other_user= &USER_INTERFACE_CONTROLLER(other)->user;

	return user->xuid[0]==other_user->xuid[0] && user->xuid[1]==other_user->xuid[1];
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

/* 0x00190d0a: the tasks of the service of games of a gamepad's player are
done with */
void user_interface_controller_close_tasks(
	long index)
{
	struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);
	long task;

	for (task= 0; task<NUMBER_OF_USER_INTERFACE_CONTROLLER_TASKS; task++)
	{
		if (controller->tasks[task]!=NONE)
		{
			online_task_close(controller->tasks[task]);
			controller->tasks[task]= NONE;
		}
	}
}

/* 0x00190da5: a gamepad's player is not signed in to the service of games
any more: who it was there is nothing, its tasks are done with, and the name
that is shown of it is the one of its profile */
void __attribute__((stdcall)) user_interface_controller_sign_out(
	long index)
{
	struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);

	controller->flags&= ~FLAG(_user_interface_controller_online_bit);
	controller->unknown204= 0;
	controller->unknown46C= FALSE;
	controller->unknown46D= FALSE;
	memset(&controller->user, 0, sizeof(controller->user));
	memset(controller->unknown4E0, 0, sizeof(controller->unknown4E0));
	memset(controller->unknownB82, 0, sizeof(controller->unknownB82));
	user_interface_controller_close_tasks(index);
	controller->unknownC65= FALSE;
	controller->unknownC5C= 0;
	controller->unknownC64= FALSE;
	memset(controller->unknownC66, 0, sizeof(controller->unknownC66));
	memcpy(controller->name, controller->profile.name, sizeof(controller->name));
}

/* 0x00190d4b: a gamepad has no player: it is signed out, its thing of the
voice is nothing, and it has no profile */
void user_interface_controller_reset(
	long index)
{
	struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);

	user_interface_controller_sign_out(index);
	if (controller->voice_index!=NONE)
	{
		game_voice_unknown_004cd904_clear(controller->voice_index);
		controller->voice_index= NONE;
	}
	user_interface_controller_remove_profile(index);
	controller->voice_index= NONE;
	controller->flags= 0;
	memset(controller->identifier, 0, sizeof(controller->identifier));
	controller->unknown14= NONE;
	controller->unknown1FC= 0;
	controller->unknown200= 0;
	controller->name[0]= 0;
}

/* 0x00190186: the guests of a gamepad's player are no players any more;
nothing if it is a guest itself */
void user_interface_controller_remove_guests(
	long index)
{
	if (!user_interface_controller_is_guest(index))
	{
		long other= 0;

		do
		{
			if (other!=index &&
				((USER_INTERFACE_CONTROLLER(other)->flags>>_user_interface_controller_online_bit)&1) &&
				user_interface_controllers_same_online_user(index, other))
			{
				user_interface_controller_reset(other);
			}
			other= user_interface_controller_next(other);
		} while (other!=NONE);
	}
}

/* 0x00190074: a gamepad's player is signed in to the service of games, or
is not; if not, the name that is shown of it is made again */
void __attribute__((stdcall)) user_interface_controller_set_online(
	long index,
	boolean online)
{
	struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);

	if (online)
	{
		controller->flags|= FLAG(_user_interface_controller_online_bit);
	}
	else
	{
		controller->flags&= ~FLAG(_user_interface_controller_online_bit);
	}
	if (!online)
	{
		user_interface_controller_update_name(index);
	}
}

/* 0x001906b4: everyone is signed out of the service of games, and no
gamepad has a player */
void user_interface_controllers_logoff(
	void)
{
	long index= 0;

	online_logoff();
	do
	{
		user_interface_controller_reset(index);
		index= user_interface_controller_next(index);
	} while (index!=NONE);
	user_interface_networking_unknown_00199e2e(0);
}

/* 0x00190728: the tasks of the service of games of a gamepad's player are
done with, and what they gave is nothing; if it is signed in and is not a
guest, the first starts again */
void __attribute__((stdcall)) user_interface_controller_restart_tasks(
	long index)
{
	struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);
	long task;

	for (task= 0; task<NUMBER_OF_USER_INTERFACE_CONTROLLER_TASKS; task++)
	{
		if (controller->tasks[task]!=NONE)
		{
			online_task_close(controller->tasks[task]);
			controller->tasks[task]= NONE;
		}
	}
	memset(controller->unknown4E0, 0, sizeof(controller->unknown4E0));
	memset(controller->unknownB82, 0, sizeof(controller->unknownB82));
	if (((controller->flags>>_user_interface_controller_online_bit)&1) && !user_interface_controller_is_guest(index))
	{
		controller->tasks[0]= online_teams_enumerate_start(&controller->user, index);
	}
}

/* 0x00190e37: the profiles that changed are written to their files */
void user_interface_controllers_write_changed_profiles(
	void)
{
	long index= 0;

	do
	{
		struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);

		if ((controller->flags&FLAG(_user_interface_controller_profile_bit)) &&
			(controller->flags&FLAG(_user_interface_controller_profile_changed_bit)))
		{
			player_profile_write(controller->saved_game, &controller->profile);
		}
		index= user_interface_controller_next(index);
	} while (index!=NONE);
}

/* 0x00190e71: the profiles that changed are written to their files, in
another way (unknown_002153dd) */
void __attribute__((stdcall)) user_interface_controllers_unknown_00190e71(
	long unknown)
{
	long index= 0;

	do
	{
		struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);

		if ((controller->flags&FLAG(_user_interface_controller_profile_bit)) &&
			(controller->flags&FLAG(_user_interface_controller_profile_changed_bit)))
		{
			unknown_002153dd(index, controller->saved_game, &controller->profile, unknown);
		}
		index= user_interface_controller_next(index);
	} while (index!=NONE);
}

/* 0x0018f5e3: the files that the game knows of are looked at again, and the
file of each profile is what it is now */
void user_interface_controllers_update_saved_games(
	void)
{
	long index= 0;

	saved_game_files_update();
	do
	{
		struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);

		if ((controller->flags&FLAG(_user_interface_controller_profile_bit)) && controller->saved_game!=NONE)
		{
			controller->saved_game= saved_game_file_get_current_index(controller->saved_game);
		}
		index= user_interface_controller_next(index);
	} while (index!=NONE);
	saved_game_files_cache_valid= FALSE;
}

/* 0x0018fd20: the profile of a gamepad's player becomes another and is
written to its file, if it is not the same or had changed already. Nothing
for a player without a profile */
void __attribute__((stdcall)) user_interface_controller_write_profile(
	long index,
	const struct player_profile *profile,
	long saved_game)
{
	struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);

	if (controller->flags&FLAG(_user_interface_controller_profile_bit))
	{
		if ((controller->flags&FLAG(_user_interface_controller_profile_changed_bit)) ||
			memcmp(&controller->profile, profile, sizeof(controller->profile))!=0)
		{
			user_interface_controller_set_profile(index, profile, saved_game);
			if (saved_game!=NONE)
			{
				unknown_002153dd(index, saved_game, profile, 0);
			}
			controller->flags&= ~FLAG(_user_interface_controller_profile_changed_bit);
		}
	}
}

/* 0x0018fcc4: the profile of a gamepad's player becomes another. In a game
that is not of the third kind it is not written to its file: it is only said
to have changed */
void __attribute__((stdcall)) user_interface_controller_update_profile(
	long index,
	const struct player_profile *profile,
	long saved_game)
{
	if (game_in_progress() && main_game_globals->options.game_mode!=_game_mode_3)
	{
		struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);

		if ((controller->flags&FLAG(_user_interface_controller_profile_bit)) &&
			memcmp(&controller->profile, profile, sizeof(controller->profile))!=0)
		{
			user_interface_controller_set_profile(index, profile, saved_game);
			controller->flags|= FLAG(_user_interface_controller_profile_changed_bit);
		}
	}
	else
	{
		user_interface_controller_write_profile(index, profile, saved_game);
	}
}

/* 0x00190001: the text of the profile of a gamepad's player becomes
another; not if the profile has its first bit, or no file */
void __attribute__((stdcall)) user_interface_controller_set_profile_unknown48(
	long index,
	const char *text)
{
	const struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);
	struct player_profile profile;
	long saved_game;

	if (((controller->flags>>_user_interface_controller_profile_bit)&1) &&
		!(controller->profile.flags&FLAG(_player_profile_unknown0_bit)))
	{
		user_interface_controller_get_profile(index, &profile, &saved_game);
		if (saved_game!=NONE)
		{
			strncpy(profile.unknown48, text, sizeof(profile.unknown48));
			profile.unknown48[sizeof(profile.unknown48)-1]= 0;
			user_interface_controller_update_profile(index, &profile, saved_game);
		}
	}
}

/* 0x001902fc: a number of the profile of a gamepad's player becomes
another */
void __attribute__((stdcall)) user_interface_controller_set_profile_unknownE4(
	long index,
	long value)
{
	struct player_profile profile;
	long saved_game;

	if ((USER_INTERFACE_CONTROLLER(index)->flags>>_user_interface_controller_profile_bit)&1)
	{
		user_interface_controller_get_profile(index, &profile, &saved_game);
		profile.unknownE4= value;
		user_interface_controller_update_profile(index, &profile, saved_game);
	}
}

/* 0x001903c2: another number of the profile of a gamepad's player becomes
another, if it is not that already; not NONE, nor without a file */
void __attribute__((stdcall)) user_interface_controller_set_profile_unknown5C(
	long value,
	long index)
{
	struct player_profile profile;
	long saved_game;

	if (value!=NONE)
	{
		user_interface_controller_get_profile(index, &profile, &saved_game);
		if (saved_game!=NONE && profile.unknown5C!=value)
		{
			profile.unknown5C= value;
			user_interface_controller_update_profile(index, &profile, saved_game);
		}
	}
}

/* 0x0019040d: that number becomes another in the profiles of all the
gamepads' players, and in the preferences, which have to be written */
void user_interface_controllers_set_profile_unknown5C(
	long value)
{
	long index;

	for (index= 0; index!=NONE; index= user_interface_controller_after(index))
	{
		user_interface_controller_set_profile_unknown5C(value, index);
	}
	global_preferences_globals.current.data.unknownC= value;
	global_preferences_globals.dirty= TRUE;
}

/* 0x0019043f: one of four numbers of the profile of a gamepad's player
becomes a higher one; not NONE, nor without a file */
void __attribute__((stdcall)) user_interface_controller_raise_profile_unknownEC(
	long value,
	long index,
	long which)
{
	struct player_profile profile;
	long saved_game;

	if (value!=NONE)
	{
		user_interface_controller_get_profile(index, &profile, &saved_game);
		if (saved_game!=NONE && profile.unknownEC[which]<value)
		{
			profile.unknownEC[which]= value;
			user_interface_controller_update_profile(index, &profile, saved_game);
		}
	}
}

/* 0x0019048c: the profile of a gamepad's player gets its second bit; not
without a file */
void __attribute__((stdcall)) user_interface_controller_set_profile_flag1(
	long index)
{
	struct player_profile profile;
	long saved_game;

	user_interface_controller_get_profile(index, &profile, &saved_game);
	if (saved_game!=NONE)
	{
		profile.flags|= FLAG(_player_profile_unknown1_bit);
		user_interface_controller_update_profile(index, &profile, saved_game);
	}
}

/* 0x001904cb: whether the profile of a gamepad's player has its second bit;
not without a file */
boolean user_interface_controller_get_profile_flag1(
	long index)
{
	struct player_profile profile;
	long saved_game;

	user_interface_controller_get_profile(index, &profile, &saved_game);
	if (saved_game!=NONE)
	{
		return (profile.flags>>_player_profile_unknown1_bit)&1;
	}

	return FALSE;
}

/* 0x001904ff: the number of the profile of a gamepad's player that
user_interface_controller_set_profile_unknown5C sets; 1 without a file */
long user_interface_controller_get_profile_unknown5C(
	long index)
{
	struct player_profile profile;
	long saved_game;
	long value= 1;

	user_interface_controller_get_profile(index, &profile, &saved_game);
	if (saved_game!=NONE)
	{
		value= profile.unknown5C;
	}

	return value;
}

/* 0x0019052c: the highest of the four numbers of the profile of a gamepad's
player, and which it is (player_profile_get_unknownEC); NONE and 1 without a
file */
void user_interface_controller_get_profile_unknownEC(
	long index,
	long *value,
	long *which)
{
	struct player_profile profile;
	long saved_game;

	user_interface_controller_get_profile(index, &profile, &saved_game);
	if (saved_game!=NONE)
	{
		player_profile_get_unknownEC(&profile, value, which);
	}
	else
	{
		*value= NONE;
		*which= 1;
	}
}

/* 0x00190565: the highest of those numbers in the profiles of all the
gamepads' players; NONE if none has a profile */
long user_interface_controllers_get_highest_profile_unknownEC(
	void)
{
	long highest= NONE;
	long index;

	for (index= 0; index!=NONE; index= user_interface_controller_after(index))
	{
		if ((USER_INTERFACE_CONTROLLER(index)->flags>>_user_interface_controller_profile_bit)&1)
		{
			long value;
			long which;

			user_interface_controller_get_profile_unknownEC(index, &value, &which);
			if (value>highest)
			{
				highest= value;
			}
		}
	}

	return highest;
}

/* 0x0018fd94: who a gamepad's player is, in twelve bytes, and what the
others are told of it; each if it is asked for. What the voice keeps of it
comes first, and then its name, the looks of its profile, and what else the
game keeps of it */
void __attribute__((stdcall)) user_interface_controller_get_properties(
	long index,
	byte *identifier,
	struct player_properties *properties)
{
	const struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);
	boolean has_voice= controller->voice_index>=0 && controller->voice_index<UNKNOWN_004CD904_COUNT;

	if (identifier)
	{
		memcpy(identifier, controller->identifier, sizeof(controller->identifier));
	}
	if (properties)
	{
		long controller_index;
		unsigned long words;

		player_properties_reset(properties);
		if (!has_voice ||
			!game_voice_unknown_004cd904_get(controller->voice_index, &controller_index, &words, (byte *)properties))
		{
			player_properties_reset(properties);
		}
		ustrnzcpy(properties->name, controller->name, PLAYER_PROFILE_NAME_LENGTH+1);
		properties->appearance= controller->profile.appearance;
		if (user_interface_controller_unknown4E0_valid(controller))
		{
			memcpy(properties->unknown70, controller->unknown4E0, sizeof(properties->unknown70));
			memcpy(properties->unknown50, controller->unknown4E0+sizeof(properties->unknown70),
				sizeof(properties->unknown50));
		}
		else
		{
			memset(properties->unknown70, 0, sizeof(properties->unknown70));
			memset(properties->unknown50, 0, sizeof(properties->unknown50));
		}
		properties->unknown7D= (byte)controller->unknown200;
		properties->unknown80= controller->unknown46C;
		properties->unknown81= (byte)controller->unknown204;
		if (controller->unknown1FC==NONE)
		{
			properties->unknown7C= NONE;
		}
		else if (controller->unknown1FC<0)
		{
			properties->unknown7C= 0;
		}
		else
		{
			properties->unknown7C= controller->unknown1FC>PLAYER_PROPERTIES_UNKNOWN7C_MAXIMUM ?
				PLAYER_PROPERTIES_UNKNOWN7C_MAXIMUM : (char)controller->unknown1FC;
		}
	}
}

/* 0x0018fe9e: what the voice keeps of a gamepad's player is what the game
has of it now; nothing if the voice keeps nothing of it */
void user_interface_controller_update_voice(
	long index)
{
	const struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);

	if (controller->voice_index!=NONE)
	{
		struct player_properties properties;
		byte identifier[sizeof(controller->identifier)];
		unsigned long words;

		user_interface_controller_get_properties(index, identifier, &properties);
		words= game_voice_controller_player_words(index);
		game_voice_unknown_004cd904_set(controller->voice_index, index, words, (const byte *)&properties);
	}
}

/* 0x0018fee9: a gamepad's player is someone of the service of games: its
name is made again, and the voice keeps something of it, as of who it is
there */
void __attribute__((stdcall)) user_interface_controller_set_online_user(
	const struct online_user *user,
	long index)
{
	struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);

	controller->user= *user;
	user_interface_controller_update_name(index);
	if (controller->voice_index==NONE)
	{
		controller->voice_index= game_voice_unknown_004cd904_new(controller->identifier);
	}
	memcpy(controller->identifier, &controller->user, sizeof(controller->identifier));
	game_voice_unknown_004cd904_set_identifier(controller->voice_index, controller->identifier);
}

/* 0x00190eb3: the name that is shown of a gamepad's player is made again:
the one of the service of games if it is signed in there, made with a text
of the game and its number if it is a guest; if not, the one of its profile;
and nothing without one. The voice is told */
void __attribute__((stdcall)) user_interface_controller_update_name(
	long index)
{
	struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);

	if ((controller->flags>>_user_interface_controller_online_bit)&1)
	{
		if (controller->user.flags&ONLINE_USER_GUEST_MASK)
		{
			wchar format[USER_INTERFACE_CONTROLLER_GUEST_NAME_FORMAT_LENGTH];
			long guest= (char)controller->user.flags&ONLINE_USER_GUEST_MASK;

			format[0]= 0;
			unknown_0023620d(USER_INTERFACE_CONTROLLER_GUEST_NAME_STRING, format);
			usnzprintf(controller->name, PLAYER_PROFILE_NAME_LENGTH+1, format, (long)(short)guest,
				controller->user.name);
		}
		else
		{
			usnzprintf(controller->name, PLAYER_PROFILE_NAME_LENGTH+1, (const wchar *)u"%hs", controller->user.name);
		}
	}
	else if ((controller->flags>>_user_interface_controller_profile_bit)&1)
	{
		ustrnzcpy(controller->name, controller->profile.name, PLAYER_PROFILE_NAME_LENGTH+1);
	}
	else
	{
		controller->name[0]= 0;
	}
	user_interface_controller_update_voice(index);
}

/* 0x0018fb34: a gamepad's player has a profile, and its file. One that had
none is someone of this console, and the voice keeps something of it. What
the game kept of the file of the profile that it had is let go of, and it
keeps the new file; the gamepad is played with as the profile says, the name
is made again, and the voice is told what the profile says of it */
void __attribute__((stdcall)) user_interface_controller_set_profile(
	long index,
	const struct player_profile *profile,
	long saved_game)
{
	struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);
	boolean had_profile= (controller->flags&FLAG(_user_interface_controller_profile_bit))!=0;
	boolean new_file= !(had_profile && controller->saved_game==saved_game);

	if (!had_profile)
	{
		player_identifier_from_controller(controller->identifier, index);
		controller->voice_index= game_voice_unknown_004cd904_new(controller->identifier);
		controller->saved_game= NONE;
	}
	if (new_file && controller->saved_game!=NONE)
	{
		cached_saved_game_file_delete(controller->saved_game);
	}
	controller->profile= *profile;
	controller->flags|= FLAG(_user_interface_controller_profile_bit);
	controller->saved_game= saved_game;
	user_interface_controller_apply_input_preferences(index);
	user_interface_controller_update_name(index);
	if (new_file && controller->saved_game!=NONE)
	{
		cached_saved_game_file_count++;
		cached_saved_game_files[cached_saved_game_file_count-1].file= controller->saved_game;
	}
	game_voice_set_controller_mask(profile->voice_mask, index);
	game_voice_set_controller_setting(index, profile->voice_setting);
}

/* 0x0019034d: what a gamepad's player is at, for the others of the service
of games, becomes something; if that is not what it was, they have to be
told. Nothing for one that is not signed in */
void __attribute__((stdcall)) user_interface_controller_set_online_state(
	long index,
	long kind,
	long unknown8,
	long unknown4,
	short time,
	short second_time)
{
	struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);

	if ((controller->flags>>_user_interface_controller_online_bit)&1)
	{
		struct online_state state;
		unsigned long encoded;

		memset(&state, 0, sizeof(state));
		state.kind= kind;
		state.time= time;
		state.second_time= second_time;
		state.unknown8= unknown8;
		state.unknown4= unknown4;
		online_state_encode(&encoded, &state);
		if (controller->unknownC5C!=encoded)
		{
			controller->unknownC5C= encoded;
			controller->unknownC65= TRUE;
		}
	}
}

/* 0x00190f9b: the others of the service of games are told what a gamepad's
player is at, if it changed: the game of the network that it is in, and some
bits. Not for a guest, nor if no one is signed in */
void __attribute__((stdcall)) user_interface_controller_update_online_state(
	long index)
{
	struct user_interface_controller *controller;
	const byte *session;
	unsigned long flags;

	if (user_interface_controller_is_guest(index) || !online_signed_in())
	{
		return;
	}
	controller= USER_INTERFACE_CONTROLLER(index);
	session= network_life_cycle_session_get_unknown4999();
	flags= controller->profile.unknown150==0;
	if (!session)
	{
		if (controller->unknownC64)
		{
			memset(controller->unknownC66, 0, sizeof(controller->unknownC66));
			controller->unknownC64= FALSE;
			controller->unknownC65= TRUE;
		}
	}
	else
	{
		if (!controller->unknownC64 || memcmp(controller->unknownC66, session, sizeof(controller->unknownC66))!=0)
		{
			controller->unknownC64= TRUE;
			memcpy(controller->unknownC66, session, sizeof(controller->unknownC66));
			controller->unknownC65= TRUE;
		}
		if (controller->unknownC5C&ONLINE_STATE_UNKNOWN_MASK)
		{
			flags|= FLAG(_online_notification_unknown1_bit);
		}
		if (user_interface_networking_unknown_0019a015())
		{
			flags|= FLAG(_online_notification_unknown4_bit);
		}
	}
	if (game_voice_controller_can_talk(index))
	{
		flags|= FLAG(_online_notification_voice_bit);
	}
	if (controller->unknownC60!=flags)
	{
		controller->unknownC60= flags;
		controller->unknownC65= TRUE;
	}
	if (controller->unknownC65)
	{
		online_notification_set_state(controller->unknownC64 ? (const unsigned long *)controller->unknownC66 : NULL,
			index, (byte *)&controller->unknownC5C, controller->unknownC60);
		controller->unknownC65= FALSE;
	}
}

/* 0x001906da: whether the player of a gamepad may use the voice: if
user_interface_networking_unknown_001999d7 says so; or if it is signed in to the service of games, is
not a guest, and the service does not say that it may not */
boolean user_interface_controller_voice_allowed(
	long index)
{
	struct online_user users[MAXIMUM_ONLINE_USERS];
	const struct online_user *user;
	unsigned long flags;

	if (user_interface_networking_unknown_001999d7())
	{
		return TRUE;
	}
	online_get_logon_users(users);
	user= &users[index];
	if ((user->xuid[0]|user->xuid[1])==0)
	{
		return FALSE;
	}
	memcpy(&flags, &user->flags, sizeof(flags));

	return !(flags&ONLINE_USER_GUEST_MASK) && !((flags>>_online_user_no_voice_bit)&1);
}

/* 0x0018f973: whether any of four things of the interface is so */
boolean user_interface_unknown_0018f973(
	void)
{
	long index;

	for (index= 0; index<USER_INTERFACE_UNKNOWN_0018F973_COUNT; index++)
	{
		if (user_interface_unknown_001492d6(4, 0, USER_INTERFACE_UNKNOWN_0018F973_FIRST+index))
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* 0x00190c34: a gamepad is played with as its player's profile says: how
fast it looks around, which button does what, and two bits */
void user_interface_controller_apply_input_preferences(
	long index)
{
	const struct player_profile *profile= &USER_INTERFACE_CONTROLLER(index)->profile;
	struct input_abstraction_preferences preferences;
	long sensitivity;
	word unknown101;

	input_abstraction_preferences_set_defaults((byte *)&preferences);
	sensitivity= profile->unknown102;
	if (sensitivity<PLAYER_PROFILE_LOOK_SENSITIVITY_MINIMUM)
	{
		sensitivity= PLAYER_PROFILE_LOOK_SENSITIVITY_MINIMUM;
	}
	else if (sensitivity>PLAYER_PROFILE_LOOK_SENSITIVITY_MAXIMUM)
	{
		sensitivity= PLAYER_PROFILE_LOOK_SENSITIVITY_MAXIMUM;
	}
	preferences.unknown4= look_pitch_rates[sensitivity-1];
	preferences.unknown0= look_yaw_rates[sensitivity-1];
	unknown101= profile->unknown101>PLAYER_PROFILE_UNKNOWN101_MAXIMUM ? PLAYER_PROFILE_UNKNOWN101_MAXIMUM :
		profile->unknown101;
	memcpy(preferences.unknown8+16, &unknown101, sizeof(unknown101));
	switch (profile->unknown100)
	{
	case 1:
		preferences.unknown8[6]= 7;
		preferences.unknown8[7]= 6;
		break;
	case 2:
		preferences.unknown8[4]= 6;
		preferences.unknown8[6]= 1;
		break;
	case 3:
		preferences.unknown8[4]= 15;
		preferences.unknown8[11]= 1;
		break;
	}
	preferences.unknown8[18]= profile->unknownFC&1;
	preferences.unknown8[19]= ((unsigned long)profile->unknownFC>>2)&1;
	input_abstraction_globals.preferences[index]= preferences;
}

/* 0x0018f8b8: whether a gamepad is there (unknown_004e61cc, of game.c) */
boolean __attribute__((stdcall)) user_interface_controller_gamepad_present(
	short index)
{
	return unknown_004e61cc[index]!=0;
}
