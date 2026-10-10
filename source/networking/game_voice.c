/*
GAME_VOICE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What the game does with the voice of the players of the network game
(voice.c). It starts with the memory that the console's libraries of the
sound ask for, which comes from one of two pools of the game: the file was
sound_memory.c until milestone 269. The name of the file is a guess, and so
are the names of its functions.
*/

#include "game_voice.h"

#include "game/game.h"
#include "input/input.h"
#include "main/main.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "main/main_game.h"
#include "networking/simulation_world.h"
#include "networking/network_life_cycle.h"
#include "networking/network_session.h"
#include "saved games/saved_game_files.h"

/* ---------- prototypes of functions not decompiled yet */


/* 0x00332f22, of the console's library of the voice: the engine does its
work of each frame */
long unknown_00332f22(void *engine);

/* 0x00068250: something of the network game */
boolean unknown_00068250(void);

/* 0x000696d0 is simulation_world_player_unknown25, of simulation_world.c */


/* 0x0005c6d0 is network_session_host_set_chosen_peer, of network_session.c */


/* 0x001906da: something of the player of a gamepad */
boolean unknown_001906da(long gamepad_index);

/* 0x000649c0: something of the network game; NULL if there is none */
byte *unknown_000649c0(void);

/* of the C library: 0x003212d6 and 0x003212c5 */
unsigned int _control87(unsigned int value, unsigned int mask);
unsigned int _clearfp(void);

/* ---------- globals */

/* 0x004c995c, 0x004c9960 and 0x004c9964: the pools, and whether the second
one is used */
struct memory_pool *game_voice_pool;
struct memory_pool *game_voice_menu_pool;
boolean game_voice_use_menu_pool;

/* 0x004c9890, 0x004c9894: how much of the voice the game has, and what the
voice is told that it is */
long game_voice_amount;
long game_voice_kind;

/* 0x004c99b8: the game has to do with the voice at all */
boolean game_voice_enabled;

/* 0x004c9966: a bit for each player that has a voice: one of this console,
if it can talk; one of another, if the library has given it a voice */
word game_voice_talkers;

/* 0x004c9888: 0 when what a player of another console said is not given to
the library */
long unknown_004c9888;

/* 0x004c988c: which of two things gives the session of the network game */
long game_voice_session_source;

/* 0x004c9878: the console of the session that is chosen for the voice
(game_voice_update_chosen_peer); NONE if none. With no voice it counts as
0 */
long unknown_004c9878;

/* 0x005259b8, 0x00527104, 0x00527108: three things that the voice hands
what the players say to */
byte unknown_005259b8[UNKNOWN_005259B8_SIZE];
byte unknown_00527104[UNKNOWN_00527104_SIZE];
byte unknown_00527108[UNKNOWN_00527108_SIZE];

/* 0x004ce120, 0x004ce124: the least that two numbers of a console have to
be for it to be chosen */
long unknown_004ce120;
long unknown_004ce124;

/* 0x004c99d4, 0x004cd854: the players that the player of each gamepad wants
not to hear, and how many: NONE if they are not known */
byte game_voice_muted_players[MAXIMUM_VOICE_CONTROLLERS][MAXIMUM_GAME_VOICE_MUTED_PLAYERS][GAME_VOICE_MUTED_PLAYER_SIZE];
long game_voice_muted_player_counts[MAXIMUM_VOICE_CONTROLLERS];

/* 0x004c9898, 0x004c989c: the players that the session had the last time: a
bit for each, and the twelve bytes that each is known by (zeros for one that
there is not) */
long game_voice_session_players;
byte game_voice_session_player_identifiers[GAME_VOICE_SESSION_PLAYER_IDENTIFIERS_SIZE];

/* 0x004cd904: what the game keeps of each profile. How many there are is a
guess */
byte unknown_004cd904[UNKNOWN_004CD904_COUNT][UNKNOWN_004CD904_SIZE];

/* 0x004e9ae8, 0x0055e4d0: something else of the game, and a table that a
number of it picks from. The size of the table is a guess */
byte *unknown_004e9ae8;
long unknown_0055e4d0[UNKNOWN_0055E4D0_COUNT];

/* 0x004c987c: how the voice goes between the consoles: with 1 or 2, by the
console chosen for the voice when it can; with 2, to one console alone if it
cannot (voice_route_peers) */
long unknown_004c987c;

/* 0x004c9880, 0x004c9884: what game_voice_unknown_00054cc0 gives when
unknown_004c9888 is 1, and when it is 2 or 3: to how many consoles
something that a player said can go from this one */
long unknown_004c9880;
long unknown_004c9884;

/* 0x004c9988: what the game knows of the voice of each player */
word game_voice_player_flag_words[MAXIMUM_GAME_VOICE_PLAYERS];

/* 0x00525a00: the queues of what the players said (voice_routing.h) */
byte unknown_00525a00[UNKNOWN_00525A00_SIZE];

/* 0x004c9968: for each player, a bit for each of the others that it wants
not to hear */
word game_voice_player_mutes[MAXIMUM_GAME_VOICE_PLAYERS];

/* 0x004c99a8: what each controller's player wants of the voice */
long game_voice_controller_settings[MAXIMUM_VOICE_CONTROLLERS];

/* 0x0043fe48: what the library changes the voice of a player with */
const real game_voice_masks[NUMBER_OF_GAME_VOICE_MASKS][NUMBER_OF_VOICE_MASK_VALUES]=
{
	{-1.0f, -1.0f, -1.0f, -1.0f},
	{0.95f, 0.5f, 0.35f, -1.0f}
};

/* 0x0046dd54: what the memory of the pool of the voice and of the menu is
asked of; the heap's, if it is NULL. The game has it point to an allocation
whose memory the texture cache lends (0x0047d8e4: see main_game.c) */
struct allocation *unknown_0046dd54;

/* 0x00476fbc: what the pool's memory is asked of: the same that gives the
files of the players theirs */
struct game_voice_allocation game_voice_allocation=
{
	{&saved_game_files_allocation_functions},
	GAME_VOICE_ALLOCATION_UNKNOWN
};

/* ---------- code */

/* whether there is a voice for the game to do something with: nearly every
function of the file starts with this */
static inline __attribute__((always_inline)) boolean game_voice_active(
	void)
{
	return game_voice_enabled && voice_globals.initialized;
}

boolean game_voice_get_session(byte **session);

/* the session that the voice goes with, if it says who is in it; NULL if
not. The game has this in each function that looks at the session */
static inline __attribute__((always_inline)) byte *game_voice_get_valid_session(
	void)
{
	byte *session= NULL;

	if (game_voice_get_session(&session) && *(const long *)(session+GAME_VOICE_SESSION_UNKNOWN741C_OFFSET) &&
		*(const long *)(session+GAME_VOICE_SESSION_MEMBERSHIP_OFFSET)!=NONE)
	{
		return session;
	}

	return NULL;
}

