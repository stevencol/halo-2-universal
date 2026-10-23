/*
LEVELS.C

the levels that there are to play. Worked out from halo2ship.exe; the name of
the file is a guess, by the names that the game gives to its two tables of
data ("level handles", "multiplayer level handles").
*/

#include "cseries/cseries.h"
#include "memory/data.h"
#include "scenario/scenario.h"

/* ---------- globals */

/* 0x004ee4e8: the levels of the campaign ("level handles": 0x14 of 8
bytes). 0x004ee4e4: those of the games of several ("multiplayer level
handles": 0x32 of 8 bytes) */
struct data_array *multiplayer_level_data;
struct data_array *campaign_level_data;

/* ---------- code */

/* 0x0019c1d0: when a map goes, neither table can be used. It is the fourth
of the functions of a part of the game (game.c) */
void levels_dispose_from_old_map(
	void)
{
	campaign_level_data->valid= FALSE;
	multiplayer_level_data->valid= FALSE;
}

/* 0x0019c120: the levels that there are are looked for. 0x0019c6d0,
0x0019c7e0: each table is filled */
void unknown_0019c120(void);
void __attribute__((stdcall)) unknown_0019c6d0(struct data_array *data, long unknown);
void __attribute__((stdcall)) unknown_0019c7e0(struct data_array *data);

/* 0x0019c190: for a new map, the two tables can be used and are empty
(data_delete_all); then the levels are looked for and each table is filled */
void levels_initialize_for_new_map(
	void)
{
	campaign_level_data->valid= TRUE;
	data_delete_all(campaign_level_data);
	multiplayer_level_data->valid= TRUE;
	data_delete_all(multiplayer_level_data);
	unknown_0019c120();
	unknown_0019c6d0(campaign_level_data, 1);
	unknown_0019c7e0(multiplayer_level_data);
}

/* 0x0019bcd0: the levels make their two tables, in memory of the heap
(data_new, which the game has written here) */
void levels_initialize(
	void)
{
	campaign_level_data= data_new("level handles", 0x14, 8, 0, heap_allocation);
	multiplayer_level_data= data_new("multiplayer level handles", 0x32, 8, 0, heap_allocation);
}

enum
{
	/* in the globals tag: the block of what it says of the levels, which
	has one thing. In that: the block of the levels of the campaign, each of
	0xB50 bytes with its number at 0x4, and the block of those of the games
	of several, each of 0xC64 bytes with its number first */
	GAME_GLOBALS_LEVELS_OFFSET= 0x178,
	LEVELS_CAMPAIGN_OFFSET= 0x8,
	LEVELS_CAMPAIGN_LEVEL_SIZE= 0xB50,
	LEVELS_CAMPAIGN_LEVEL_NUMBER_OFFSET= 0x4,
	LEVELS_MULTIPLAYER_OFFSET= 0x10,
	LEVELS_MULTIPLAYER_LEVEL_SIZE= 0xC64
};

/* a block of a tag: how many things, and where they are */
struct levels_tag_block
{
	long count;
	byte *address;
};

/* what the globals tag says of the levels, or nothing */
static inline __attribute__((always_inline)) const byte *levels_get_definition(
	void)
{
	const struct levels_tag_block *block=
		(const struct levels_tag_block *)((const byte *)global_game_globals+GAME_GLOBALS_LEVELS_OFFSET);

	return block->count ? block->address : NULL;
}

/* one of the things of a block of that; the game has the check of the block
written with it, and with no block it gives a place from nothing */
static inline __attribute__((always_inline)) const byte *levels_get_element(
	long block_offset,
	long index,
	long size)
{
	const byte *definition= levels_get_definition();
	const byte *elements= NULL;

	if (definition && ((const struct levels_tag_block *)(definition+block_offset))->count>0)
	{
		elements= ((const struct levels_tag_block *)(definition+block_offset))->address;
	}

	return elements+index*size;
}

/* 0x0019c1f0: what the globals tag says of a level of the campaign, by its
number; NULL if it says nothing of the levels, or of that one */
const byte *__attribute__((stdcall)) levels_get_campaign_level(
	long number)
{
	const byte *definition= levels_get_definition();
	long count;
	long index;

	if (!definition)
	{
		return NULL;
	}
	count= ((const struct levels_tag_block *)(definition+LEVELS_CAMPAIGN_OFFSET))->count;
	for (index= 0; index<count; index++)
	{
		const byte *level= levels_get_element(LEVELS_CAMPAIGN_OFFSET, index, LEVELS_CAMPAIGN_LEVEL_SIZE);

		if (*(const long *)(level+LEVELS_CAMPAIGN_LEVEL_NUMBER_OFFSET)==number)
		{
			return level;
		}
	}

	return NULL;
}

/* 0x0019c5f0: the same of a level of the games of several */
const byte *__attribute__((stdcall)) levels_get_multiplayer_level(
	long number)
{
	const byte *definition= levels_get_definition();
	long count;
	long index;

	if (!definition)
	{
		return NULL;
	}
	count= ((const struct levels_tag_block *)(definition+LEVELS_MULTIPLAYER_OFFSET))->count;
	for (index= 0; index<count; index++)
	{
		const byte *level= levels_get_element(LEVELS_MULTIPLAYER_OFFSET, index, LEVELS_MULTIPLAYER_LEVEL_SIZE);

		if (*(const long *)level==number)
		{
			return level;
		}
	}

	return NULL;
}

/* 0x0019bd50: the levels give their two tables back (data_dispose, which the
game has written here), each if it is there, and have none */
void levels_dispose(
	void)
{
	if (multiplayer_level_data)
	{
		data_dispose(multiplayer_level_data);
		multiplayer_level_data= NULL;
	}
	if (campaign_level_data)
	{
		data_dispose(campaign_level_data);
		campaign_level_data= NULL;
	}
}

