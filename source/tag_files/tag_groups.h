/*
TAG_GROUPS.H

The parts of a tag as a cache file holds them. The addresses are those the
data has in the console's memory.
*/

#ifndef __TAG_GROUPS_H
#define __TAG_GROUPS_H

#include <stdint.h>

/* ---------- constants */

#define NONE -1

/* ---------- structures */

struct tag_block
{
	int32_t count;
	uint32_t address;
};

struct tag_reference
{
	uint32_t group_tag;
	uint32_t index;
};

typedef char verify_tag_block_size[
	sizeof(struct tag_block) == 0x8 ? 1 : -1];
typedef char verify_tag_reference_size[
	sizeof(struct tag_reference) == 0x8 ? 1 : -1];

#endif