/* unknown_004c9878, as the functions of the file read it */
static inline __attribute__((always_inline)) long game_voice_get_unknown_004c9878(
	void)
{
	return game_voice_active() ? unknown_004c9878 : 0;
}

/* the pool that the memory comes from now */
static inline __attribute__((always_inline)) struct memory_pool *game_voice_get_pool(
	void)
{
	return game_voice_use_menu_pool ? game_voice_menu_pool : game_voice_pool;
}

/* 0x00053290: a block of the pool; NULL if there is no room. An allocation
of no size gets 0x200 bytes. What is asked for of it is not looked at */
void *game_voice_allocate(
	unsigned long size,
	unsigned long attributes)
{
	void *block= NULL;

	(void)attributes;

	if (!size)
	{
		size= GAME_VOICE_DEFAULT_SIZE;
	}
	memory_pool_block_allocate(game_voice_get_pool(), &block, (long)size, 0, 0);

	return block;
}

/* 0x000532e0: gives back a block that game_voice_allocate gave */
void __attribute__((stdcall)) game_voice_free(
	void *address,
	unsigned long attributes)
{
	(void)attributes;

	if (address)
	{
		memory_pool_block_free(game_voice_get_pool(), &address);
	}
}

/* 0x00053310: when a map is loaded, by how the memory is shared out: says
how much of the voice the game has and, if it is all of it, makes the pool
and starts the voice, if it is not started. A block of the pool is not moved,
and a new one goes where there is room among the others. With 0x000533e0, in
the table of those that take memory then (0x00453c00).

The game does not see whether there was memory for the pool before it writes
in it. It has voice_initialize in the function itself */
void GAME_VOICE_CALLBACK game_voice_pool_new(
	long configuration)
{
	long voice;
	long size= 0;
	long kind= _voice_kind_none;

	if (configuration<=_main_game_memory_default)
	{
		voice= _game_voice_full;
	}
	else
	{
		voice= configuration<=_main_game_memory_campaign_session_coop ? _game_voice_session :
			_game_voice_none;
	}

	switch (voice)
	{
	case _game_voice_session:
		kind= _voice_kind_session;
		break;
	case _game_voice_full:
		size= GAME_VOICE_POOL_SIZE;
		kind= _voice_kind_full;
		break;
	}

	game_voice_amount= voice;
	game_voice_kind= kind;
	if (voice==_game_voice_full)
	{
		game_voice_pool= memory_pool_new(&game_voice_allocation.allocation, size, "voice pool");
		game_voice_pool->no_references= TRUE;
		game_voice_pool->fill_gaps= TRUE;
		game_voice_pool->unknown3E= TRUE;
		voice_initialize(&voice_globals, kind);
	}
}

/* 0x000533e0: stops the voice and gives the pool back, if there is one */
void game_voice_pool_delete(
	void)
{
	if (game_voice_pool)
	{
		voice_dispose(&voice_globals);
		memory_pool_delete(game_voice_pool);
		game_voice_pool= NULL;
	}
}

/* 0x00053420: in a campaign in a session, makes the pool of the voice and of
the menu, and starts the voice: the memory that the libraries ask for comes
from this pool from now on. The game has memory_pool_new in the function
itself */
void game_voice_menu_pool_new(
	void)
{
	if (game_voice_amount==_game_voice_session)
	{
		struct memory_pool *pool;

		game_voice_use_menu_pool= TRUE;
		pool= memory_pool_new(unknown_0046dd54, GAME_VOICE_MENU_POOL_SIZE, "voice y menu pool");
		game_voice_menu_pool= pool;
		if (pool)
		{
			pool->no_references= TRUE;
			game_voice_menu_pool->fill_gaps= TRUE;
			game_voice_menu_pool->unknown3E= TRUE;
			game_voice_start();
		}
	}
}

/* 0x000534a0: stops the voice and gives the pool of the voice and of the
menu back. The game has game_voice_stop and memory_pool_delete in the
function itself */
void game_voice_menu_pool_delete(
	void)
{
	if (game_voice_amount==_game_voice_session)
	{
		if (game_voice_menu_pool)
		{
			game_voice_stop();
			memory_pool_delete(game_voice_menu_pool);
			game_voice_menu_pool= NULL;
		}
		game_voice_use_menu_pool= FALSE;
	}
}

/* 0x00053510: starts the voice, if it is not started: a full one, or that
of a session if the game is a campaign */
void game_voice_start(
	void)
{
	if (game_voice_enabled &&
		(game_voice_kind==_voice_kind_full ||
		(main_game_globals->options.game_mode==_game_mode_campaign && game_voice_kind==_voice_kind_session)) &&
		!voice_globals.initialized)
	{
		voice_initialize(&voice_globals, game_voice_kind);
	}
}

/* 0x00053550: stops the voice, if it is started */
void game_voice_stop(
	void)
{
	if (game_voice_active())
	{
		voice_dispose(&voice_globals);
		game_voice_talkers= 0;
	}
}

/* 0x00053720: whether a player of another console has a voice */
boolean game_voice_has_remote_talker(
	long identifier)
{
	boolean result= FALSE;

	if (game_voice_active())
	{
		result= voice_has_remote_talker(&voice_globals, identifier);
	}

	return result;
}

/* 0x000537b0: the mode of the voice of a controller. The game has a
function of voice.c in this one, which it has no copy of */
void game_voice_set_controller_mode(
	long controller_index,
	long mode)
{
	if (game_voice_active())
	{
		if (unknown_00332da8(voice_globals.engine, controller_index, voice_modes[mode])>=0)
		{
			voice_globals.controller_modes[controller_index]= mode;
		}
	}
}

/* 0x000537f0: the mode of the voice of a controller; 0 with no voice */
long game_voice_get_controller_mode(
	long controller_index)
{
	return voice_globals.initialized ? voice_globals.controller_modes[controller_index] : 0;
}

/* 0x00053810: the way that the voice of a controller's player is changed:
one of game_voice_masks */
void game_voice_set_controller_mask(
	long mask,
	long controller_index)
{
	if (game_voice_active())
	{
		voice_set_controller_mask(&voice_globals, controller_index, game_voice_masks[mask]);
	}
}

/* 0x00053850: whether a player wants not to hear another */
boolean game_voice_player_has_muted(
	long player_index,
	long other_player_index)
{
	boolean result= FALSE;

	if (game_voice_active())
	{
		result= ((1<<other_player_index)&game_voice_player_mutes[player_index])!=0;
	}

	return result;
}

/* 0x00053890: what a player of another console said, as it came from the
network, goes to the library, if the player has a voice */
void game_voice_remote_talker_data(
	long identifier,
	const void *data,
	long size)
{
	if (game_voice_active() && unknown_004c9888!=0)
	{
		if (game_voice_has_remote_talker(identifier))
		{
			game_voice_library_begin();
			voice_remote_talker_data(data, size, &voice_globals, identifier);
			game_voice_library_end();
		}
	}
}

