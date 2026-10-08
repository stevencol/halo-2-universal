/*
SOUND_MEMORY.H

The memory that the console's libraries of the sound ask for. The name of
the file is a guess, and so are the names of its functions.
*/

#ifndef __SOUND_MEMORY_H
#define __SOUND_MEMORY_H

#include "cseries/cseries.h"
#include "memory/memory_pool.h"

/* ---------- constants */

enum
{
	/* what is given to an allocation of no size */
	SOUND_MEMORY_DEFAULT_SIZE= 0x200,

	/* the pool of the voice, in a game that has it */
	SOUND_MEMORY_POOL_SIZE= 0x78000,

	/* what the allocation of the pool is told with: as that of the files of
	the players */
	SOUND_MEMORY_ALLOCATION_UNKNOWN= 2
};

/* how much of the voice a game has, by how its memory is shared out */
enum
{
	/* a campaign that is not in a session of the network game */
	_sound_memory_voice_none,
	/* a campaign in a session */
	_sound_memory_voice_session,
	/* anything but a campaign: the only one with the pool */
	_sound_memory_voice_full
};

/* what the voice is told that it is, in each of them */
enum
{
	_voice_kind_none,
	_voice_kind_session,
	_voice_kind_full
};

/* as main_game.c calls those that take memory when a map is loaded */
#define SOUND_MEMORY_CALLBACK __attribute__((stdcall))

/* ---------- structures */

/* what the memory of the pool is asked with: the functions, and a number
for them */
struct sound_memory_allocation
{
	struct allocation allocation;
	long unknown;
};

/* 0x00476fc8: the voice of the players of the network game. Only what this
file sets is known */
struct voice_globals
{
	void *unknown0;
	boolean initialized;
	byte pad5[3];
	long kind;
	unsigned long unknownC;
	/* of the console's library; NULL if there is none */
	void *engine;
	byte unknown14[0x7C];
};

#if defined(__i386__)
typedef char verify_sound_memory_allocation_size[
	sizeof(struct sound_memory_allocation) == 0x8 ? 1 : -1];
typedef char verify_voice_globals_size[
	sizeof(struct voice_globals) == 0x90 ? 1 : -1];
#endif

/* ---------- prototypes/SOUND_MEMORY.C */

void *sound_memory_allocate(unsigned long size, unsigned long attributes);
void __attribute__((stdcall)) sound_memory_free(void *address, unsigned long attributes);
void SOUND_MEMORY_CALLBACK sound_memory_pool_new(long configuration);
void sound_memory_pool_delete(void);

/* ---------- globals */

extern struct memory_pool *sound_memory_pool;
extern struct memory_pool *sound_memory_other_pool;
extern boolean sound_memory_use_other_pool;
extern long sound_memory_voice;
extern long sound_memory_voice_kind;
extern struct sound_memory_allocation sound_memory_allocation;
extern struct voice_globals voice_globals;

#endif
