/*
USER_INTERFACE_CONTROLLER.H

What the game keeps of the player of each gamepad: its profile, and who it is
in the service of games. Worked out from halo2ship.exe; the names are guesses.
*/

#ifndef __USER_INTERFACE_CONTROLLER_H
#define __USER_INTERFACE_CONTROLLER_H

#include "cseries/cseries.h"
#include "input/input.h"
#include "networking/online_tasks.h"
#include "saved games/player_profile.h"
#include "text/unicode.h"

/* ---------- constants */

enum
{
	/* the bits of a gamepad's player: it has a profile; it is signed in to
	the service of games; its profile changed and is not written yet */
	_user_interface_controller_profile_bit= 4,
	_user_interface_controller_online_bit= 5,
	_user_interface_controller_profile_changed_bit= 6,
	/* the bits of a player of the service of games that say that it is the
	guest of another, and which */
	ONLINE_USER_GUEST_MASK= 0x3,
	/* how many tasks of the service of games a gamepad's player has */
	NUMBER_OF_USER_INTERFACE_CONTROLLER_TASKS= 2
};

/* ---------- structures */

struct user_interface_controller
{
	unsigned long flags;
	/* which of the things of the voice (unknown_004cd904) is its own, or
	NONE */
	long voice_index;
	/* the first twelve bytes of who it is in the service of games */
	byte identifier[12];
	long unknown14;
	struct player_profile profile;
	/* the file of its profile, or NONE */
	long saved_game;
	long unknown1FC;
	long unknown200;
	long unknown204;
	byte unknown208[0x264];
	boolean unknown46C;
	boolean unknown46D;
	byte unknown46E[2];
	/* who it is in the service of games */
	struct online_user user;
	byte unknown4E0[0x6A2];
	byte unknownB82[0x92];
	/* its tasks of the service of games, or NONE */
	long tasks[NUMBER_OF_USER_INTERFACE_CONTROLLER_TASKS];
	/* the name of its profile */
	wchar name[PLAYER_PROFILE_NAME_LENGTH+1];
	unsigned long unknownC5C;
	byte unknownC60[4];
	boolean unknownC64;
	boolean unknownC65;
	byte unknownC66[8];
	byte unknownC6E[2];
};

typedef char verify_user_interface_controller_size[
	sizeof(struct user_interface_controller) == UNKNOWN_0054E8E0_SIZE ? 1 : -1];
typedef char verify_user_interface_controller_user_offset[
	offsetof(struct user_interface_controller, user) == 0x470 ? 1 : -1];
typedef char verify_user_interface_controller_tasks_offset[
	offsetof(struct user_interface_controller, tasks) == 0xC14 ? 1 : -1];
typedef char verify_user_interface_controller_unknownC66_offset[
	offsetof(struct user_interface_controller, unknownC66) == 0xC66 ? 1 : -1];

/* ---------- macros */

/* the table is input_xbox.c's, as bytes (unknown_0054e8e0) */
#define USER_INTERFACE_CONTROLLER(index) ((struct user_interface_controller *)unknown_0054e8e0[index])

/* ---------- prototypes/USER_INTERFACE_CONTROLLER.C */

struct user_interface_controller *user_interface_controller_get(long index);
long user_interface_controller_next(long index);
boolean user_interface_controller_is_guest(long index);
long user_interface_controller_find_voice_index(long voice_index);
short user_interface_controllers_count_profiles(void);
short user_interface_controllers_count_profiles_not_guests(void);
long __attribute__((stdcall)) user_interface_controllers_first_profile(long not_guests);
long __attribute__((stdcall)) user_interface_controllers_first_online_user(const struct online_user *users);
long user_interface_controllers_get_only_profile(void);
long user_interface_controllers_get_only_saved_game(void);

#endif
