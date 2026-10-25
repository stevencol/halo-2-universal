/*
GAME_UNKNOWN_00134D90.C

a part of the game: 0x204 bytes of the state of the game, that are zeros in a new map.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "cseries/cseries.h"

#include <string.h>

#include "saved games/game_state.h"
#include "scenario/scenario.h"
#include "cache/cache_files.h"

/* ---------- constants */

enum
{
	GAME_UNKNOWN_00134D90_SIZE= 0x204
};

/* ---------- globals */

/* 0x004e6740: the 0x204 bytes; NULL when the part is closed */
byte *unknown_004e6740;

/* ---------- code */

/* 0x00134d90: the bytes take their memory in the state of the game
(game_state_malloc, which the game has written here) */
void game_unknown_00134d90_initialize(
	void)
{
	unknown_004e6740= game_state_malloc(GAME_UNKNOWN_00134D90_SIZE);
}

/* 0x00134dd0: the part is closed: there are no bytes */
void game_unknown_00134d90_dispose(
	void)
{
	unknown_004e6740= NULL;
}

/* 0x00134de0: for a new map the bytes are zeros, if they are there */
void game_unknown_00134d90_initialize_for_new_map(
	void)
{
	if (unknown_004e6740)
	{
		memset(unknown_004e6740, 0, GAME_UNKNOWN_00134D90_SIZE);
	}
}

/* 0x00135750: the bytes have one thing of 0x20 bytes for each of what the
scenario counts at 0x3C0, and each loses the bit 0 of its byte at 0x20
(after the first number of the bytes, its byte at 0x1C). Nothing if the part
is closed or there is no scenario */
void game_unknown_00134d90_clear_bits(
	void)
{
	byte *unknown= unknown_004e6740;

	if (unknown && global_scenario)
	{
		long index;

		for (index= 0; index<*(const long *)((const byte *)global_scenario+0x3C0); index++)
		{
			unknown[0x20+index*0x20]&= ~1;
		}
	}
}

/* 0x00134d20: the entry (0x20 bytes, after the first long) of the 0x204
bytes for one of the entries (0x18 bytes, of the block at 0x3C0) of the
scenario that starts with a number, and which it is; the number is kept in
the first long. Nothing and NONE when it is not there */
byte *game_unknown_00134d90_find(
	long *index,
	long number)
{
	byte *data= unknown_004e6740;

	if (data && global_scenario && number)
	{
		const long *entries= *(const long *const *)((const byte *)global_scenario+0x3C4);
		long count= *(const long *)((const byte *)global_scenario+0x3C0);
		long which;

		for (which= 0; which<count; which++)
		{
			if (entries[which*6] && entries[which*6]==number)
			{
				*(long *)data= number;
				if (index)
				{
					*index= which;
				}
				return data+which*0x20+4;
			}
		}
	}
	if (index)
	{
		*index= NONE;
	}

	return NULL;
}

/* 0x00137650: a count of a tag: 1 with the bit 0x04 of its byte at 0x04,
else the long at 0x04 of the element (0x0C bytes, at 0x38) that its byte
0x3C+which names plus its long at 0x48; and the byte at 0x25 of the element
(0x5C bytes, at 0x28) of each of so many bytes (as many as at 0x1C; 0xFF
for none) */
long tag_unknown_00137650(
	long tag_index,
	long which,
	const byte *elements)
{
	const byte *tag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;
	long count;
	long index;

	if (tag[0x04]&4)
	{
		count= 1;
	}
	else
	{
		count= *(const long *)(*(const byte *const *)(tag+0x38)+(signed char)tag[which+0x3C]*0x0C+0x04)
			+ *(const long *)(tag+0x48);
	}
	for (index= 0; index<*(const long *)(tag+0x1C); index++)
	{
		if (elements[index]!=0xFF)
		{
			count+= (*(const byte *const *)(tag+0x28))[elements[index]*0x5C+0x25];
		}
	}

	return count;
}

