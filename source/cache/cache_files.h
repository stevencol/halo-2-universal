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
	uint8_t unknown20[0x100];

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

/* ---------- prototypes */

int32_t cache_file_maximum_length(long scenario_type);
bool cache_file_build_verify(const char *build);
bool cache_file_header_verify(const struct cache_file_header *header);

#endif
