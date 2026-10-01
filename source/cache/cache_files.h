/*
CACHE_FILES.H

The cache files (maps) of Halo 2 for Xbox, build 02.09.27.09809.

The fields with names were worked out from the executable (halo2ship.exe,
the functions at 0x00122870 and 0x001228f0) and from the maps themselves.
The fields called unknown are not understood yet.
*/

#ifndef __CACHE_FILES_H
#define __CACHE_FILES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* ---------- constants */

#define CACHE_FILE_HEADER_SIGNATURE 'head'
#define CACHE_FILE_FOOTER_SIGNATURE 'foot'
#define CACHE_FILE_TAG_HEADER_SIGNATURE 'tags'

enum
{
	CACHE_FILE_VERSION= 8,
	CACHE_FILE_NAME_LENGTH= 32,
	CACHE_FILE_STRING_ID_LENGTH= 128
};

enum
{
	_scenario_type_solo,
	_scenario_type_multiplayer,
	_scenario_type_main_menu,
	_scenario_type_shared,
	_scenario_type_single_player_shared,
	NUMBER_OF_SCENARIO_TYPES
};

/* ---------- structures */

struct cache_file_time
{
	uint32_t low;
	uint32_t high;
};

struct cache_file_header
{
	uint32_t header_signature;
	int32_t version;
	int32_t file_length;
	uint32_t unknownC;

	/* the tag index, then the tag data, are one run of the file that the
	game reads to one block of memory */
	uint32_t tag_data_offset;
	uint32_t tag_index_size;
	uint32_t tag_data_size;
	/* the memory the game allocates for that block: the tag index at its
	start, the tag data at its end, and space for a structure bsp between */
	uint32_t tag_memory_size;
	/* empty in the file: the game keeps here the path of the file that it
	read the header from */
	char path[256];

	char build[32];
	int16_t scenario_type;
	uint16_t unknown142;
	uint32_t unknown144;
	uint32_t unknown148;
	uint8_t unknown14C[8];
	uint32_t unknown154_count;
	uint32_t unknown158_offset;
	uint32_t unknown15C_size;

	/* string ids: the same strings twice, as fixed 128-byte entries and as
	packed text with an index of offsets */
	uint32_t string_id_buffer_offset;
	int32_t string_id_count;
	uint32_t string_id_data_size;
	uint32_t string_id_index_offset;
	uint32_t string_id_data_offset;
	uint8_t unknown174[4];

	/* FILETIMEs; the first is this file's, the others are those of the
	shared maps it takes resources from */
	struct cache_file_time times[4];

	char name[CACHE_FILE_NAME_LENGTH];
	uint32_t unknown1B8;
	char scenario_path[256];
	uint32_t unknown2BC;

	/* the tag names: packed text with an index of offsets, one per tag */
	int32_t tag_name_count;
	uint32_t tag_name_data_offset;
	uint32_t tag_name_data_size;
	uint32_t tag_name_index_offset;
	uint32_t checksum;
	uint8_t unknown2D4[0x528];

	uint32_t footer_signature;
};

/* at tag_data_offset; the addresses are those the block has in the console's
memory, and the tag groups follow this header */
struct cache_file_tag_header
{
	uint32_t tag_groups;
	int32_t tag_group_count;
	uint32_t tag_instances;
	uint32_t scenario_index;
	uint32_t globals_index;
	uint32_t checksum;
	int32_t tag_count;
	uint32_t signature;
};

struct cache_file_tag_group
{
	uint32_t group_tag;
	uint32_t parent_group_tags[2];
};

struct cache_file_tag_instance
{
	uint32_t group_tag;
	uint32_t tag_index;
	uint32_t address;
	uint32_t size;
};

typedef char verify_cache_file_header_size[
	sizeof(struct cache_file_header) == 0x800 ? 1 : -1];
typedef char verify_cache_file_header_build_offset[
	offsetof(struct cache_file_header, build) == 0x120 ? 1 : -1];
typedef char verify_cache_file_header_name_offset[
	offsetof(struct cache_file_header, name) == 0x198 ? 1 : -1];
typedef char verify_cache_file_header_tag_name_count_offset[
	offsetof(struct cache_file_header, tag_name_count) == 0x2C0 ? 1 : -1];