/* 0x000538e0: a controller plays a message, unless its player wants none.
It is played without a headset if the player wants that, or if the
controller cannot talk */
void game_voice_play_message(
	long controller_index,
	const long *message,
	long size)
{
	if (game_voice_active())
	{
		long setting= game_voice_controller_settings[controller_index];
		boolean without_headset= FALSE;

		if (setting==_game_voice_controller_setting1 || !game_voice_controller_can_talk(controller_index))
		{
			without_headset= TRUE;
		}
		if (setting!=_game_voice_controller_setting3 && message && size>0)
		{
			voice_play_message(message, without_headset, size, controller_index, &voice_globals);
		}
	}
}

/* 0x00053940: a controller stops the message that it records or plays. The
game has a function of voice.c in this one, which it has no copy of */
void game_voice_stop_message(
	long controller_index)
{
	if (game_voice_active())
	{
		if (voice_globals.controller_modes[controller_index]==_voice_mode_unknown3)
		{
			unknown_00332e93(voice_globals.engine, controller_index);
		}
		voice_globals.controller_message_busy[controller_index]= FALSE;
	}
}

/* 0x00053970: whether a controller records a message or plays one */
boolean game_voice_message_busy(
	long controller_index)
{
	boolean result= FALSE;

	if (game_voice_active())
	{
		result= voice_globals.controller_message_busy[controller_index];
	}

	return result;
}

/* 0x000539a0: a controller records a message: see voice_record_message */
void game_voice_record_message(
	long controller_index,
	void *buffer,
	long *size,
	long unknown,
	unsigned long buffer_size,
	long *length)
{
	if (game_voice_active())
	{
		voice_record_message(buffer_size, controller_index, &voice_globals, unknown, buffer, size, length);
	}
}

/* 0x000539e0: as game_voice_stop_message, but the library is only told if
the controller has a headset */
void game_voice_stop_message_with_headset(
	long controller_index)
{
	if (game_voice_active())
	{
		if (voice_globals.controller_modes[controller_index]==_voice_mode_unknown3 &&
			voice_globals.controller_has_headset[controller_index])
		{
			unknown_00332e93(voice_globals.engine, controller_index);
		}
		voice_globals.controller_message_busy[controller_index]= FALSE;
	}
}

/* 0x00053b00: writes what there is to send to a console of what the players
said; how much it is, and 0 with no voice (voice_queues_build_packet) */
long game_voice_build_packet(
	long peer,
	byte *packet,
	long size)
{
	long result= 0;

	if (game_voice_active() && unknown_004c9888!=0)
	{
		result= voice_queues_build_packet(packet, unknown_00525a00, peer, size);
	}

	return result;
}

/* 0x00053b40: whether a player can talk and wants a voice */
boolean game_voice_player_can_be_heard(
	long player_index)
{
	boolean result= FALSE;

	if (game_voice_active())
	{
		if ((game_voice_player_flags(player_index)&(1u<<_game_voice_player_can_talk_bit)) &&
			!(game_voice_active() &&
			((game_voice_player_flags(player_index)>>_game_voice_player_voice_off_bit)&1)))
		{
			result= TRUE;
		}
	}

	return result;
}

/* 0x00053b90 */
boolean game_voice_player_voice_off(
	long player_index)
{
	boolean result= FALSE;

	if (game_voice_active())
	{
		result= (game_voice_player_flags(player_index)>>_game_voice_player_voice_off_bit)&1;
	}

	return result;
}

/* 0x00053bb0 */
boolean game_voice_player_not_allowed(
	long player_index)
{
	boolean result= FALSE;

	if (game_voice_active())
	{
		result= (game_voice_player_flags(player_index)>>_game_voice_player_not_allowed_bit)&1;
	}

	return result;
}

/* 0x00053be0: what the game knows of the voice of a player: 0 if the
session does not have the player */
unsigned long game_voice_player_flags(
	long player_index)
{
	if (game_voice_active())
	{
		unsigned long players= 0;
		const byte *membership= game_voice_get_membership();

		if (membership)
		{
			players= *(const unsigned long *)(membership+GAME_VOICE_MEMBERSHIP_PLAYERS_OFFSET);
		}
		if (players&(1u<<player_index))
		{
			return game_voice_player_flag_words[player_index];
		}
	}

	return 0;
}

/* 0x00053c30: the session of the network game that the voice goes with;
whether there is one */
boolean game_voice_get_session(
	byte **session)
{
	if (game_voice_active() && unknown_004c9888!=0)
	{
		switch (game_voice_session_source)
		{
		case _game_voice_session_source1:
			return network_life_cycle_get_session(session);
		case _game_voice_session_source2:
			return network_life_cycle_get_other_session(session);
		}
	}

	return FALSE;
}

/* 0x00053d90 */
boolean game_voice_unknown_00053d90(
	void)
{
	const byte *unknown= unknown_000649c0();

	return unknown && *(const long *)(unknown+UNKNOWN_000649C0_STATE_OFFSET)==_unknown_000649c0_state7;
}

/* 0x00053db0: whether a message is a sound file: see voice_play_message */
boolean game_voice_message_is_sound_file(
	const long *message,
	long size)
{
	return size>VOICE_MESSAGE_MINIMUM_SIZE && message[0]==VOICE_MESSAGE_FILE_TAG &&
		message[2]==VOICE_MESSAGE_FORM_TAG && message[3]==VOICE_MESSAGE_FORMAT_TAG;
}

/* 0x00053de0: what the session says of who is in it; NULL if there is no
session or it says nothing */
byte *game_voice_get_membership(
	void)
{
	if (game_voice_active())
	{
		byte *session= NULL;

		if (game_voice_get_session(&session))
		{
			if (!*(const long *)(session+GAME_VOICE_SESSION_UNKNOWN741C_OFFSET))
			{
				return NULL;
			}
			if (*(const long *)(session+GAME_VOICE_SESSION_MEMBERSHIP_OFFSET)==NONE)
			{
				return NULL;
			}

			return session+GAME_VOICE_SESSION_MEMBERSHIP_OFFSET;
		}
	}

	return NULL;
}

/* 0x00053f50: before the library of the voice works: the reals as it wants
them, and no error in one stops the program */
void game_voice_library_begin(
	void)
{
	_control87(GAME_VOICE_FLOATING_POINT_CONTROL, GAME_VOICE_FLOATING_POINT_LIBRARY_MASK);
	__builtin_ia32_ldmxcsr(__builtin_ia32_stmxcsr()|GAME_VOICE_MXCSR_EXCEPTION_MASKS);
}

/* 0x00053f80: after the library of the voice works: the errors that it left
noted are forgotten, and the reals are as the game has them */
void game_voice_library_end(
	void)
{
	__builtin_ia32_ldmxcsr(__builtin_ia32_stmxcsr()&~(unsigned int)GAME_VOICE_MXCSR_EXCEPTION_FLAGS);
	_clearfp();
	_control87(GAME_VOICE_FLOATING_POINT_CONTROL, GAME_VOICE_FLOATING_POINT_MASK);
}

