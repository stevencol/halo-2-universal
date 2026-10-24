/*
TAG_UNKNOWN_0016D180.C

finding things by their names (they look like string ids) in the blocks of
some kind of tag. Worked out from halo2ship.exe; what the tag is is not
known, and the name of the file is only the address of its first function.
*/

#include "cseries/cseries.h"

#include <stdint.h>

#include "cache/cache_files.h"

/* ---------- code */

/* the address of a tag */
#define TAG_ADDRESS(tag_index) ((const byte *)(uintptr_t)global_tag_instances[(tag_index)&0xFFFF].address)

/* 0x0016d180: which element (0x38 bytes, of the block at 0x50) of a tag starts with a
name; NONE if none, or for NONE. Without a name, 0 if the block has any */
long tag_unknown_0016d180(
	long tag_index,
	long name)
{
	const byte *tag;
	long index;

	if (tag_index==NONE)
	{
		return NONE;
	}
	tag= TAG_ADDRESS(tag_index);
	if (!name)
	{
		return *(const long *)(tag+0x50)>0 ? 0 : NONE;
	}
	for (index= 0; index<*(const long *)(tag+0x50); index++)
	{
		if (*(const long *)(*(const byte *const *)(tag+0x54)+index*0x38)==name)
		{
			return index;
		}
	}

	return NONE;
}

/* 0x0016d1d0: the same of the block at 0x70 (0x10 bytes); NONE without a
name */
long tag_unknown_0016d1d0(
	long tag_index,
	long name)
{
	const byte *tag;
	long index;

	if (tag_index==NONE || !name)
	{
		return NONE;
	}
	tag= TAG_ADDRESS(tag_index);
	for (index= 0; index<*(const long *)(tag+0x70); index++)
	{
		if (*(const long *)(*(const byte *const *)(tag+0x74)+index*0x10)==name)
		{
			return index;
		}
	}

	return NONE;
}

/* 0x0016d220: the same of the block at 0x08 (8 bytes) of one of those */
long tag_unknown_0016d220(
	long element,
	long tag_index,
	long name)
{
	const byte *block;
	long index;

	if (tag_index==NONE || element==NONE || !name)
	{
		return NONE;
	}
	block= *(const byte *const *)(TAG_ADDRESS(tag_index)+0x74)+element*0x10;
	for (index= 0; index<*(const long *)(block+0x08); index++)
	{
		if (*(const long *)(*(const byte *const *)(block+0x0C)+index*8)==name)
		{
			return index;
		}
	}

	return NONE;
}

/* 0x0016d890: which of the block at 0x70 has a byte at 0x05 */
long tag_unknown_0016d890(
	long tag_index,
	long value)
{
	const byte *tag= TAG_ADDRESS(tag_index);
	long index;

	for (index= 0; index<*(const long *)(tag+0x70); index++)
	{
		if ((*(const signed char *const *)(tag+0x74))[index*0x10+5]==value)
		{
			return index;
		}
	}

	return NONE;
}

/* 0x0016da90: the same as 0x0016d180 of the block at 0x78 (0x5C bytes) */
long tag_unknown_0016da90(
	long tag_index,
	long name)
{
	const byte *tag;
	long index;

	if (tag_index==NONE)
	{
		return NONE;
	}
	tag= TAG_ADDRESS(tag_index);
	for (index= 0; index<*(const long *)(tag+0x78); index++)
	{
		if (*(const long *)(*(const byte *const *)(tag+0x7C)+index*0x5C)==name)
		{
			return index;
		}
	}

	return NONE;
}
