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

#include "cseries/cseries.h"

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
	/* the same in a map and in its copy on the hard disk, if the copy is
	good */
	uint32_t unknown144;
	uint32_t unknown148;
	/* when the copy of the map on the hard disk was made; empty in the
	map of the disc */
	struct file_time copy_time;
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
	/* whether the map takes resources from each of the shared maps, in
	the order of the three last times */
	uint8_t uses_shared_map[3];
	uint8_t unknown177;

	/* FILETIMEs; the first is this file's, the others are those of the
	shared maps it takes resources from: the main menu's, the shared one
	and the single player shared one */
	struct file_time times[4];

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
	MAXIMUM_CACHED_MAP_FILES= 6,
	MAXIMUM_CACHED_MAP_FILE_FAILURES= 8,
	MAXIMUM_CACHED_MAP_FILE_FAILURE_COUNT= 3
};

/* what cached_map_file_status tells of a map */
enum
{
	_cached_map_file_status_none,
	/* asked for; the copy starts when the one that is being made ends */
	_cached_map_file_status_pending,
	_cached_map_file_status_copying,
	/* the copy is on the hard disk */
	_cached_map_file_status_cached,
	_cached_map_file_status_failed
};

/* the steps of the copy of a map to the hard disk. In those with a name
that ends in a task, the game waits for the task before the next step */
enum
{
	_cached_map_file_copy_idle,
	_cached_map_file_copy_open_source,
	_cached_map_file_copy_check_source,
	_cached_map_file_copy_read_header,
	_cached_map_file_copy_check_header,
	_cached_map_file_copy_get_size,
	_cached_map_file_copy_check_size,
	_cached_map_file_copy_erase_header,
	_cached_map_file_copy_check_erase,
	_cached_map_file_copy_flush_erase,
	_cached_map_file_copy_start_data,
	_cached_map_file_copy_copy_data,
	_cached_map_file_copy_check_data,
	_cached_map_file_copy_flush_data,
	_cached_map_file_copy_write_header,
	_cached_map_file_copy_check_write,
	_cached_map_file_copy_finish,
	_cached_map_file_copy_end,
	_cached_map_file_copy_close_source,
	_cached_map_file_copy_clean_up,
	NUMBER_OF_CACHED_MAP_FILE_COPY_STATES
};

/* how a copy ended */
enum
{
	_cached_map_file_copy_result_none,
	_cached_map_file_copy_result_success,
	_cached_map_file_copy_result_cancelled,
	_cached_map_file_copy_result_failure
};

struct cached_map_file
{
	/* the handle of the file */
	uint32_t file;
	struct cache_file_header header;
};

/* a map that could not be copied to the hard disk */
struct cached_map_file_failure
{
	/* of the map on the disc; empty in a place that is not in use */
	char path[256];
	/* of the last time (GetTickCount) */
	long time;
	/* at three, the game does not try again */
	long count;
};

struct cached_map_file_globals
{
	struct cached_map_file files[MAXIMUM_CACHED_MAP_FILES];
	/* the map whose tags are loaded; NONE if there is none */
	long open_map_index;
	struct cached_map_file_failure failures[MAXIMUM_CACHED_MAP_FILE_FAILURES];
};

typedef char verify_cached_map_file_size[
	sizeof(struct cached_map_file) == 0x804 ? 1 : -1];
typedef char verify_cached_map_file_globals_open_map_index_offset[
	offsetof(struct cached_map_file_globals, open_map_index) == 0x3018 ? 1 : -1];

/* a map that the game was asked to copy to the hard disk */
struct cached_map_file_request
{
	/* the path of the map on the disc */
	char name[256];
	/* from 0 to 6: the copy that is being made is stopped for one with as
	much or more */
	long priority;
};

