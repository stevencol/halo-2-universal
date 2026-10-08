/*
VOICE_MUTE_LIST.H

The players that the player of each gamepad wants not to hear, as the
console's library of the network keeps them. The lists themselves are in
game_voice.c. The name of the file is a guess, and so are the names of its
functions.
*/

#ifndef __VOICE_MUTE_LIST_H
#define __VOICE_MUTE_LIST_H

#include "cseries/cseries.h"
#include "networking/game_voice.h"

/* ---------- structures */

/* the twelve bytes that a player is known by */
struct voice_mute_list_player
{
	unsigned long words[GAME_VOICE_PLAYER_IDENTIFIER_SIZE/sizeof(unsigned long)];
};

/* a function that takes its arguments off the stack */
#define VOICE_MUTE_LIST_CALLBACK __attribute__((stdcall))

/* ---------- prototypes/VOICE_MUTE_LIST.C */

void voice_mute_list_reset(long controller_index);
void voice_mute_list_add(long controller_index, const struct voice_mute_list_player *player);
void voice_mute_list_remove(long controller_index, const struct voice_mute_list_player *player);
boolean VOICE_MUTE_LIST_CALLBACK voice_mute_list_has_player(long controller_index,
	const struct voice_mute_list_player *player);

/* ---------- globals */

extern long unknown_004c99c0;
extern long voice_mute_list_tasks[MAXIMUM_VOICE_CONTROLLERS];

#endif
