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
	if (game_voice_enabled && voice_globals.initialized)
	{
		voice_dispose(&voice_globals);
		unknown_004c9966= 0;
	}
}
