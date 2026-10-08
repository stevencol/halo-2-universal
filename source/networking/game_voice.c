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
#include "main/main_game.h"
#include "saved games/saved_game_files.h"

/* ---------- prototypes of functions not decompiled yet */

/* 0x000536b0: something of a controller's player */
boolean unknown_000536b0(long controller_index);

/* 0x00059670, 0x000596a0: the session of the network game, from one place
or the other; whether there is one */
boolean unknown_00059670(byte **session);
boolean unknown_000596a0(byte **session);

/* 0x00056380: something that 0x00525a00 does with three numbers */
long unknown_00056380(long unknown4, byte *unknown, long unknown0, long unknown8);

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

/* 0x004c9966: something of the voice that is no more when it stops */
word unknown_004c9966;

/* 0x004c9888: 0 when what a player of another console said is not given to
the library */
long unknown_004c9888;

/* 0x004c988c: which of two things gives the session of the network game */
long game_voice_session_source;

/* 0x004c9988: what the game knows of the voice of each player */
word game_voice_player_flag_words[MAXIMUM_GAME_VOICE_PLAYERS];

/* 0x00525a00: something that game_voice_unknown_00053b00 hands on. Its size
is not known */
byte unknown_00525a00[4];

/* 0x004c9968: for each player, a bit for each of the others */
word unknown_004c9968[MAXIMUM_GAME_VOICE_PLAYERS];

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
		unknown_004c9966= 0;
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

/* 0x00053850: whether a player has the bit of another */
boolean game_voice_unknown_00053850(
	long player_index,
	long other_player_index)
{
	boolean result= FALSE;

	if (game_voice_active())
	{
		result= ((1<<other_player_index)&unknown_004c9968[player_index])!=0;
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
It is played without a headset if the player wants that, or if what
unknown_000536b0 says of the player is not so */
void game_voice_play_message(
	long controller_index,
	const long *message,
	long size)
{
	if (game_voice_active())
	{
		long setting= game_voice_controller_settings[controller_index];
		boolean without_headset= FALSE;

		if (setting==_game_voice_controller_setting1 || !unknown_000536b0(controller_index))
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

/* 0x00053b00: hands three numbers on to unknown_00056380; 0 with no voice */
long game_voice_unknown_00053b00(
	long unknown0,
	long unknown4,
	long unknown8)
{
	long result= 0;

	if (game_voice_active() && unknown_004c9888!=0)
	{
		result= unknown_00056380(unknown4, unknown_00525a00, unknown0, unknown8);
	}

	return result;
}

/* 0x00053b40: whether a player has the first flag and not the second */
boolean game_voice_unknown_00053b40(
	long player_index)
{
	boolean result= FALSE;

	if (game_voice_active())
	{
		if ((game_voice_player_flags(player_index)&(1u<<_game_voice_player_unknown0_bit)) &&
			!(game_voice_active() &&
			((game_voice_player_flags(player_index)>>_game_voice_player_unknown1_bit)&1)))
		{
			result= TRUE;
		}
	}

	return result;
}

/* 0x00053b90 */
boolean game_voice_player_unknown1(
	long player_index)
{
	boolean result= FALSE;

	if (game_voice_active())
	{
		result= (game_voice_player_flags(player_index)>>_game_voice_player_unknown1_bit)&1;
	}

	return result;
}

/* 0x00053bb0 */
boolean game_voice_player_unknown2(
	long player_index)
{
	boolean result= FALSE;

	if (game_voice_active())
	{
		result= (game_voice_player_flags(player_index)>>_game_voice_player_unknown2_bit)&1;
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
			return unknown_00059670(session);
		case _game_voice_session_source2:
			return unknown_000596a0(session);
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