/* 0x00137590: a number from the element (0x3C bytes, of the block at 0x3C) of
a tag that a number names (around): a value around its short at 0x22 plus
its short at 0x20, or the short of the value's entry (0x20 bytes) of its
block at 0x34; the value itself when that is NONE or there is no block. 0
for a tag without elements, NONE for NONE or a tag that is not loaded */
long __attribute__((stdcall)) tag_unknown_00137590(
	long tag_index,
	short which,
	short value)
{
	const byte *tag;
	const byte *element;
	long count;
	long result;

	if (tag_index==NONE)
	{
		return NONE;
	}
	tag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;
	if (!tag)
	{
		return NONE;
	}
	count= *(const long *)(tag+0x3C);
	if (count<=0)
	{
		return 0;
	}
	element= *(const byte *const *)(tag+0x40)+(which%count)*0x3C;
	if (*(const short *)(element+0x22)>0)
	{
		result= value%*(const short *)(element+0x22)+*(const short *)(element+0x20);
	}
	else
	{
		if (!*(const long *)(element+0x34))
		{
			return value;
		}
		result= *(const short *)(*(const byte *const *)(element+0x38)+value*0x20);
	}

	return result==NONE ? value : result;
}

/* 0x00137610: the bitmap (0x74 bytes, of those at 0x48 of a bitmap group,
as many as at 0x44) that tag_unknown_00137590 picks; NULL if none */
void *game_unknown_00137610(
	long tag_index,
	short which,
	short value)
{
	long index= tag_unknown_00137590(tag_index, which, value);
	const byte *group;

	if (index==NONE)
	{
		return NULL;
	}
	group= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;
	if (index<0 || index>=*(const long *)(group+0x44))
	{
		return NULL;
	}

	return *(byte *const *)(group+0x48)+index*0x74;
}

/* 0x0013b390 and 0x0013bb90, of the functions of tags */
real unknown_0013b390(const byte *function, real input, real range);
real __attribute__((stdcall)) unknown_0013bb90(const byte *function, real input, real range);

/* 0x00134fe0: the value of a ramp of the scenario (24 bytes each, at 0x3C4,
as many as at 0x3C0) at a value, plus what it keeps of its own at 0x18 (of
its 32 bytes from 0x04 of unknown_004e6740): by its function at 0x0C; one
scaled by another ramp (the short at 0x16) has it times the value of that
one, which has to have no scale itself (0 if it has), and then taken to 0..1
and to its bounds, unless the function says not to */
real __attribute__((stdcall)) game_unknown_00134fe0(
	long index,
	real value)
{
	const byte *state= unknown_004e6740+index*32+4;
	const byte *scenario= (const byte *)global_scenario;
	const byte *ramps= *(const byte *const *)(scenario+0x3C4);
	const byte *ramp= ramps+index*24;
	short scale_index= *(const short *)(ramp+0x16);
	long clamped;
	real output;
	real result;
	const byte *data;

	if (!state)
	{
		return 0.0f;
	}
	value+= *(const real *)(state+0x18);
	clamped= scale_index<0 ? 0 : scale_index>*(const long *)(scenario+0x3C0)-1 ? *(const long *)(scenario+0x3C0)-1 : scale_index;
	if (clamped!=scale_index)
	{
		return unknown_0013bb90(ramp+0x0C, value, 0.0f);
	}
	if (*(const short *)(ramps+scale_index*24+0x16)!=NONE)
	{
		return 0.0f;
	}
	output= unknown_0013b390(ramp+0x0C, value, 0.0f);
	result= 0.0f;
	if (unknown_004e6740+scale_index*32+4)
	{
		result= game_unknown_00134fe0(scale_index, *(const real *)(unknown_004e6740+scale_index*32+4));
	}
	result*= output;
	data= *(const byte *const *)(ramp+0x0C+4);
	if (data[1]&0xF0)
	{
		return result;
	}
	if (0.0f>result)
	{
		result= 0.0f;
	}
	else if (result>1.0f)
	{
		result= 1.0f;
	}

	return (*(const real *)(data+8)-*(const real *)(data+4))*result + *(const real *)(data+4);
}

/* 0x00134fc0: the value of a ramp of the scenario at what it keeps at 0x00
of its own */
real game_unknown_00134fc0(
	long index)
{
	const byte *state= unknown_004e6740+index*32+4;

	if (!state)
	{
		return 0.0f;
	}

	return game_unknown_00134fe0(index, *(const real *)state);
}
