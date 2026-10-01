/*
MAPINFO.C

Opens a Halo 2 cache file as the game does and lists what it holds: the
header, the tag groups and the tags.

mapinfo <map> [--tags]
*/

#include "cache_files.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------- structures */

struct cache_file
{
	FILE *file;
	struct cache_file_header header;

	/* the tag index, as it is in the file */
	uint8_t *tag_index;
	struct cache_file_tag_header *tag_header;
	struct cache_file_tag_group *tag_groups;
	struct cache_file_tag_instance *tag_instances;
	/* the address the tag index has in the console's memory */
	uint32_t base_address;

	char *tag_name_data;
	uint32_t *tag_name_index;
	char *string_id_data;
	uint32_t *string_id_index;
};

struct group_total
{
	uint32_t group_tag;
	long count;
	long external_count;
	unsigned long long size;
};

/* ---------- code */

static void fail(
	const char *message)
{
	fprintf(stderr, "mapinfo: %s\n", message);
	exit(1);
}

static void *cache_file_read(
	struct cache_file *cache,
	uint32_t offset,
	size_t size)
{
	void *buffer= malloc(size ? size : 1);

	if (!buffer ||
		_fseeki64(cache->file, offset, SEEK_SET)!=0 ||
		fread(buffer, 1, size, cache->file)!=size)
	{
		fail("the file is shorter than its header says");
	}

	return buffer;
}

/* a pointer to an address inside the tag index, or NULL */
static void *tag_index_pointer(
	struct cache_file *cache,
	uint32_t address,
	size_t size)
{
	uint32_t offset= address-cache->base_address;

	return (address>=cache->base_address && offset+size<=cache->header.tag_index_size) ?
		cache->tag_index+offset :
		NULL;
}

static const char *group_tag_to_string(
	uint32_t group_tag,
	char *string)
{
	int i;

	for (i= 0; i<4; i++)
	{
		char character= (char)(group_tag>>(24-8*i));
		string[i]= (character>=0x20 && character<0x7F) ? character : '?';
	}
	string[4]= 0;

	return string;
}

static void cache_file_open(
	struct cache_file *cache,
	const char *path)
{
	struct cache_file_header *header= &cache->header;

	cache->file= fopen(path, "rb");
	if (!cache->file)
	{
		fail("cannot open the file");
	}
	if (fread(header, 1, sizeof(*header), cache->file)!=sizeof(*header) ||
		!cache_file_header_verify(header))
	{
		fail("this does not appear to be a Halo 2 cache file");
	}

	cache->tag_index= cache_file_read(cache, header->tag_data_offset, header->tag_index_size);
	cache->tag_header= (struct cache_file_tag_header *)cache->tag_index;
	if (header->tag_index_size<sizeof(struct cache_file_tag_header) ||
		cache->tag_header->signature!=CACHE_FILE_TAG_HEADER_SIGNATURE ||
		cache->tag_header->tag_count<=0)
	{
		fail("the tag header is not valid");
	}

	/* the tag groups follow the header */
	cache->base_address= cache->tag_header->tag_groups-sizeof(struct cache_file_tag_header);
	cache->tag_groups= tag_index_pointer(cache, cache->tag_header->tag_groups,
		cache->tag_header->tag_group_count*sizeof(struct cache_file_tag_group));
	cache->tag_instances= tag_index_pointer(cache, cache->tag_header->tag_instances,
		cache->tag_header->tag_count*sizeof(struct cache_file_tag_instance));
	if (!cache->tag_groups || !cache->tag_instances)
	{
		fail("the tag groups or the tag instances are outside the tag index");
	}

	cache->tag_name_data= cache_file_read(cache, header->tag_name_data_offset, header->tag_name_data_size);
	cache->tag_name_index= cache_file_read(cache, header->tag_name_index_offset,
		header->tag_name_count*sizeof(uint32_t));
	cache->string_id_data= cache_file_read(cache, header->string_id_data_offset, header->string_id_data_size);
	cache->string_id_index= cache_file_read(cache, header->string_id_index_offset,
		header->string_id_count*sizeof(uint32_t));
}

static const char *cache_file_tag_name(
	struct cache_file *cache,
	long tag_index)
{
	uint32_t offset;

	if (tag_index<0 || tag_index>=cache->header.tag_name_count)
	{
		return "<none>";
	}
	offset= cache->tag_name_index[tag_index];

	return offset<cache->header.tag_name_data_size ? cache->tag_name_data+offset : "<bad name>";
}

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
	printf("scenario tag    %s\n", cache_file_tag_name(cache, tag_header->scenario_index&0xFFFF));
	printf("globals tag     %s\n", cache_file_tag_name(cache, tag_header->globals_index&0xFFFF));
}

static void print_tags(
	struct cache_file *cache,
	bool list_tags)
{
	struct cache_file_header *header= &cache->header;
	long tag_count= cache->tag_header->tag_count;
	long group_count= cache->tag_header->tag_group_count;
	/* the tag data is at the end of the block in memory; between the tag
	index and it is the space for the structure bsp, which the game loads
	apart */
	uint32_t data_end= cache->base_address+header->tag_memory_size;
	uint32_t data_start= data_end-header->tag_data_size;
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
		else if (instance->address>=data_start && instance->address+instance->size<=data_end)
		{
			local_count++;
		}
		else if (instance->address==0 || instance->size==0)
		{
			/* the tag's data is in a shared map */
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
			uint32_t offset= cache->string_id_index[index];

			printf("  %5ld  %s\n", index,
				offset<cache->header.string_id_data_size ? cache->string_id_data+offset : "<bad string>");
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

	memset(&cache, 0, sizeof(cache));
	cache_file_open(&cache, argv[1]);
	print_header(&cache);
	print_tags(&cache, list_tags);
	print_string_ids(&cache);

	return 0;
}