/* 0x000547e0: says which of the two places the session comes from: none, if
network_life_cycle_get_current_session gives nothing */
void game_voice_update_session_source(
	void)
{
	byte *unknown;

	game_voice_session_source= 0;
	if (network_life_cycle_get_current_session(&unknown))
	{
		game_voice_session_source=
			(*(const long *)(unknown+UNKNOWN_000596D0_UNKNOWN14_OFFSET)==_unknown_000596d0_unknown14_2)+
			_game_voice_session_source1;
	}
}

/* 0x00054810: takes what another console sent of what its players said
(voice_queues_read_packet) */
void game_voice_read_packet(
	const byte *packet,
	long size)
{
	if (game_voice_active() && unknown_004c9888!=0)
	{
		voice_queues_read_packet(size, unknown_00525a00, packet);
	}
}

/* 0x00054850: whether there is something to send to a console */
boolean game_voice_has_packet_for_peer(
	long peer)
{
	boolean result= FALSE;

	if (game_voice_active() && unknown_004c9888!=0)
	{
		result= voice_queues_go_to_peer(peer, unknown_00525a00);
	}

	return result;
}

/* 0x00054990: how much there is to send to a console */
long game_voice_packet_size_for_peer(
	long peer)
{
	long result= 0;

	if (game_voice_active() && unknown_004c9888!=0)
	{
		result= voice_queues_size_for_peer(unknown_00525a00, peer);
	}

	return result;
}

