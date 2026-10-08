/*
SOUND_MEMORY.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The memory that the console's libraries of the sound ask for, which comes
from one of two pools of the game. The name of the file is a guess, and so
are the names of its functions.
*/

#include "sound_memory.h"

#include "main/main_game.h"
#include "saved games/saved_game_files.h"

/* ---------- prototypes of functions not decompiled yet */

/* 0x00055810: starts the voice with the console's library; whether it
could */
boolean unknown_00055810(struct voice_globals *voice);

/* 0x000550b0: stops the voice, if it was started */
void unknown_000550b0(struct voice_globals *voice);

/* ---------- globals */

/* 0x004c995c, 0x004c9960 and 0x004c9964: the pools, and whether the second
one is used */
struct memory_pool *sound_memory_pool;
struct memory_pool *sound_memory_other_pool;
boolean sound_memory_use_other_pool;

/* 0x004c9890, 0x004c9894: how much of the voice the game has, and what the
voice is told that it is */
long sound_memory_voice;
long sound_memory_voice_kind;

/* 0x00476fbc: what the pool's memory is asked of: the same that gives the
files of the players theirs */
struct sound_memory_allocation sound_memory_allocation=
{
	{&saved_game_files_allocation_functions},
	SOUND_MEMORY_ALLOCATION_UNKNOWN
};

/* ---------- code */

/* the pool that the memory comes from now */
static inline __attribute__((always_inline)) struct memory_pool *sound_memory_get_pool(
	void)
{
	return sound_memory_use_other_pool ? sound_memory_other_pool : sound_memory_pool;
}

/* 0x00053290: a block of the pool; NULL if there is no room. An allocation
of no size gets 0x200 bytes. What is asked for of it is not looked at */
void *sound_memory_allocate(
	unsigned long size,
	unsigned long attributes)
{
	void *block= NULL;

	(void)attributes;

	if (!size)
	{
		size= SOUND_MEMORY_DEFAULT_SIZE;
	}
	memory_pool_block_allocate(sound_memory_get_pool(), &block, (long)size, 0, 0);

	return block;
}

/* 0x000532e0: gives back a block that sound_memory_allocate gave */
void __attribute__((stdcall)) sound_memory_free(
	void *address,
	unsigned long attributes)
{
	(void)attributes;

	if (address)
	{
		memory_pool_block_free(sound_memory_get_pool(), &address);
	}
}

/* 0x00053310: when a map is loaded, by how the memory is shared out: says
how much of the voice the game has and, if it is all of it, makes the pool
and starts the voice, if it is not started. A block of the pool is not moved,
and a new one goes where there is room among the others. With 0x000533e0, in
the table of those that take memory then (0x00453c00).

The game does not see whether there was memory for the pool before it writes
in it */
void SOUND_MEMORY_CALLBACK sound_memory_pool_new(
	long configuration)
{
	long voice;
	long size= 0;
	long kind= _voice_kind_none;

	if (configuration<=_main_game_memory_default)
	{
		voice= _sound_memory_voice_full;
	}
	else
	{
		voice= configuration<=_main_game_memory_campaign_session_coop ? _sound_memory_voice_session :
			_sound_memory_voice_none;
	}

	switch (voice)
	{
	case _sound_memory_voice_session:
		kind= _voice_kind_session;
		break;
	case _sound_memory_voice_full:
		size= SOUND_MEMORY_POOL_SIZE;
		kind= _voice_kind_full;
		break;
	}

	sound_memory_voice= voice;
	sound_memory_voice_kind= kind;
	if (voice==_sound_memory_voice_full)
	{
		sound_memory_pool= memory_pool_new(&sound_memory_allocation.allocation, size, "voice pool");
		sound_memory_pool->no_references= TRUE;
		sound_memory_pool->fill_gaps= TRUE;
		sound_memory_pool->unknown3E= TRUE;
		if (!voice_globals.initialized)
		{
			voice_globals.engine= NULL;
			voice_globals.kind= kind;
			voice_reset_controller_levels(&voice_globals);
			voice_reset_controllers(&voice_globals);
			voice_globals.initialized= unknown_00055810(&voice_globals);
		}
	}
}

/* 0x000533e0: stops the voice and gives the pool back, if there is one */
void sound_memory_pool_delete(
	void)
{
	if (sound_memory_pool)
	{
		unknown_000550b0(&voice_globals);
		memory_pool_delete(sound_memory_pool);
		sound_memory_pool= NULL;
	}
}
