/*
BITMAP_GROUP.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.
*/

#include "bitmap_definitions.h"

#include "cache/cache_files.h"

/* ---------- code */

/* 0x00137550: a bitmap of a bitmap group; NULL if the group has no data or
has no such bitmap. The index of the tag is not examined. The name is Halo
1's, which takes the group and not its index */
struct bitmap_data *bitmap_group_get_bitmap(
	long bitmap_group_index,
	short bitmap_index)
{
	struct bitmap_group *group=
		(struct bitmap_group *)(uintptr_t)global_tag_instances[bitmap_group_index&0xFFFF].address;

	if (group && bitmap_index>=0 && bitmap_index<group->bitmaps.count)
	{
		return &((struct bitmap_data *)(uintptr_t)group->bitmaps.address)[bitmap_index];
	}

	return NULL;
}
