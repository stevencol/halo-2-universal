/*
CACHE_FILES.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.
*/

#include "cache_files.h"

#include "memory/binary_search.h"

#include <string.h>

/* ---------- constants */

#define NONE -1

/* ---------- globals */

/* 0x005476f8 */
struct cache_file_globals cache_file_globals;

/* 0x004e3b44: the tag instances of the tag header */
struct cache_file_tag_instance *global_tag_instances;

/* 0x0043e86c: the builds whose maps the game accepts */
static const char *const cache_file_builds[]=
{
	"02.09.27.09809"
};

/* ---------- code */

/* 0x00214ed0 */
int32_t cache_file_maximum_length(
	long scenario_type)
{
	switch (scenario_type)
	{
	case _scenario_type_solo:
		return 0x11800000;
	case _scenario_type_multiplayer:
	case _scenario_type_main_menu:
		return 0x05000000;
	case _scenario_type_shared:
		return 0x0B400000;
	case _scenario_type_single_player_shared:
		return 0x20800000;
	}

	return 0;
}

/* 0x00012130 */
bool cache_file_build_verify(
	const char *build)
{
	size_t build_index;

	for (build_index= 0; build_index<sizeof(cache_file_builds)/sizeof(cache_file_builds[0]); build_index++)
	{
		if (strcmp(build, cache_file_builds[build_index])==0)
		{
			return true;
		}
	}

	return false;
}

/* 0x00122870 */
bool cache_file_header_verify(
	const struct cache_file_header *header)
{
	return
		header->header_signature==CACHE_FILE_HEADER_SIGNATURE &&
		header->footer_signature==CACHE_FILE_FOOTER_SIGNATURE &&
		header->file_length>=0 &&
		header->file_length<=0x20800000 &&
		header->scenario_type>=0 &&
		header->scenario_type<NUMBER_OF_SCENARIO_TYPES &&
		header->file_length<=cache_file_maximum_length(header->scenario_type) &&
		memchr(header->name, 0, CACHE_FILE_NAME_LENGTH)!=NULL &&
		header->version==CACHE_FILE_VERSION;
}

/* 0x00122cf0: for binary_search; the tag groups are in the order of their
group tags, as signed integers */
static long CALLBACK_CONVENTION cache_file_tag_group_compare(
	const void *key,
	const void *element,
	void *context)
{
	(void)context;

	return (long)(*(const uint32_t *)key - *(const uint32_t *)element);
}

/* 0x00122d00: the tag group of a group tag, with its two parents; NULL if
there is none or no cache file is loaded. The name is a guess */
struct cache_file_tag_group *cache_file_tag_group_get(
	uint32_t group_tag)
{
	struct cache_file_tag_header *tag_header= cache_file_globals.tag_header;
	struct cache_file_tag_group *tag_groups;
	struct cache_file_tag_group key;
	long index;

	if (!tag_header)
	{
		return NULL;
	}

	key.group_tag= group_tag;
	key.parent_group_tags[0]= (uint32_t)NONE;
	key.parent_group_tags[1]= (uint32_t)NONE;
	tag_groups= (struct cache_file_tag_group *)(uintptr_t)tag_header->tag_groups;
	index= binary_search(tag_header->tag_group_count, &key, tag_groups, sizeof(struct cache_file_tag_group),
		cache_file_tag_group_compare, NULL);
	if (index==NONE)
	{
		return NULL;
	}

	/* the game reads the tag header again here */
	return &((struct cache_file_tag_group *)(uintptr_t)cache_file_globals.tag_header->tag_groups)[index];
}

/* 0x00122c10: the data of a tag, if the index is of a tag that is there and
the tag is of the group or of a group that has it as a parent; NULL
otherwise. As in Halo 1 */
void *tag_get(
	uint32_t group_tag,
	long tag_index)
{
	short absolute_index= (short)tag_index;
	struct cache_file_tag_instance *instance= NULL;

	if (absolute_index>=0 && absolute_index<cache_file_globals.tag_header->tag_count)
	{
		instance= &global_tag_instances[absolute_index];
		if ((uint32_t)tag_index!=instance->tag_index)
		{
			instance= NULL;
		}
	}

	if (instance)
	{
		struct cache_file_tag_group *tag_group= cache_file_tag_group_get(instance->group_tag);

		if (tag_group->group_tag==group_tag ||
			tag_group->parent_group_tags[0]==group_tag ||
			tag_group->parent_group_tags[1]==group_tag)
		{
			return (void *)(uintptr_t)instance->address;
		}
	}

	return NULL;
}

/* 0x00122c70: the index of the next tag of the iterator's group; NONE when
there are no more. As in Halo 1 */
long tag_iterator_next(
	struct tag_iterator *iterator)
{
	while (iterator->next_tag_instance_index<cache_file_globals.tag_header->tag_count)
	{
		struct cache_file_tag_instance *instance= &global_tag_instances[iterator->next_tag_instance_index];

		iterator->next_tag_instance_index++;
		if (instance && instance->group_tag!=(uint32_t)NONE && instance->tag_index!=(uint32_t)NONE)
		{
			if (iterator->group_tag!=(uint32_t)NONE)
			{
				struct cache_file_tag_group *tag_group= cache_file_tag_group_get(instance->group_tag);

				if (iterator->group_tag!=tag_group->group_tag &&
					iterator->group_tag!=tag_group->parent_group_tags[0] &&
					iterator->group_tag!=tag_group->parent_group_tags[1])
				{
					continue;
				}
			}
			iterator->tag_index= (int32_t)instance->tag_index;
			return (long)instance->tag_index;
		}
	}

	return NONE;
}
