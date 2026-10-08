/*
GAME_VOICE.H

What the game does with the voice of the players of the network game, and
the memory that the console's libraries of the sound ask for. The name of
the file is a guess, and so are the names of its functions.
*/

#ifndef __GAME_VOICE_H
#define __GAME_VOICE_H

#include "cseries/cseries.h"
#include "memory/memory_pool.h"
#include "networking/voice.h"

/* ---------- constants */

enum
{
	/* what is given to an allocation of no size */
	GAME_VOICE_DEFAULT_SIZE= 0x200,

	/* the pool of the voice, in a game that has it */
	GAME_VOICE_POOL_SIZE= 0x78000,
	/* the pool of the voice and of the menu, in a campaign in a session */
	GAME_VOICE_MENU_POOL_SIZE= 0x61800,

	/* what the allocation of the pool is told with: as that of the files of
	the players */
	GAME_VOICE_ALLOCATION_UNKNOWN= 2
};

/* how much of the voice a game has, by how its memory is shared out */
enum
{
	/* a campaign that is not in a session of the network game */
	_game_voice_none,
	/* a campaign in a session */
	_game_voice_session,
	/* anything but a campaign: the only one with the pool */
	_game_voice_full
};

enum
{
	/* the ways that the voice of a player can be changed: not at all, and
	so that it is not known */
	NUMBER_OF_GAME_VOICE_MASKS= 2,

	/* the players of a game: each has a bit in the numbers of 16 bits */
	MAXIMUM_GAME_VOICE_PLAYERS= 16
};

/* what a controller's player wants of the voice (0x004c99a8). What the
others are is not known */
enum
{
	_game_voice_controller_setting1= 1,
	_game_voice_controller_setting3= 3
};

/* as main_game.c calls those that take memory when a map is loaded */
#define GAME_VOICE_CALLBACK __attribute__((stdcall))

/* ---------- structures */

/* what the memory of the pool is asked with: the functions, and a number
for them */
struct game_voice_allocation
{
	struct allocation allocation;
	long unknown;
};

#if defined(__i386__)
typedef char verify_game_voice_allocation_size[
	sizeof(struct game_voice_allocation) == 0x8 ? 1 : -1];
#endif

/* ---------- prototypes/GAME_VOICE.C */

void *game_voice_allocate(unsigned long size, unsigned long attributes);
void __attribute__((stdcall)) game_voice_free(void *address, unsigned long attributes);
void GAME_VOICE_CALLBACK game_voice_pool_new(long configuration);
void game_voice_pool_delete(void);
void game_voice_menu_pool_new(void);
void game_voice_menu_pool_delete(void);
void game_voice_start(void);
void game_voice_stop(void);
boolean game_voice_has_remote_talker(long identifier);
void game_voice_set_controller_mode(long controller_index, long mode);
long game_voice_get_controller_mode(long controller_index);
void game_voice_set_controller_mask(long mask, long controller_index);
boolean game_voice_unknown_00053850(long player_index, long other_player_index);
void game_voice_remote_talker_data(long identifier, const void *data, long size);
void game_voice_play_message(long controller_index, const long *message, long size);
void game_voice_stop_message(long controller_index);
boolean game_voice_message_busy(long controller_index);
void game_voice_record_message(long controller_index, void *buffer, long *size, long unknown,
	unsigned long buffer_size, long *length);
void game_voice_stop_message_with_headset(long controller_index);

/* ---------- globals */

extern struct memory_pool *game_voice_pool;
extern struct memory_pool *game_voice_menu_pool;
extern boolean game_voice_use_menu_pool;
extern long game_voice_amount;
extern long game_voice_kind;
extern boolean game_voice_enabled;
extern long unknown_004c9888;
extern word unknown_004c9968[MAXIMUM_GAME_VOICE_PLAYERS];
extern long game_voice_controller_settings[MAXIMUM_VOICE_CONTROLLERS];
extern const real game_voice_masks[NUMBER_OF_GAME_VOICE_MASKS][NUMBER_OF_VOICE_MASK_VALUES];
extern word unknown_004c9966;
extern struct allocation *unknown_0046dd54;
extern struct game_voice_allocation game_voice_allocation;

#endif