/* 0x0055b4f0: the copy of a map of the disc to the hard disk, which is
made a step at a time. The name is a guess */
struct cached_map_file_copy_globals
{
	/* of the map that is copied */
	struct cache_file_header header;
	/* what the last task that reads, writes or copies did */
	uint32_t bytes_transferred;
	long bytes_requested;
	/* how much of the map is copied */
	uint32_t offset;
	uint32_t file_size;
	/* of the last task */
	long task_id;
	long state;
	long result;
	/* set when the last task is done */
	uint8_t done;
	uint8_t pad81D[3];
	/* the place in cached_map_file_globals that the copy goes to */
	long destination_index;
	uint32_t source_file;
	/* what is copied goes through here */
	void *buffer;
	long buffer_size;
	/* the game wants the memory of the buffer back */
	uint8_t release_requested;
	char source_path[256];
	uint8_t pad931[3];
	struct cached_map_file_request current;
	struct cached_map_file_request pending;
	/* of what is copied so far */
	uint32_t checksum;
};

typedef char verify_cached_map_file_globals_size[
	sizeof(struct cached_map_file_globals) == 0x385C ? 1 : -1];
#if defined(__i386__)
typedef char verify_cached_map_file_copy_globals_size[
	sizeof(struct cached_map_file_copy_globals) == 0xB40 ? 1 : -1];
typedef char verify_cached_map_file_copy_globals_current_offset[
	offsetof(struct cached_map_file_copy_globals, current) == 0x934 ? 1 : -1];
#endif
typedef char verify_cached_map_file_failure_size[
	sizeof(struct cached_map_file_failure) == 0x108 ? 1 : -1];
typedef char verify_cache_file_header_copy_time_offset[
	offsetof(struct cache_file_header, copy_time) == 0x14C ? 1 : -1];

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

uint32_t cached_map_file_maximum_size(long index);
void cached_map_file_get_path(char *path, unsigned long size, long index);
void cached_map_files_delete(long first_index, long end_index);
struct cached_map_file *cached_map_file_shared_get(long shared_index);
uint8_t cached_map_file_read_header(long index);
void cached_map_files_open(void);
uint32_t cache_file_xor_checksum(const void *buffer, uint32_t size);
uint8_t cache_file_map_paths_equal(const char *name1, const char *name2);
long cached_map_file_shared_index(long shared_index);
uint8_t cached_map_file_is_better_place(long best_index, long index);
long cached_map_file_choose_place(long scenario_type, uint32_t file_length);
struct cached_map_file_failure *cached_map_file_failure_find(const char *name);
void cached_map_file_failure_add(const char *name);
void cached_map_file_failure_remove(const char *name);
uint8_t cached_map_file_failure_is_final(const char *name);
long cached_map_file_copy_current_priority(const char *name);
long cached_map_file_copy_pending_priority(const char *name);
long cached_map_file_copy_priority(const char *name);
long cached_map_file_status(const char *name);
void cached_map_file_copy_cancel(void);
uint8_t cached_map_file_request(const char *name, long priority);
void cached_map_file_copy_buffer_acquire(long size);
void cached_map_file_copy_buffer_release(void);
uint8_t cached_map_file_copy_get_progress(float *progress);
float cached_map_file_get_progress(const char *name);
uint8_t cached_map_file_copy_update(void);
void cache_files_initialize(void);
void cache_file_get_map_name(const char *path, char *name);
void cache_file_get_map_path(const char *scenario_name, char *path);
long cached_map_files_find_map(const char *scenario_name);
uint8_t cache_file_read_header(struct cache_file_header *header, const char *scenario_name);
void cache_file_read_async(uint32_t offset, long size, void *buffer, long *bytes_read, uint8_t *done,
	long category, long priority);
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
extern struct cache_file_tag_instance *global_tag_instances;
extern const char *cache_file_extension;
extern struct cached_map_file_copy_globals cached_map_file_copy_globals;
extern long global_language;
extern long cached_map_files_language;
extern long cached_map_files_build_number;
extern uint8_t cached_map_files_state_changed;

#endif