/* 0x0019c670: the table of the levels of the campaign, or NULL if the globals
tag says nothing of the levels */
struct data_array *levels_get_campaign_level_data(
	void)
{
	if (levels_get_definition())
	{
		return campaign_level_data;
	}

	return NULL;
}

/* 0x0019c6a0: the same for the table of the multiplayer levels */
struct data_array *levels_get_multiplayer_level_data(
	void)
{
	if (levels_get_definition())
	{
		return multiplayer_level_data;
	}

	return NULL;
}

/* ---------- more of the levels */

#include "memory/crc.h"

enum
{
	/* something of 0x2D3FC bytes that starts with its version, 2, and the
	checksum of all of it with the checksum as 0 */
	LEVELS_UNKNOWN_0019BDC0_SIZE= 0x2D3FC,
	LEVELS_UNKNOWN_0019BDC0_VERSION= 2,
	LEVELS_UNKNOWN_0019BDC0_CHECKSUM_OFFSET= 0x4,
	/* of a level of the games of several: a number that they are put in
	order by */
	LEVELS_MULTIPLAYER_LEVEL_UNKNOWNC4C_OFFSET= 0xC4C
};

/* 0x0019bdc0: whether something of 0x2D3FC bytes is of the second version and
its checksum (crc_checksum_buffer, from CRC_NEW, with the checksum as 0) is
the one that it has; the checksum is put back */
boolean levels_unknown_0019bdc0_valid(
	byte *data)
{
	unsigned long checksum;
	unsigned long crc;

	if (*(const long *)data!=LEVELS_UNKNOWN_0019BDC0_VERSION)
	{
		return FALSE;
	}
	checksum= *(const unsigned long *)(data+LEVELS_UNKNOWN_0019BDC0_CHECKSUM_OFFSET);
	*(unsigned long *)(data+LEVELS_UNKNOWN_0019BDC0_CHECKSUM_OFFSET)= 0;
	crc= CRC_NEW;
	crc_checksum_buffer(data, LEVELS_UNKNOWN_0019BDC0_SIZE, &crc);
	*(unsigned long *)(data+LEVELS_UNKNOWN_0019BDC0_CHECKSUM_OFFSET)= checksum;

	return checksum==crc;
}

/* 0x0019c580: the lowest of the numbers of the levels of the games of several
that are not NONE; NONE if there is none, or if the globals tag says nothing
of the levels */
long levels_get_lowest_multiplayer_level_number(
	void)
{
	const byte *definition= levels_get_definition();
	long lowest= NONE;
	long count;
	long index;

	if (!definition)
	{
		return NONE;
	}
	count= ((const struct levels_tag_block *)(definition+LEVELS_MULTIPLAYER_OFFSET))->count;
	for (index= 0; index<count; index++)
	{
		long number= *(const long *)levels_get_element(LEVELS_MULTIPLAYER_OFFSET, index, LEVELS_MULTIPLAYER_LEVEL_SIZE);

		if (number!=NONE && (lowest==NONE || number<lowest))
		{
			lowest= number;
		}
	}

	return lowest;
}

/* 0x0019c9a0: for putting levels of the games of several in order, by their
numbers: 1, 0 or -1 as the number of 0xC4C of the first (by
levels_get_multiplayer_level) is more than, the same as or less than that of
the second. The game does not look at whether there are levels */
long levels_multiplayer_level_compare(
	const long *number0,
	const long *number1)
{
	const byte *level0= levels_get_multiplayer_level(*number0);
	const byte *level1= levels_get_multiplayer_level(*number1);
	long value0= *(const long *)(level0+LEVELS_MULTIPLAYER_LEVEL_UNKNOWNC4C_OFFSET);
	long value1= *(const long *)(level1+LEVELS_MULTIPLAYER_LEVEL_UNKNOWNC4C_OFFSET);

	if (value0>value1)
	{
		return 1;
	}

	return value0>=value1 ? 0 : -1;
}

/* ---------- two more near the levels */

#include <string.h>

/* 0x0019d650 (game.c): sets up a part of the options of a multiplayer game */
void unknown_0019d650(void *unknown);

enum
{
	/* something that 0x0019b70b looks at: its kind first, and a number at 8
	that it cares about for the fifth kind */
	LEVELS_UNKNOWN_0019B70B_KIND= 5,
	LEVELS_UNKNOWN_0019B70B_NUMBER_OFFSET= 0x8,
	/* the size of what 0x0019d620 copies: the options of a game of
	several (the variant), most likely */
	LEVELS_UNKNOWN_0019D620_SIZE= 0x130
};

/* 0x0019b70b: whether something is of the fifth kind, with 1, 3 or 13 for
its number of 8 */
boolean __attribute__((stdcall)) levels_unknown_0019b70b(
	const long *unknown)
{
	long number;

	if (*unknown!=LEVELS_UNKNOWN_0019B70B_KIND)
	{
		return FALSE;
	}
	number= *(const long *)((const byte *)unknown+LEVELS_UNKNOWN_0019B70B_NUMBER_OFFSET);

	return number==1 || number==3 || number==13;
}

/* 0x0019d620: unknown_0019d650 with a copy of 0x130 bytes, which is not
kept */
void levels_unknown_0019d620(
	const byte *unknown)
{
	byte copy[LEVELS_UNKNOWN_0019D620_SIZE];

	memcpy(copy, unknown, sizeof(copy));
	unknown_0019d650(copy);
}
