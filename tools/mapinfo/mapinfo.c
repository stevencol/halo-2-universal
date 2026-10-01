/*
MAPINFO.C

Opens a Halo 2 cache file as the game does and lists what it holds: the
header, the tag groups and the tags.

mapinfo <map> [--tags]
*/

#include "cache_file_reader.h"

#include <stdlib.h>
#include <string.h>

/* ---------- structures */

struct group_total
{
	uint32_t group_tag;
	long count;
	long external_count;
	unsigned long long size;
};

/* ---------- code */

static int compare_group_totals(
	const void *a,
	const void *b)
{
	const struct group_total *total_a= a;
	const struct group_total *total_b= b;

	return total_a->size<total_b->size ? 1 : total_a->size>total_b->size ? -1 : 0;
}

static void print_header(
	struct cache_file *cache)
{
	static const char *const scenario_type_names[NUMBER_OF_SCENARIO_TYPES]=
	{
		"solo", "multiplayer", "main menu", "shared", "single player shared"
	};
	struct cache_file_header *header= &cache->header;
	struct cache_file_tag_header *tag_header= cache->tag_header;

	printf("name            %s\n", header->name);
	printf("scenario        %s\n", header->scenario_path);
	printf("type            %s\n", scenario_type_names[header->scenario_type]);
	printf("build           %s%s\n", header->build,
		cache_file_build_verify(header->build) ? "" : " (not the build of this executable)");
	printf("version         %d\n", header->version);
	printf("file length     %d\n", header->file_length);
	printf("tag data        offset 0x%X, index 0x%X + data 0x%X, memory 0x%X\n", header->tag_data_offset,
		header->tag_index_size, header->tag_data_size, header->tag_memory_size);
	printf("base address    0x%08X\n", cache->base_address);
	printf("tag groups      %d\n", tag_header->tag_group_count);
	printf("tags            %d\n", tag_header->tag_count);
	printf("string ids      %d\n", header->string_id_count);
	printf("scenario tag    %s\n", cache_file_tag_name(cache, tag_header->scenario_index));
	printf("globals tag     %s\n", cache_file_tag_name(cache, tag_header->globals_index));
}

static void print_tags(
	struct cache_file *cache,
	bool list_tags)
{
	long tag_count= cache->tag_header->tag_count;
	long group_count= cache->tag_header->tag_group_count;
	struct group_total *totals= calloc(group_count, sizeof(struct group_total));
	long local_count= 0, external_count= 0, empty_count= 0, outside_count= 0;
	long tag_index, group_index;
	char string[5];

	for (group_index= 0; group_index<group_count; group_index++)
	{
		totals[group_index].group_tag= cache->tag_groups[group_index].group_tag;
	}

	for (tag_index= 0; tag_index<tag_count; tag_index++)
	{
		struct cache_file_tag_instance *instance= &cache->tag_instances[tag_index];
		bool external= false;

		if (instance->group_tag==0xFFFFFFFF)
		{
			empty_count++;
			continue;
		}
		else if (cache_file_tag_data(cache, instance->address, instance->size))
		{
			local_count++;
		}
		else if (instance->address==0 || instance->size==0)
		{
			/* a structure bsp or its lightmap, loaded apart, or a tag whose
			data is in a shared map */
			external= true;
			external_count++;
		}
		else
		{
			outside_count++;
		}

		for (group_index= 0; group_index<group_count; group_index++)
		{
			if (totals[group_index].group_tag==instance->group_tag)
			{
				totals[group_index].count++;
				totals[group_index].external_count+= external;
				totals[group_index].size+= instance->size;
				break;
			}
		}

		if (list_tags)
		{
			printf("%4s  %08X  %08X  %8u  %s\n", group_tag_to_string(instance->group_tag, string),
				instance->tag_index, instance->address, instance->size, cache_file_tag_name(cache, tag_index));
		}
	}

	printf("\ntags by where their data is\n");
	printf("  in this file    %ld\n", local_count);
	printf("  no data here    %ld\n", external_count);
	printf("  empty slots     %ld\n", empty_count);
	printf("  outside (bad)   %ld\n", outside_count);

	qsort(totals, group_count, sizeof(struct group_total), compare_group_totals);
	printf("\ngroup   tags  no data       bytes\n");
	for (group_index= 0; group_index<group_count; group_index++)
	{
		if (totals[group_index].count)
		{
			printf("%4s  %6ld  %7ld  %10llu\n", group_tag_to_string(totals[group_index].group_tag, string),
				totals[group_index].count, totals[group_index].external_count, totals[group_index].size);
		}
	}

	free(totals);
}

static void print_string_ids(
	struct cache_file *cache)
{
	long count= cache->header.string_id_count;
	long index;

	printf("\nstring ids (the first and the last)\n");
	for (index= 0; index<count; index++)
	{
		if (index<6 || index>=count-4)
		{
			printf("  %5ld  %s\n", index, cache_file_string_id(cache, index));
		}
	}
}

int main(
	int argc,
	char **argv)
{
	struct cache_file cache;
	bool list_tags= argc>2 && strcmp(argv[2], "--tags")==0;

	if (argc<2)
	{
		fail("usage: mapinfo <map> [--tags]");
	}

	cache_file_open(&cache, argv[1]);
	print_header(&cache);
	print_tags(&cache, list_tags);
	print_string_ids(&cache);
	cache_file_close(&cache);

	return 0;
}
