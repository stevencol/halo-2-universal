/*
CACHE_FILE_READER.C
*/

#include "cache_file_reader.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/* ---------- code */

void fail(
	const char *format,
	...)
{
	va_list arguments;

	va_start(arguments, format);
	vfprintf(stderr, format, arguments);
	va_end(arguments);
	fputc('\n', stderr);

	exit(1);
}

void *cache_file_read(
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

void cache_file_open(
	struct cache_file *cache,
	const char *path)
{
	struct cache_file_header *header= &cache->header;

	memset(cache, 0, sizeof(*cache));
	cache->file= fopen(path, "rb");
	if (!cache->file)
	{
		fail("cannot open '%s'", path);
	}
	if (fread(header, 1, sizeof(*header), cache->file)!=sizeof(*header) ||
		!cache_file_header_verify(header))
	{
		fail("'%s' does not appear to be a Halo 2 cache file", path);
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

	/* the tag data follows the tag index in the file; in memory it is at the
	end of the block, after the space for the structure bsp */
	cache->tag_data= cache_file_read(cache, header->tag_data_offset+header->tag_index_size, header->tag_data_size);
	cache->tag_data_address= cache->base_address+header->tag_memory_size-header->tag_data_size;

	cache->tag_name_data= cache_file_read(cache, header->tag_name_data_offset, header->tag_name_data_size);
	cache->tag_name_index= cache_file_read(cache, header->tag_name_index_offset,
		header->tag_name_count*sizeof(uint32_t));
	cache->string_id_data= cache_file_read(cache, header->string_id_data_offset, header->string_id_data_size);
	cache->string_id_index= cache_file_read(cache, header->string_id_index_offset,
		header->string_id_count*sizeof(uint32_t));
}

void cache_file_close(
	struct cache_file *cache)
{
	free(cache->tag_index);
	free(cache->tag_data);
	free(cache->tag_name_data);
	free(cache->tag_name_index);
	free(cache->string_id_data);
	free(cache->string_id_index);
	fclose(cache->file);
	memset(cache, 0, sizeof(*cache));
}

void *cache_file_tag_data(
	struct cache_file *cache,
	uint32_t address,
	size_t size)
{
	uint32_t offset= address-cache->tag_data_address;

	return (address>=cache->tag_data_address && offset+size<=cache->header.tag_data_size) ?
		cache->tag_data+offset :
		NULL;
}

const char *cache_file_tag_name(
	struct cache_file *cache,
	uint32_t tag_index)
{
	uint32_t offset;

	if (tag_index==0xFFFFFFFF || (tag_index&0xFFFF)>=(uint32_t)cache->header.tag_name_count)
	{
		return "<none>";
	}
	offset= cache->tag_name_index[tag_index&0xFFFF];

	return offset<cache->header.tag_name_data_size ? cache->tag_name_data+offset : "<bad name>";
}

const char *cache_file_string_id(
	struct cache_file *cache,
	long string_index)
{
	uint32_t offset;

	if (string_index<0 || string_index>=cache->header.string_id_count)
	{
		return "<none>";
	}
	offset= cache->string_id_index[string_index];

	return offset<cache->header.string_id_data_size ? cache->string_id_data+offset : "<bad string>";
}

const char *group_tag_to_string(
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
