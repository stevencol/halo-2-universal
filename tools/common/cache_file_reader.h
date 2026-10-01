/*
CACHE_FILE_READER.H

Opens a cache file for the tools: the header, the tag index, the tag data
and the tag names, in memory.
*/

#ifndef __CACHE_FILE_READER_H
#define __CACHE_FILE_READER_H

#include "cache/cache_files.h"

#include <stdio.h>

/* ---------- structures */

struct cache_file
{
	FILE *file;
	struct cache_file_header header;

	/* the tag index, as it is in the file, and its address in the console's
	memory */
	uint8_t *tag_index;
	uint32_t base_address;
	struct cache_file_tag_header *tag_header;
	struct cache_file_tag_group *tag_groups;
	struct cache_file_tag_instance *tag_instances;

	/* the tag data and its address in the console's memory */
	uint8_t *tag_data;
	uint32_t tag_data_address;

	char *tag_name_data;
	uint32_t *tag_name_index;
	char *string_id_data;
	uint32_t *string_id_index;
};

/* ---------- prototypes */

/* prints the message and ends the program */
void fail(const char *format, ...);

/* these end the program if the file is not as expected */
void cache_file_open(struct cache_file *cache, const char *path);
void *cache_file_read(struct cache_file *cache, uint32_t offset, size_t size);

void cache_file_close(struct cache_file *cache);

/* the tag data at an address, or NULL if it is not all in the tag data */
void *cache_file_tag_data(struct cache_file *cache, uint32_t address, size_t size);

/* takes a tag index or the index of a tag reference */
const char *cache_file_tag_name(struct cache_file *cache, uint32_t tag_index);
const char *cache_file_string_id(struct cache_file *cache, long string_index);

/* string has 5 characters */
const char *group_tag_to_string(uint32_t group_tag, char *string);

#endif
