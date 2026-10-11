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
	NUMBER_OF_USER_INTERFACE_CONTROLLER_TASKS= 2,
	/* a file of a profile with this bit is not looked for among the ones
	that the gamepads' players have */
	_saved_game_unknown21_bit= 21,
	/* from what number on something is so, in user_interface_unknown_001907bf */
	USER_INTERFACE_UNKNOWN_001907BF_MINIMUM= 250
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
void user_interface_controller_remove_profile(long index);
void user_interface_controller_get_profile(long index, struct player_profile *profile, long *saved_game);
boolean __attribute__((stdcall)) user_interface_controllers_have_saved_game(long saved_game);
long user_interface_controller_get_unknown1FC(long index);
void user_interface_controller_get_online_user(long index, struct online_user *user);
boolean user_interface_controller_has_tasks(long index);
boolean user_interface_controller_get_unknown4E0(long index, byte *unknown);
boolean user_interface_controller_get_unknownB82(long index, byte *unknown);
short user_interface_controllers_count_online_not_guests(void);
short __attribute__((stdcall)) user_interface_controller_count_guests(long index);
short user_interface_controllers_get_profile_mask(void);
short user_interface_controllers_get_online_mask(void);
boolean user_interface_controllers_any_online(void);
wchar *user_interface_controller_get_name(long index);
long user_interface_controller_get_profile_unknownE4(long index);
boolean __attribute__((stdcall)) user_interface_unknown_001907bf(char unknown, word number);
void user_interface_controller_close_tasks(long index);
void __attribute__((stdcall)) user_interface_controller_sign_out(long index);
void user_interface_controller_reset(long index);
void user_interface_controller_remove_guests(long index);
void __attribute__((stdcall)) user_interface_controller_set_online(long index, boolean online);
void user_interface_controllers_logoff(void);
void __attribute__((stdcall)) user_interface_controller_restart_tasks(long index);
void user_interface_controllers_write_changed_profiles(void);
void __attribute__((stdcall)) user_interface_controllers_unknown_00190e71(long unknown);
void user_interface_controllers_update_saved_games(void);

#endif