typedef char verify_cache_file_tag_header_size[
	sizeof(struct cache_file_tag_header) == 0x20 ? 1 : -1];
typedef char verify_cache_file_tag_group_size[
	sizeof(struct cache_file_tag_group) == 0xC ? 1 : -1];
typedef char verify_cache_file_tag_instance_size[
	sizeof(struct cache_file_tag_instance) == 0x10 ? 1 : -1];

/* what the game keeps of the cache file that is loaded. As in Halo 1, with
the address of the tags' memory added */
struct cache_file_globals
{
	uint8_t tags_loaded;
	uint8_t pad1[3];
	/* the block that the tag index and the tag data are read to */
	void *tag_memory;
	struct cache_file_header header;
	struct cache_file_tag_header *tag_header;
	struct cache_file_structure_bsp_header *structure_bsp_header;
};

/* the maps that the game has ready to read, with the header of each */
enum
{
	MAXIMUM_CACHED_MAP_FILES= 6
};

struct cached_map_file
{
	/* the handle of the file */
	uint32_t file;
	struct cache_file_header header;
};

struct cached_map_file_globals
{
	struct cached_map_file files[MAXIMUM_CACHED_MAP_FILES];
	/* the map whose tags are loaded; NONE if there is none */
	long open_map_index;
	uint8_t unknown301C[0x840];
};

typedef char verify_cached_map_file_size[
	sizeof(struct cached_map_file) == 0x804 ? 1 : -1];
typedef char verify_cached_map_file_globals_open_map_index_offset[
	offsetof(struct cached_map_file_globals, open_map_index) == 0x3018 ? 1 : -1];

struct cache_file_buffer_globals
{
	uint8_t allocated;
	uint8_t pad1[3];
	void *buffer;
	uint32_t size;
};

/* goes through the tags of a group, or through all of them */
struct tag_iterator
{
	uint32_t unknown0;
	uint32_t unknown4;
	/* of the tag that tag_iterator_next returned */
	int32_t tag_index;
	/* the place in the tag instances of the next one to look at; 0 to
	start */
	int32_t next_tag_instance_index;
	/* NONE for the tags of every group */
	uint32_t group_tag;
};

#if defined(__i386__)
typedef char verify_cache_file_globals_size[
	sizeof(struct cache_file_globals) == 0x810 ? 1 : -1];
typedef char verify_cache_file_globals_tag_header_offset[
	offsetof(struct cache_file_globals, tag_header) == 0x808 ? 1 : -1];
#endif
typedef char verify_tag_iterator_size[
	sizeof(struct tag_iterator) == 0x14 ? 1 : -1];

/* ---------- prototypes */

int32_t cache_file_maximum_length(long scenario_type);
bool cache_file_build_verify(const char *build);
bool cache_file_header_verify(const struct cache_file_header *header);

struct scenario_structure_bsp_reference;

void *cache_file_buffer_allocate(uint32_t size1, uint32_t size2);
void cache_file_get_map_name(const char *path, char *name);
void cache_file_get_map_path(const char *scenario_name, char *path);
long cached_map_files_find_map(const char *scenario_name);
uint8_t cache_file_read_header(struct cache_file_header *header, const char *scenario_name);
void cache_file_read_async(uint32_t offset, long size, void *buffer, long *bytes_read, uint8_t *done,
	long unknown1, long unknown2);
void cache_file_block_until_not_busy(void);
uint8_t scenario_load(const char *scenario_name);
void scenario_tags_unload(void);
uint8_t cache_file_blocking_read(long unknown, uint32_t offset, void *buffer, long size);
uint8_t cache_file_structure_bsp_load(const struct scenario_structure_bsp_reference *reference);
struct cache_file_tag_group *cache_file_tag_group_get(uint32_t group_tag);
void *tag_get(uint32_t group_tag, long tag_index);
long tag_iterator_next(struct tag_iterator *iterator);

/* ---------- globals */

extern struct cache_file_globals cache_file_globals;
extern struct cached_map_file_globals cached_map_file_globals;
extern struct cache_file_buffer_globals cache_file_buffer_globals;
extern struct cache_file_tag_instance *global_tag_instances;

#endif