/* 0x000549d0: whether this console is that of unknown_004c9878 */
boolean game_voice_unknown_000549d0(
	void)
{
	if (game_voice_active())
	{
		long peer= game_voice_get_local_peer();

		if (peer!=NONE && peer==game_voice_get_unknown_004c9878())
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* 0x00054a20: which of the consoles of the session this one is; NONE with
no session */
long game_voice_get_local_peer(
	void)
{
	if (game_voice_active())
	{
		const byte *session= game_voice_get_valid_session();

		if (session)
		{
			return *(const long *)(session+GAME_VOICE_SESSION_LOCAL_PEER_OFFSET);
		}
	}

	return NONE;
}

/* 0x00054a70: another number of the session; NONE with no session */
long game_voice_session_unknown40(
	void)
{
	if (game_voice_active())
	{
		const byte *session= game_voice_get_valid_session();

		if (session)
		{
			return *(const long *)(session+GAME_VOICE_SESSION_UNKNOWN40_OFFSET);
		}
	}

	return NONE;
}

/* 0x00054ac0: a bit for each console that this one is joined to, as the
session has it; 0 with no session */
long game_voice_local_peer_unknownA8(
	void)
{
	if (game_voice_active())
	{
		const byte *membership= game_voice_get_membership();
		long peer= game_voice_get_local_peer();

		if (membership && peer!=NONE)
		{
			return *(const long *)(membership+peer*GAME_VOICE_MEMBERSHIP_PEER_SIZE+
				GAME_VOICE_MEMBERSHIP_PEER_UNKNOWNA8_OFFSET);
		}
	}

	return 0;
}

/* 0x00054b10: a bit for each player that the session has */
unsigned long game_voice_get_players(
	void)
{
	if (game_voice_active())
	{
		const byte *membership= game_voice_get_membership();

		if (membership)
		{
			return *(const unsigned long *)(membership+GAME_VOICE_MEMBERSHIP_PLAYERS_OFFSET);
		}
	}

	return 0;
}

/* 0x00054b40: the players of the session; NULL with no session */
byte *game_voice_get_player_array(
	void)
{
	if (game_voice_active())
	{
		byte *membership= game_voice_get_membership();

		if (membership)
		{
			return membership+GAME_VOICE_MEMBERSHIP_PLAYER_ARRAY_OFFSET;
		}
	}

	return NULL;
}

/* 0x00054b70: the player of this console that has a controller; NONE if
there is none. The game has game_voice_get_player_array and
game_voice_get_players in the function itself */
long game_voice_find_local_player(
	long controller_index)
{
	if (game_voice_active())
	{
		const byte *membership= game_voice_get_membership();
		const byte *players= membership ? membership+GAME_VOICE_MEMBERSHIP_PLAYER_ARRAY_OFFSET : NULL;
		long peer= game_voice_get_local_peer();
		unsigned long mask= 0;

		if (game_voice_active())
		{
			const byte *membership_again= game_voice_get_membership();

			if (membership_again)
			{
				mask= *(const unsigned long *)(membership_again+GAME_VOICE_MEMBERSHIP_PLAYERS_OFFSET);
			}
		}
		if (peer!=NONE && players && mask)
		{
			long player_index;

			for (player_index= 0; player_index<MAXIMUM_GAME_VOICE_PLAYERS; player_index++)
			{
				const byte *player= players+player_index*GAME_VOICE_MEMBERSHIP_PLAYER_SIZE;

				if ((mask&(1u<<player_index)) &&
					*(const long *)(player+GAME_VOICE_MEMBERSHIP_PLAYER_PEER_OFFSET)==peer &&
					*(const long *)(player+GAME_VOICE_MEMBERSHIP_PLAYER_CONTROLLER_OFFSET)==controller_index)
				{
					return player_index;
				}
			}
		}
	}

	return NONE;
}

/* 0x00054c20: the controller of a player of this console; NONE if it is
not one */
long game_voice_player_controller(
	long player_index)
{
	if (game_voice_active())
	{
		if ((1u<<player_index)&game_voice_get_local_players())
		{
			const byte *players= game_voice_get_player_array();

			if (players)
			{
				return *(const long *)(players+player_index*GAME_VOICE_MEMBERSHIP_PLAYER_SIZE+
					GAME_VOICE_MEMBERSHIP_PLAYER_CONTROLLER_OFFSET);
			}
		}
	}

	return NONE;
}

/* 0x00054c70: whether there is a console in unknown_004c9878 and it is not
this one */
boolean game_voice_unknown_00054c70(
	void)
{
	if (game_voice_active())
	{
		if (!game_voice_unknown_000549d0())
		{
			return game_voice_get_unknown_004c9878()!=NONE;
		}
	}

	return FALSE;
}

/* 0x00054cc0 */
long game_voice_unknown_00054cc0(
	void)
{
	if (game_voice_enabled)
	{
		if (voice_globals.initialized)
		{
			if (unknown_004c9888==1)
			{
				return unknown_004c9880;
			}
			if (unknown_004c9888>1 && unknown_004c9888<=3)
			{
				return unknown_004c9884;
			}
		}
	}

	return 0;
}

/* 0x00054d00: a bit for each player that has a voice; 0 with no voice */
unsigned long game_voice_get_talkers(
	void)
{
	return game_voice_active() ? game_voice_talkers : 0;
}

/* 0x000536b0: whether the player of a controller can talk: the controller
has a headset, its player does not have the third setting and, with a flag
of what the game knows of the player, what unknown_001906da says */
boolean game_voice_controller_can_talk(
	long controller_index)
{
	if (game_voice_active())
	{
		boolean setting_allows= game_voice_controller_settings[controller_index]!=_game_voice_controller_setting3;
		boolean has_headset= voice_globals.controller_has_headset[controller_index];
		boolean allowed= TRUE;
		unsigned long flags;

		memcpy(&flags, unknown_0054e8e0[controller_index]+UNKNOWN_0054E8E0_FLAGS_OFFSET, sizeof(flags));
		if ((flags>>_unknown_0054e8e0_unknown5_bit)&1)
		{
			allowed= unknown_001906da(controller_index);
		}
		if (has_headset && setting_allows && allowed)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* 0x00053750: whether a player is talking: one of this console, if the
library gave something that its controller's player said since the last
time; another, if the library says so */
boolean game_voice_player_is_talking(
	long player_index)
{
	if (game_voice_active())
	{
		if ((1u<<player_index)&game_voice_get_local_players())
		{
			long controller_index= game_voice_player_controller(player_index);

			if (controller_index==NONE)
			{
				return FALSE;
			}

			return voice_globals.initialized ? voice_globals.controller_talked[controller_index] : FALSE;
		}

		return voice_remote_talker_is_talking(&voice_globals, player_index);
	}

	return FALSE;
}

/* 0x00054d20: a bit for each player of the session that is of this console */
unsigned long game_voice_get_local_players(
	void)
{
	unsigned long local_players= 0;

	if (game_voice_active())
	{
		const byte *membership= game_voice_get_membership();

		if (membership)
		{
			const byte *players= membership+GAME_VOICE_MEMBERSHIP_PLAYER_ARRAY_OFFSET;
			unsigned long mask= game_voice_get_players();
			long peer= game_voice_get_local_peer();
			long player_index;

			for (player_index= 0; player_index<MAXIMUM_GAME_VOICE_PLAYERS; player_index++)
			{
				if ((mask&(1u<<player_index)) &&
					*(const long *)(players+player_index*GAME_VOICE_MEMBERSHIP_PLAYER_SIZE+
					GAME_VOICE_MEMBERSHIP_PLAYER_PEER_OFFSET)==peer)
				{
					local_players|= 1u<<player_index;
				}
			}
		}
	}

	return local_players;
}

/* 0x00054ec0 */
boolean game_voice_unknown_00054ec0(
	void)
{
	boolean result= FALSE;

	if (game_voice_active())
	{
		result= unknown_004c9888!=0;
	}

	return result;
}

/* 0x00054ef0 */
boolean game_voice_unknown_00054ef0(
	void)
{
	boolean result= FALSE;

	if (game_voice_active())
	{
		result= unknown_004c9888==1;
	}

	return result;
}

/* 0x00054f20: which of the two places the session comes from; 0 with no
voice */
long game_voice_get_session_source(
	void)
{
	return game_voice_active() ? game_voice_session_source : 0;
}

/* 0x00054f40: the two numbers of 16 bits of the player of a controller, as
one: its bits of the others below, and what the game knows of its voice
above. 0 if the controller has no player */
unsigned long game_voice_controller_player_words(
	long controller_index)
{
	unsigned long result= 0;

	if (game_voice_active())
	{
		long player_index= game_voice_find_local_player(controller_index);
		word low= 0;
		word high= 0;

		if (player_index!=NONE)
		{
			low= game_voice_player_mutes[player_index];
			high= game_voice_player_flag_words[player_index];
		}
		result= ((unsigned long)high<<16)|low;
	}

	return result;
}

/* 0x00054f90: what a controller's player wants of the voice; 0 with no
voice */
long game_voice_get_controller_setting(
	long controller_index)
{
	return game_voice_active() ? game_voice_controller_settings[controller_index] : 0;
}

/* 0x00054fc0 */
void game_voice_set_controller_setting(
	long controller_index,
	long setting)
{
	if (game_voice_active())
	{
		game_voice_controller_settings[controller_index]= setting;
	}
}

/* 0x00055060: writes formatted text to a text of GAME_VOICE_TEXT_SIZE; the
text */
char *game_voice_text_printf(
	char *text,
	const char *format,
	...)
{
	va_list arguments;

	va_start(arguments, format);
	vsnprintf(text, GAME_VOICE_TEXT_SIZE-1, format, arguments);
	va_end(arguments);
	text[GAME_VOICE_TEXT_SIZE-1]= 0;

	return text;
}

/* 0x00053580: the engine of the voice does its work of the frame. What each
controller's player said is forgotten first: the engine gives it again as
it works. The game has game_voice_library_begin, game_voice_library_end and
a function of voice.c in this one */
void game_voice_library_update(
	void)
{
	if (game_voice_active())
	{
		game_voice_library_begin();
		if (voice_globals.initialized)
		{
			memset(voice_globals.controller_talked, 0, sizeof(voice_globals.controller_talked));
			unknown_00332f22(voice_globals.engine);
		}
		game_voice_library_end();
	}
}

/* 0x00053d40: with a flag of the kind of game, if unknown_0055e4d0 has
something for the number of unknown_004e9ae8; or what
game_voice_unknown_00053d90 says, which the game has in this one */
boolean game_voice_unknown_00053d40(
	void)
{
	boolean flag= FALSE;
	const byte *unknown;
	boolean state7;

	if (unknown_0055e4d0[*(const long *)(unknown_004e9ae8+UNKNOWN_004E9AE8_UNKNOWNC14_OFFSET)])
	{
		flag= *((const byte *)main_game_globals+MAIN_GAME_GLOBALS_VARIANT_FLAGS_OFFSET)&1;
	}
	unknown= unknown_000649c0();
	state7= unknown && *(const long *)(unknown+UNKNOWN_000649C0_STATE_OFFSET)==_unknown_000649c0_state7;

	return flag || state7;
}

/* 0x00053fb0: nothing is known of the voice of any player */
void game_voice_reset_players(
	void)
{
	game_voice_talkers= 0;
	game_voice_session_players= 0;
	memset(game_voice_session_player_identifiers, 0, sizeof(game_voice_session_player_identifiers));
	memset(game_voice_player_mutes, 0, sizeof(game_voice_player_mutes));
	memset(game_voice_player_flag_words, 0, sizeof(game_voice_player_flag_words));
}

/* 0x00054410: takes what the session says of the voice of a player, and has
the library give the player a voice or take it away, by what
game_voice_player_can_be_heard says. The game does not see whether there is a
session, and has game_voice_get_player_array and
game_voice_has_remote_talker in the function itself */
void game_voice_update_player(
	long player_index)
{
	const byte *membership= game_voice_active() ? game_voice_get_membership() : NULL;
	const byte *players= membership ? membership+GAME_VOICE_MEMBERSHIP_PLAYER_ARRAY_OFFSET : NULL;
	const byte *player= players+player_index*GAME_VOICE_MEMBERSHIP_PLAYER_SIZE;
	boolean wanted;
	boolean has_voice= FALSE;
	boolean result;

	game_voice_player_mutes[player_index]= *(const word *)(player+GAME_VOICE_MEMBERSHIP_PLAYER_MUTES_OFFSET);
	game_voice_player_flag_words[player_index]= *(const word *)(player+GAME_VOICE_MEMBERSHIP_PLAYER_FLAGS_OFFSET);
	wanted= game_voice_player_can_be_heard(player_index);
	if (game_voice_active())
	{
		has_voice= voice_has_remote_talker(&voice_globals, player_index);
	}
	result= wanted;
	if (has_voice!=wanted)
	{
		boolean set= voice_remote_talker_set(&voice_globals, player_index, wanted);

		if (wanted)
		{
			result= set;
		}
	}
	if (result)
	{
		game_voice_talkers|= (word)(1u<<player_index);
	}
	else
	{
		game_voice_talkers&= (word)~(1u<<player_index);
	}
}

/* 0x000544e0: what the voice makes of the state of the network game */
void game_voice_update_network_state(
	void)
{
	long state= unknown_00527330 ? unknown_00527334 : 0;

	if (state<1 || state>_network_game_state_last)
	{
		unknown_004c9888= _game_voice_network_none;
		return;
	}
	switch (state)
	{
	case _network_game_state3:
		if (game_in_progress() && unknown_00068250())
		{
			unknown_004c9888= _game_voice_network1;
			return;
		}
		break;
	case _network_game_state8:
		if (game_in_progress() && unknown_00068250())
		{
			unknown_004c9888= _game_voice_network1;
			return;
		}
		/* fall through */
	case _network_game_state7:
		{
			const byte *unknown= unknown_000649c0();

			if (unknown && (unknown[UNKNOWN_000649C0_FLAGS_OFFSET]&1))
			{
				unknown_004c9888= _game_voice_network3;
				return;
			}
		}
		break;
	}
	unknown_004c9888= _game_voice_network2;
}

/* 0x00054df0: something of a player, in a game that goes on: what
simulation_world_player_unknown25 says of what unknown_004cf770 has of it.
The game looks whether there is such a player before asking, and that
function looks again */
boolean game_voice_unknown_00054df0(
	long player)
{
	if (game_voice_active() && main_game_globals)
	{
		if (!main_game_globals->active || !unknown_004cf770.unknownC)
		{
			return FALSE;
		}
		else
		{
			unsigned long index= (unsigned long)player&0xFFFF;

			if (index<MAXIMUM_GAME_VOICE_PLAYERS)
			{
				const byte *unknown= unknown_004cf770.unknownC+index*UNKNOWN_004CF770_PLAYER_SIZE+
					UNKNOWN_004CF770_PLAYER_OFFSET;

				if (*(const long *)unknown!=NONE && simulation_world_player_unknown25(player, unknown_004cf770.unknownC))
				{
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

/* 0x00054e60: a player of the game, if game_voice_unknown_00054df0 says so;
NULL if not */
byte *game_voice_get_player_datum(
	long player)
{
	if (game_voice_active() && game_voice_unknown_00054df0(player))
	{
		long handle= NONE;

		if (player!=NONE)
		{
			handle= ((long)*(const short *)((const byte *)player_data->data+player_data->size*player)<<16)|player;
		}

		return (byte *)player_data->data+(handle&0xFFFF)*GAME_VOICE_PLAYER_DATUM_SIZE;
	}

	return NULL;
}

/* 0x00054fe0: what the game keeps of the profile of a gamepad's player gets
the gamepad and the numbers of the voice of the player. The game copies a
part of it out and back in around that */
void game_voice_update_profile(
	long controller_index)
{
	long profile_index;

	memcpy(&profile_index, unknown_0054e8e0[controller_index]+UNKNOWN_0054E8E0_PROFILE_OFFSET, sizeof(profile_index));
	if (profile_index!=NONE && unknown_004cd904[profile_index][0])
	{
		byte *profile= unknown_004cd904[profile_index];
		byte unknown[UNKNOWN_004CD904_UNKNOWN14_SIZE];
		unsigned long words;

		memcpy(unknown, profile+UNKNOWN_004CD904_UNKNOWN14_OFFSET, sizeof(unknown));
		words= game_voice_controller_player_words(controller_index);
		memcpy(profile+UNKNOWN_004CD904_GAMEPAD_OFFSET, &controller_index, sizeof(controller_index));
		memcpy(profile+UNKNOWN_004CD904_UNKNOWN14_OFFSET, unknown, sizeof(unknown));
		memcpy(profile+UNKNOWN_004CD904_VOICE_WORDS_OFFSET, &words, sizeof(words));
	}
}

/* 0x00053610: the voice, each frame: the engine does its work and, with a
full voice, the game looks at where the session comes from and at the state
of the network game. With no network the players of other consoles lose
their voices and nothing is known of any player; with one, the rest of what
is done each frame */
void game_voice_update(
	void)
{
	if (game_voice_active())
	{
		game_voice_library_update();
		if (game_voice_kind==_voice_kind_full)
		{
			game_voice_update_session_source();
			game_voice_update_network_state();
			if (!(game_voice_active() && game_voice_session_source!=0))
			{
				unknown_004c9888= _game_voice_network_none;
			}
			game_voice_update_session_players();
			if (!game_voice_unknown_00054ec0())
			{
				voice_remove_remote_talkers(&voice_globals);
				game_voice_reset_players();
				return;
			}
			game_voice_update_players();
			game_voice_update_chosen_peer();
			voice_channels_update(unknown_00527108);
			if (unknown_005259b8[0])
			{
				voice_targets_update(unknown_005259b8);
			}
		}
	}
}

/* 0x00053a20: the library gave something that the player of a controller
said (voice_callback_data_ready). It goes on its way if the controller has a
player, the player has neither the second flag nor the third, unknown_005259b8
has a number for the player and unknown_00527104 gives something for that
number. The game has game_voice_player_voice_off in the function itself */
void GAME_VOICE_CALLBACK game_voice_local_data_ready(
	long controller_index,
	void *data,
	unsigned long size)
{
	if (game_voice_active())
	{
		long player_index= game_voice_find_local_player(controller_index);

		if (player_index!=NONE &&
			!(game_voice_active() &&
			((game_voice_player_flags(player_index)>>_game_voice_player_voice_off_bit)&1)) &&
			!game_voice_player_not_allowed(player_index))
		{
			long unknown= voice_targets_get(unknown_005259b8, player_index);

			if (unknown)
			{
				word words[2];
				boolean flag;

				words[0]= 0;
				words[1]= 0;
				flag= FALSE;
				voice_unknown_00527104_get_words(unknown, words, unknown_00527104);
				if (words[0])
				{
					player_index= game_voice_find_local_player(controller_index);
					if (player_index!=NONE)
					{
						if (unknown_00527108[0] &&
							*(const long *)(unknown_00527108+UNKNOWN_00527108_PLAYERS_OFFSET+
							player_index*sizeof(long)))
						{
							flag= TRUE;
						}
						voice_queues_add(player_index, unknown_00525a00, words, data, size, &flag);
					}
				}
			}
		}
	}
}

/* 0x00053c70: with a session from the second place: a bit for each of its
consoles that the session of the first place has too. The game does not see
whether the session says who is in it */
unsigned long game_voice_unknown_00053c70(
	void)
{
	unsigned long result= 0;

	if (game_voice_active() && game_voice_session_source==_game_voice_session_source2)
	{
		byte *session= NULL;
		byte *other_session= NULL;

		if (network_life_cycle_get_other_session(&session) && network_life_cycle_get_session(&other_session))
		{
			const byte *membership= NULL;
			long count;
			long peer;

			if (*(const long *)(session+GAME_VOICE_SESSION_UNKNOWN741C_OFFSET) &&
				*(const long *)(session+GAME_VOICE_SESSION_MEMBERSHIP_OFFSET)!=NONE)
			{
				membership= session+GAME_VOICE_SESSION_MEMBERSHIP_OFFSET;
			}
			count= *(const long *)(membership+GAME_VOICE_MEMBERSHIP_PEER_COUNT_OFFSET);
			for (peer= 0; peer<count; peer++)
			{
				if (network_session_find_peer(other_session, membership+GAME_VOICE_MEMBERSHIP_PEER_ARRAY_OFFSET+
					peer*GAME_VOICE_MEMBERSHIP_PEER_SIZE)!=NONE)
				{
					result|= 1u<<peer;
				}
			}
		}
	}

	return result;
}

/* 0x00053e30: says which console of the session is chosen for the voice
(unknown_004c9878). A console that does not run the session takes the one
that the session has. The one that runs it chooses: of the others, the one
with the greatest second number among those that can be chosen, have their
two numbers no less than the least and are joined to every console; if
there is none, this one, unless game_voice_unknown_00054ef0 says so; and
tells the session if it had another */
void game_voice_update_chosen_peer(
	void)
{
	byte *session;

	unknown_004c9878= NONE;
	if (game_voice_active())
	{
		session= NULL;
		if (game_voice_get_session(&session))
		{
			const byte *membership= game_voice_get_membership();
			long local_peer= game_voice_get_local_peer();
			long host_peer= game_voice_session_unknown40();

			if (membership && local_peer!=NONE && host_peer!=NONE)
			{
				if (local_peer==host_peer)
				{
					long count= *(const long *)(membership+GAME_VOICE_MEMBERSHIP_PEER_COUNT_OFFSET);
					long chosen= NONE;
					long greatest= 0;
					long peer;

					for (peer= 0; peer<count; peer++)
					{
						const byte *entry= membership+peer*GAME_VOICE_MEMBERSHIP_PEER_SIZE;
						long value= *(const long *)(entry+GAME_VOICE_MEMBERSHIP_PEER_UNKNOWNA0_OFFSET);

						if (local_peer!=peer &&
							*(const long *)(entry+GAME_VOICE_MEMBERSHIP_PEER_UNKNOWNA4_OFFSET)!=_game_voice_peer_unknownA4_3 &&
							*(const long *)(entry+GAME_VOICE_MEMBERSHIP_PEER_UNKNOWN9C_OFFSET)>=unknown_004ce124 &&
							value>=unknown_004ce120 &&
							*(const long *)(entry+GAME_VOICE_MEMBERSHIP_PEER_UNKNOWNA8_OFFSET)==(1<<count)-1 &&
							value>greatest)
						{
							chosen= peer;
							greatest= value;
						}
					}
					if (chosen==NONE && !game_voice_unknown_00054ef0())
					{
						chosen= local_peer;
					}
					if (network_session_get_chosen_peer(session)!=chosen)
					{
						network_session_host_set_chosen_peer(session, chosen);
					}
					unknown_004c9878= chosen;
				}
				else
				{
					unknown_004c9878= network_session_get_chosen_peer(session);
				}
			}
		}
	}
}

/* 0x00054890: whether this console is the one chosen for the voice, or
what some player of it says goes to a console: one that has neither the
second flag nor the third, and whose number of unknown_005259b8 gets from
unknown_00527104 the bit of the console. FALSE with no session */
boolean game_voice_unknown_00054890(
	long peer)
{
	boolean result= FALSE;

	if (game_voice_active() && game_voice_get_membership())
	{
		unsigned long local_players;
		long player_index;

		if (game_voice_unknown_000549d0())
		{
			return TRUE;
		}
		local_players= game_voice_get_local_players();
		for (player_index= 0; player_index<MAXIMUM_GAME_VOICE_PLAYERS; player_index++)
		{
			if ((local_players&(1u<<player_index)) &&
				!(game_voice_active() &&
				((game_voice_player_flags(player_index)>>_game_voice_player_voice_off_bit)&1)) &&
				!game_voice_player_not_allowed(player_index))
			{
				long unknown= voice_targets_get(unknown_005259b8, player_index);

				if (unknown)
				{
					word words[2];

					words[0]= 0;
					words[1]= 0;
					voice_unknown_00527104_get_words(unknown, words, unknown_00527104);
					if (words[0]&(1u<<peer))
					{
						return TRUE;
					}
				}
			}
		}
	}

	return result;
}

/* 0x00054030: each frame, each of the sixteen players. One that the session
does not have loses the voice that the library gave it, and its bit. One of
another console: game_voice_update_player. One of this console: the same
loss, and game_voice_update_local_player. The game has
game_voice_get_players in the function itself, and where a player loses its
voice, game_voice_has_remote_talker and voice_remote_talker_set */
void game_voice_update_players(
	void)
{
	if (game_voice_active() && game_voice_get_membership())
	{
		unsigned long local_players= game_voice_get_local_players();
		unsigned long players= 0;
		long player_index;

		if (game_voice_active())
		{
			const byte *membership= game_voice_get_membership();

			if (membership)
			{
				players= *(const unsigned long *)(membership+GAME_VOICE_MEMBERSHIP_PLAYERS_OFFSET);
			}
		}
		for (player_index= 0; player_index<MAXIMUM_GAME_VOICE_PLAYERS; player_index++)
		{
			unsigned long bit= 1u<<player_index;

			if (players&bit)
			{
				if (local_players&bit)
				{
					if (game_voice_active() && voice_has_remote_talker(&voice_globals, player_index))
					{
						voice_remote_talker_set(&voice_globals, player_index, FALSE);
					}
					game_voice_update_local_player(player_index);
				}
				else
				{
					game_voice_update_player(player_index);
				}
			}
			else
			{
				if (game_voice_active() && voice_has_remote_talker(&voice_globals, player_index))
				{
					voice_remote_talker_set(&voice_globals, player_index, FALSE);
				}
				game_voice_talkers&= (word)~bit;
			}
		}
	}
}

/* 0x000541a0: each frame, a player of this console. By whether its
controller can talk, whether its player wants a voice and whether it is
allowed one, sets the mode of the controller (the third one for a voice, the
first for none) and the three flags of the player. Then, if the list of
those that the player wants not to hear is known, says which of the players
of other consoles are in it; has what the game keeps of the profile brought
up to date; and gives the player its bit, or takes it away */
void GAME_VOICE_CALLBACK game_voice_update_local_player(
	long player_index)
{
	long controller_index= game_voice_player_controller(player_index);

	if (controller_index!=NONE)
	{
		const byte *players= NULL;
		boolean voice_off;
		boolean allowed= TRUE;
		boolean can_talk;
		long mode;
		const byte *profile;
		unsigned long flags;
		word player_flags;

		if (game_voice_active())
		{
			const byte *membership= game_voice_get_membership();

			if (membership)
			{
				players= membership+GAME_VOICE_MEMBERSHIP_PLAYER_ARRAY_OFFSET;
			}
		}
		voice_off= game_voice_controller_settings[controller_index]==_game_voice_controller_setting3;
		mode= voice_globals.initialized ? voice_globals.controller_modes[controller_index] : 0;
		profile= unknown_0054e8e0[controller_index];
		memcpy(&flags, profile+UNKNOWN_0054E8E0_FLAGS_OFFSET, sizeof(flags));
		if ((flags>>_unknown_0054e8e0_unknown5_bit)&1)
		{
			allowed= unknown_001906da(controller_index);
		}
		can_talk= game_voice_controller_can_talk(controller_index) && !voice_off && allowed;

		player_flags= game_voice_player_flag_words[player_index];
		if (!voice_off && allowed)
		{
			if (mode==_voice_mode_unknown0)
			{
				game_voice_set_controller_mode(controller_index, _voice_mode_unknown2);
				player_flags= game_voice_player_flag_words[player_index];
			}
			player_flags&= (word)~(1u<<_game_voice_player_voice_off_bit);
		}
		else
		{
			if (mode==_voice_mode_unknown2)
			{
				game_voice_set_controller_mode(controller_index, _voice_mode_unknown0);
				player_flags= game_voice_player_flag_words[player_index];
			}
			if (voice_off)
			{
				player_flags|= 1u<<_game_voice_player_voice_off_bit;
			}
			else
			{
				player_flags&= (word)~(1u<<_game_voice_player_voice_off_bit);
			}
		}
		if (can_talk)
		{
			player_flags|= 1u<<_game_voice_player_can_talk_bit;
		}
		else
		{
			player_flags&= (word)~(1u<<_game_voice_player_can_talk_bit);
		}
		if (!allowed)
		{
			player_flags|= 1u<<_game_voice_player_not_allowed_bit;
		}
		else
		{
			player_flags&= (word)~(1u<<_game_voice_player_not_allowed_bit);
		}
		game_voice_player_flag_words[player_index]= player_flags;

		memcpy(&flags, profile+UNKNOWN_0054E8E0_FLAGS_OFFSET, sizeof(flags));
		if (((flags>>_unknown_0054e8e0_unknown5_bit)&1) && game_voice_muted_player_counts[controller_index]!=NONE)
		{
			long count= game_voice_muted_player_counts[controller_index];
			unsigned long session_players= game_voice_get_players();
			unsigned long local_players= game_voice_get_local_players();
			long other_index;

			for (other_index= 0; other_index<MAXIMUM_GAME_VOICE_PLAYERS; other_index++)
			{
				unsigned long bit= 1u<<other_index;

				if ((session_players&bit) && !(local_players&bit))
				{
					const byte *other= players+other_index*GAME_VOICE_MEMBERSHIP_PLAYER_SIZE+
						GAME_VOICE_MEMBERSHIP_PLAYER_IDENTIFIER_OFFSET;
					boolean muted= FALSE;
					long index;

					for (index= 0; index<count; index++)
					{
						if (!memcmp(game_voice_muted_players[controller_index][index], other,
							GAME_VOICE_PLAYER_IDENTIFIER_SIZE))
						{
							muted= TRUE;
							break;
						}
					}
					if (muted)
					{
						game_voice_player_mutes[player_index]|= (word)bit;
					}
					else
					{
						game_voice_player_mutes[player_index]&= (word)~bit;
					}
				}
			}
		}
		game_voice_update_profile(controller_index);
		if (can_talk)
		{
			game_voice_talkers|= (word)(1u<<player_index);
			return;
		}
	}
	game_voice_talkers&= (word)~(1u<<player_index);
}

/* 0x00054590: each frame, sees whether the players of the session are the
ones of the last time: the same bits, and each known by the same twelve
bytes. If not, keeps the new ones and tells unknown_00525a00. The game also
makes a text that says which of the two changed, and does nothing with it:
what told of it is not in this build. It has game_voice_get_player_array
and game_voice_get_players in the function itself */
void game_voice_update_session_players(
	void)
{
	const byte *players= NULL;
	unsigned long session_players= 0;
	char text[GAME_VOICE_TEXT_SIZE];
	byte none[GAME_VOICE_PLAYER_IDENTIFIER_SIZE];
	boolean changed= FALSE;
	long player_index;

	if (game_voice_active())
	{
		const byte *membership= game_voice_get_membership();

		if (membership)
		{
			players= membership+GAME_VOICE_MEMBERSHIP_PLAYER_ARRAY_OFFSET;
		}
	}
	if (game_voice_active())
	{
		const byte *membership= game_voice_get_membership();

		if (membership)
		{
			session_players= *(const unsigned long *)(membership+GAME_VOICE_MEMBERSHIP_PLAYERS_OFFSET);
		}
	}
	text[0]= 0;
	memset(none, 0, sizeof(none));
	if ((unsigned long)game_voice_session_players!=session_players)
	{
		strncpy(text, "session player mask change", GAME_VOICE_TEXT_SIZE);
		text[GAME_VOICE_TEXT_SIZE-1]= 0;
		changed= TRUE;
	}
	else
	{
		for (player_index= 0; player_index<MAXIMUM_GAME_VOICE_PLAYERS; player_index++)
		{
			const byte *identifier= (session_players&(1u<<player_index)) ?
				players+player_index*GAME_VOICE_MEMBERSHIP_PLAYER_SIZE+GAME_VOICE_MEMBERSHIP_PLAYER_IDENTIFIER_OFFSET :
				none;

			if (memcmp(game_voice_session_player_identifiers+player_index*GAME_VOICE_PLAYER_IDENTIFIER_SIZE,
				identifier, GAME_VOICE_PLAYER_IDENTIFIER_SIZE))
			{
				game_voice_text_printf(text, "player identifier change");
				changed= TRUE;
				break;
			}
		}
	}
	if (changed)
	{
		game_voice_session_players= (long)session_players;
		for (player_index= 0; player_index<MAXIMUM_GAME_VOICE_PLAYERS; player_index++)
		{
			const byte *identifier= (session_players&(1u<<player_index)) ?
				players+player_index*GAME_VOICE_MEMBERSHIP_PLAYER_SIZE+GAME_VOICE_MEMBERSHIP_PLAYER_IDENTIFIER_OFFSET :
				none;

			memcpy(game_voice_session_player_identifiers+player_index*GAME_VOICE_PLAYER_IDENTIFIER_SIZE, identifier,
				GAME_VOICE_PLAYER_IDENTIFIER_SIZE);
		}
		voice_queues_reset(unknown_00525a00);
	}
}
