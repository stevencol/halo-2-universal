/*
ACTORS.C

the actors: the characters that the game moves by itself. Worked out from
halo2ship.exe; the name of the file is a guess, by the name that the game
gives to its table of data ("actor").
*/

#include "cseries/cseries.h"

#include <stdint.h>

#include "cache/cache_files.h"
#include "game/game.h"
#include "memory/data.h"
#include "memory/hash_table.h"
#include "saved games/game_state.h"

/* ---------- constants */

enum
{
	MAXIMUM_ACTORS= 0x100,
	ACTOR_SIZE= 0x888,

	/* of the table of who has each place to fire from */
	ACTOR_FIRING_POSITION_OWNER_LIST_COUNT= 0x400,

	/* of what unknown_004f93a0 points to */
	ACTOR_UNKNOWN_SIZE= 0x640
};

/* ---------- globals */

/* 0x004f55f0: the actors ("actor": 0x100 of 0x888 bytes). 0x004f93a0: 0x640
bytes of the state of the game. 0x00557c6c: who has each place to fire from
("actor firing-position owners": a number for each key) */
struct data_array *actor_data;
byte *unknown_004f93a0;
struct hash_table *actor_firing_position_owners;

/* ---------- code */

/* 0x0025dd20: the hash of a key of the table of owners: its lowest byte, by
four. In the game it is far from the function that gives it to the table, so
it may be of another file */
unsigned long HASH_TABLE_CALLBACK actor_firing_position_owner_hash(
	long key)
{
	return (unsigned long)(key&0xFF)<<2;
}

/* 0x0025dd30: whether two keys of the table of owners are the same. Far
from the others too */
boolean HASH_TABLE_CALLBACK actor_firing_position_owner_compare(
	long key,
	long other)
{
	return key==other;
}

/* 0x001dfae0: the table of the actors is made, in the memory of the state of
the game (data_new, which the game has written here); the 0x640 bytes are
taken there (game_state_malloc, written here too), and so is the table of the
owners */
void actors_initialize(
	void)
{
	actor_data= data_new("actor", MAXIMUM_ACTORS, ACTOR_SIZE, 0, game_state_allocation);
	unknown_004f93a0= game_state_malloc(ACTOR_UNKNOWN_SIZE);
	actor_firing_position_owners= hash_table_new(game_state_allocation, MAXIMUM_ACTORS,
		"actor firing-position owners", sizeof(long), ACTOR_FIRING_POSITION_OWNER_LIST_COUNT,
		actor_firing_position_owner_hash, actor_firing_position_owner_compare);
}

/* 0x001e4990: the first thing of the block that the definition of a
character (a tag) has at 0x34, or that of the character that it comes from
(at 8), and so on; NULL if none has one. By the tags of other games of the
same engine it is what is general of the character: its kind is the short at
4 of it */
void *character_definition_get_unknown34(
	long character_index)
{
	while (character_index!=NONE)
	{
		const byte *character= (const byte *)(uintptr_t)global_tag_instances[character_index&0xFFFF].address;

		if (*(const long *)(character+0x34)>0)
		{
			return *(void *const *)(character+0x38);
		}
		character_index= *(const long *)(character+8);
	}

	return NULL;
}

/* ---------- small functions of the actors

Most of the functions that follow take, besides the actor, one or two things
on the stack that they do not use: they look like functions that a table
names, all with the same arguments. What the things are is not known */

#define ACTOR_GET(actor_index) ((byte *)actor_data->data+((actor_index)&0xFFFF)*ACTOR_SIZE)

/* 0x001a8380: an actor has 4 for its short at 0x84; always true */
boolean __attribute__((stdcall)) actor_unknown_001a8380(
	long actor_index,
	long unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	*(short *)(actor+0x84)= 4;

	return TRUE;
}

/* 0x001b2310: an actor has 3 for its short at 0x84; always true */
boolean __attribute__((stdcall)) actor_unknown_001b2310(
	long actor_index,
	long unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	*(short *)(actor+0x84)= 3;

	return TRUE;
}

/* 0x001ab3a0: an actor has the time of the game for its number at 0x1F8 */
void __attribute__((stdcall)) actor_unknown_001ab3a0(
	long actor_index,
	long unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	*(unsigned long *)(actor+0x1F8)= game_time_globals->time;
}

/* 0x001c04c0: an actor has the time of the game for its number at 0x1F4 */
void __attribute__((stdcall)) actor_unknown_001c04c0(
	long actor_index,
	long unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	*(unsigned long *)(actor+0x1F4)= game_time_globals->time;
}

/* 0x001b5790: an actor has NONE for its number at 0x344 */
void __attribute__((stdcall)) actor_unknown_001b5790(
	long actor_index,
	long unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	*(long *)(actor+0x344)= NONE;
}

/* 0x001b5dd0: an actor has zeros for its bytes at 0x220 and 0x221 */
void __attribute__((stdcall)) actor_unknown_001b5dd0(
	long actor_index,
	long unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	actor[0x220]= 0;
	actor[0x221]= 0;
}

/* 0x001b6880: an actor has 1 for its byte at 0x4A1 */
void __attribute__((stdcall)) actor_unknown_001b6880(
	long actor_index,
	long unknown1,
	long unknown2)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	(void)unknown2;
	actor[0x4A1]= 1;
}

/* 0x001be6b0: an actor has 0 for its byte at 0x228 */
void __attribute__((stdcall)) actor_unknown_001be6b0(
	long actor_index,
	long unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	actor[0x228]= 0;
}

/* 0x001c1640: an actor has 0 for its byte at 0x354 */
void __attribute__((stdcall)) actor_unknown_001c1640(
	long actor_index,
	long unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	actor[0x354]= 0;
}

/* 0x001ad550: 3 for an actor that has something at 0x338 (not NONE), 0 if
not */
long __attribute__((stdcall)) actor_unknown_001ad550(
	long actor_index)
{
	if (*(const long *)(ACTOR_GET(actor_index)+0x338)!=NONE)
	{
		return 3;
	}

	return 0;
}

/* 0x001b2da0: the same for what it has at 0x85C */
long __attribute__((stdcall)) actor_unknown_001b2da0(
	long actor_index)
{
	if (*(const long *)(ACTOR_GET(actor_index)+0x85C)!=NONE)
	{
		return 3;
	}

	return 0;
}

/* 0x001e4260: an actor takes the bit 1 of its number at 0x810 */
void actor_unknown_001e4260(
	long actor_index)
{
	*(unsigned long *)(ACTOR_GET(actor_index)+0x810)|= 2;
}

/* 0x001f3610: the short at 0x61E of an actor is no less than a number:
it takes the number if it had less. Always true */
boolean actor_unknown_001f3610(
	long actor_index,
	short minimum)
{
	byte *actor= ACTOR_GET(actor_index);

	if (*(const short *)(actor+0x61E)<minimum)
	{
		*(short *)(actor+0x61E)= minimum;
	}

	return TRUE;
}
