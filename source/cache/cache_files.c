/*
CACHE_FILES.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.
*/

#include "cache_files.h"

#include "cseries/async.h"
#include "cseries/cseries.h"
#include "memory/binary_search.h"
#include "memory/physical_memory.h"
#include "scenario/scenario.h"
#include "scenario/scenario_definitions.h"
#include "structures/structure_bsp_definitions.h"

#include <string.h>

/* ---------- prototypes of functions of the console's libraries */

/* 0x002d2750 */
uint32_t CreateFileA(const char *path, uint32_t access, uint32_t share_mode, void *security_attributes,
	uint32_t creation_disposition, uint32_t flags, uint32_t template_file);

/* 0x002d2241 */
int ReadFile(uint32_t file, void *buffer, uint32_t size, uint32_t *bytes_read, void *overlapped);

/* 0x002d1f3a */
int CloseHandle(uint32_t handle);

/* 0x002d6b6b */
int DeleteFileA(const char *path);

/* 0x00331598 */
long CompareFileTime(const struct cache_file_time *time1, const struct cache_file_time *time2);

/* 0x0033039a */
long XGetLanguage(void);

/* 0x002d1d66 */
void SetLastError(uint32_t error);

/* ---------- constants */

#define GENERIC_READ 0x80000000
#define OPEN_EXISTING 3
#define GENERIC_WRITE 0x40000000
#define OPEN_ALWAYS 4
#define FILE_FLAG_NO_BUFFERING 0x20000000
#define INVALID_HANDLE_VALUE 0xFFFFFFFF

enum
{
	/* a read is of whole sectors of the disc */
	CACHE_FILE_SECTOR_SIZE= 0x200,

	/* the two high bits of an offset tell the file */
	CACHE_FILE_OFFSET_FILE_SHIFT= 30,
	CACHE_FILE_OFFSET_MASK= 0x3FFFFFFF
};

/* the places in cached_map_file_globals of the maps that the others take
resources from, and the two high bits of an offset in each */
enum
{
	_cached_map_file_shared,
	_cached_map_file_single_player_shared,
	_cached_map_file_main_menu
};

enum
{
	_cache_file_offset_local,
	_cache_file_offset_main_menu,
	_cache_file_offset_shared,
	_cache_file_offset_single_player_shared
};

/* the shared maps in the order that a map's header has them */
enum
{
	_cache_file_shared_map_main_menu,
	_cache_file_shared_map_shared,
	_cache_file_shared_map_single_player_shared,
	NUMBER_OF_CACHE_FILE_SHARED_MAPS
};

enum
{
	/* the build of the game */
	CACHED_MAP_FILES_BUILD_NUMBER= 9809,
	/* copies with a number from the count of places to here are from
	another build of the game */
	MAXIMUM_OLD_CACHED_MAP_FILES= 20,
	/* of the tasks that the game waits for here */
	CACHED_MAP_FILE_TASK_PRIORITY= 6
};

/* ---------- globals */

/* 0x005476f8 */
struct cache_file_globals cache_file_globals;

/* 0x00557c90 */
struct cached_map_file_globals cached_map_file_globals;

/* 0x00510a08: the number of the build that made the copies of the maps on
the hard disk, 9809; zero makes the game copy them again. A field of a
structure that is not understood yet, as the next one. The name is a guess */
long cached_map_files_build_number;

/* 0x00510819: set when something of that structure changes */
uint8_t cached_map_files_state_changed;

/* 0x00510838: the language that the copies of the maps on the hard disk are
in. Another field of that structure */
long cached_map_files_language;

/* 0x0047ff38: the language of the game; NONE until it is asked for */
long global_language= NONE;

/* 0x0055b4f0 */
struct cached_map_file_copy_globals cached_map_file_copy_globals;

/* 0x004e3b44: the tag instances of the tag header */
struct cache_file_tag_instance *global_tag_instances;

/* 0x0043e86c: the builds whose maps the game accepts */
static const char *const cache_file_builds[]=
{
	"02.09.27.09809"
};

/* 0x00448f98: the most that the copy of a map on the hard disk can take,
for the places of cached_map_file_globals from the first to the last: the
main menu's, the shared one, the single player shared one, a multiplayer
map's and those of two solo maps */
static const struct cached_map_file_size
{
	long first_index;
	long last_index;
	uint32_t maximum_size;
} cached_map_file_sizes[]=
{
	{2, 2, 0x05000000},
	{0, 0, 0x0B400000},
	{1, 1, 0x20800000},
	{3, 3, 0x05000000},
	{4, 5, 0x11800000}
};

/* 0x0046880c */
const char *cache_file_extension= ".map";

/* 0x005020d8: a buffer that the functions at 0x00215180 and 0x002151f0
use, which are not understood yet. The name is a guess */
struct cache_file_buffer_globals cache_file_buffer_globals;

/* ---------- code */

/* 0x00214f10: takes the memory of that buffer, of the sum of two sizes;
its address, or NULL. The name is a guess */
void *cache_file_buffer_allocate(
	uint32_t size1,
	uint32_t size2)
{
	uint32_t size= size1+size2;
	void *buffer= physical_memory_allocate_high(size);

	cache_file_buffer_globals.buffer= buffer;
	cache_file_buffer_globals.size= size;
	cache_file_buffer_globals.allocated= 1;

	return buffer;
}

static inline __attribute__((always_inline)) const struct cached_map_file_size *cached_map_file_size_find(
	long index)
{
	unsigned long size_index;

	for (size_index= 0; size_index<sizeof(cached_map_file_sizes)/sizeof(cached_map_file_sizes[0]); size_index++)
	{
		const struct cached_map_file_size *size= &cached_map_file_sizes[size_index];

		if (index>=size->first_index && index<=size->last_index)
		{
			return size;
		}
	}

	return NULL;
}

/* 0x00214c90: the most that the copy of a map on the hard disk can take, by
its place in cached_map_file_globals; 0 if there is no such place. The name
is a guess */
uint32_t cached_map_file_maximum_size(
	long index)
{
	const struct cached_map_file_size *size= cached_map_file_size_find(index);

	return size ? size->maximum_size : 0;
}

/* 0x00214cc0: the path of the copy of a map on the hard disk, by its place
in cached_map_file_globals: the shared maps on one drive and the others on
another. Empty if there is no such place. The name is a guess */
void cached_map_file_get_path(
	char *path,
	unsigned long size,
	long index)
{
	const struct cached_map_file_size *file_size= cached_map_file_size_find(index);

	strncpy(path, "", size);
	path[size-1]= 0;
	if (file_size)
	{
		const char *drive= NULL;

		if (index>=0 && index<=1)
		{
			drive= "z:\\";
		}
		else if (index>=2 && index<=5)
		{
			drive= "n:\\";
		}
		csnzprintf(path, size, "%scache%03d%s", drive, index, cache_file_extension);
	}
}

/* 0x002138f0: deletes the copies of maps on the hard disk, from a place to
before another, on both drives. The name is a guess */
void cached_map_files_delete(
	long first_index,
	long end_index)
{
	char path[256];
	long index;

	for (index= first_index; index<end_index; index++)
	{
		csnzprintf(path, sizeof(path), "%scache%03d%s", "z:\\", index, cache_file_extension);
		DeleteFileA(path);
		csnzprintf(path, sizeof(path), "%scache%03d%s", "n:\\", index, cache_file_extension);
		DeleteFileA(path);
	}
	SetLastError(0);
}

/* 0x00214c30: the place in cached_map_file_globals of one of the shared
maps, in the order that a map's header has them; NULL if there is no such
map. The name is a guess */
struct cached_map_file *cached_map_file_shared_get(
	long shared_index)
{
	switch (shared_index)
	{
	case _cache_file_shared_map_main_menu:
		return &cached_map_file_globals.files[_cached_map_file_main_menu];
	case _cache_file_shared_map_shared:
		return &cached_map_file_globals.files[_cached_map_file_shared];
	case _cache_file_shared_map_single_player_shared:
		return &cached_map_file_globals.files[_cached_map_file_single_player_shared];
	}

	return NULL;
}

/* 0x002149c0: reads the header of the copy of a map on the hard disk, which
has to be open; whether it was read and is good. The name is a guess */
uint8_t cached_map_file_read_header(
	long index)
{
	struct cached_map_file *file= &cached_map_file_globals.files[index];
	char path[256];
	long bytes_read= 0;
	volatile uint8_t done= 0;

	/* the game does nothing with the path */
	cached_map_file_get_path(path, sizeof(path), index);
	async_read_position(file->file, sizeof(struct cache_file_header), 0, 0, CACHED_MAP_FILE_TASK_PRIORITY,
		&bytes_read, &file->header, (uint8_t *)&done);
	while (!done)
	{
		SwitchToThread();
	}

	return bytes_read==sizeof(struct cache_file_header) && cache_file_header_verify(&file->header);
}

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

/* 0x00214d40: the name of a map from a path: what is before the first
period that is not at the start, in lower case. At most 255 characters.
The name is a guess */
void cache_file_get_map_name(
	const char *path,
	char *name)
{
	char *extension;
	long count;

	strncpy(name, path, 256);
	name[255]= 0;
	extension= name+strspn(name, ".");
	if (*extension)
	{
		extension= strpbrk(extension, ".");
		if (extension)
		{
			*extension= 0;
		}
	}

	for (count= 256; *name && count; count--, name++)
	{
		if (*name>='A' && *name<='Z')
		{
			*name+= 'a'-'A';
		}
	}
}

/* 0x00214db0: the path of the file of a scenario's map: on the disc, in
the folder of the maps; or, if the name starts with the drive t: or w:, where
the name says. As in Halo 1 */
void cache_file_get_map_path(
	const char *scenario_name,
	char *path)
{
	char name[256];

	if ((scenario_name[0]=='t' && scenario_name[1]==':' && scenario_name[2]=='\\') ||
		(scenario_name[0]=='w' && scenario_name[1]==':' && scenario_name[2]=='\\'))
	{
		cache_file_get_map_name(scenario_name, name);
		csprintf256(path, "%s.map", name);
	}
	else
	{
		const char *last_backslash= strrchr(scenario_name, '\\');

		cache_file_get_map_name(last_backslash ? last_backslash+1 : scenario_name, name);
		csprintf256(path, "d:\\maps\\%s.map", name);
	}
}

/* 0x00214a60: the place in cached_map_file_globals of the map of a
scenario; NONE if it is not there. The name is a guess */
long cached_map_files_find_map(
	const char *scenario_name)
{
	char path[256];
	long index;

	path[0]= 0;
	cache_file_get_map_path(scenario_name, path);
	for (index= 0; index<MAXIMUM_CACHED_MAP_FILES; index++)
	{
		if (csstrcasecmp(path, cached_map_file_globals.files[index].header.path)==0)
		{
			return index;
		}
	}

	return NONE;
}

/* 0x00213800: reads the header of a scenario's map from its file; whether
it was read and is good. The name is a guess */
uint8_t cache_file_read_header(
	struct cache_file_header *header,
	const char *scenario_name)
{
	char path[256];
	uint32_t bytes_read;
	uint32_t file;
	uint8_t result= 0;

	path[0]= 0;
	cache_file_get_map_path(scenario_name, path);
	file= CreateFileA(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);
	if (file==INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	if (ReadFile(file, header, sizeof(struct cache_file_header), &bytes_read, NULL) &&
		bytes_read==sizeof(struct cache_file_header) &&
		cache_file_header_verify(header))
	{
		result= 1;
	}
	CloseHandle(file);

	return result;
}

static inline __attribute__((always_inline)) void cached_map_file_wait(
	const volatile uint8_t *done)
{
	while (!*done)
	{
		SwitchToThread();
	}
}

/* 0x00213970: opens the copies of the maps on the hard disk, making those
that are not there. A copy that does not have the size of its place is
given it, with an empty header, and so are all the ones after it. The
header of a copy that cannot be used is left empty. The name is a guess */
void cached_map_files_open(
	void)
{
	uint8_t shared_map_valid[NUMBER_OF_CACHE_FILE_SHARED_MAPS]= {1, 1, 1};
	uint8_t delete_rest= 1;
	long index;

	cached_map_files_delete(MAXIMUM_CACHED_MAP_FILES, MAXIMUM_OLD_CACHED_MAP_FILES);
	for (index= 0; index<MAXIMUM_CACHED_MAP_FILES; index++)
	{
		struct cached_map_file *file= &cached_map_file_globals.files[index];
		union
		{
			struct async_create_file_parameters create;
			uint32_t parameters[ASYNC_TASK_PARAMETER_COUNT];
		} task;
		char path[256];
		uint32_t maximum_size;
		uint32_t file_size;
		volatile uint8_t done;
		uint8_t keep= 0;

		cached_map_file_get_path(path, sizeof(path), index);
		maximum_size= cached_map_file_maximum_size(index);

		memset(&task, 0, sizeof(task));
		task.create.path= path;
		task.create.access= GENERIC_READ|GENERIC_WRITE;
		task.create.share_mode= 0;
		task.create.creation_disposition= OPEN_ALWAYS;
		task.create.flags= FILE_FLAG_NO_BUFFERING;
		task.create.file= &file->file;
		task.create.create_directories= 0;
		async_task_add(CACHED_MAP_FILE_TASK_PRIORITY, task.parameters, 0, async_create_file_work, (uint8_t *)&done);
		cached_map_file_wait(&done);

		if (file->file!=INVALID_HANDLE_VALUE)
		{
			async_get_file_size(file->file, 0, CACHED_MAP_FILE_TASK_PRIORITY, &file_size, (uint8_t *)&done);
			cached_map_file_wait(&done);

			if (file_size!=maximum_size)
			{
				uint8_t size_set;
				uint8_t written= 0;

				/* what the copies after this one have is of no use now */
				if (delete_rest)
				{
					cached_map_files_delete(index+1, MAXIMUM_CACHED_MAP_FILES);
					delete_rest= 0;
				}

				async_set_file_size(file->file, (long)maximum_size, 0, CACHED_MAP_FILE_TASK_PRIORITY, &size_set,
					(uint8_t *)&done);
				cached_map_file_wait(&done);
				if (size_set)
				{
					/* the game writes a header with whatever the stack
					has, and only empties its strings */
					struct cache_file_header header;
					uint32_t bytes_written;

					header.path[0]= 0;
					header.build[0]= 0;
					header.name[0]= 0;
					header.scenario_path[0]= 0;
					async_write_position(file->file, &header, sizeof(header), 0, 0, 0, CACHED_MAP_FILE_TASK_PRIORITY,
						&bytes_written, (uint8_t *)&done);
					cached_map_file_wait(&done);
					written= bytes_written==sizeof(header);
				}

				if (!written && file->file!=INVALID_HANDLE_VALUE)
				{
					async_close_file(file->file, 0, CACHED_MAP_FILE_TASK_PRIORITY, (uint8_t *)&done);
					cached_map_file_wait(&done);
					file->file= INVALID_HANDLE_VALUE;
				}
			}
			else if (cached_map_file_read_header(index))
			{
				if (index!=_cached_map_file_single_player_shared)
				{
					keep= 1;
				}
				else
				{
					/* the copy has to be of the map that the disc has */
					struct cache_file_header disc_header;

					disc_header.path[0]= 0;
					disc_header.build[0]= 0;
					disc_header.name[0]= 0;
					disc_header.scenario_path[0]= 0;
					if (cache_file_read_header(&disc_header, file->header.path) &&
						file->header.unknown144==disc_header.unknown144)
					{
						keep= 1;
						/* the game has this here, where the place is always
						that of a shared map: the copy of another map also
						has to be of the same shared maps that are there */
						if (index!=_cached_map_file_shared && index!=_cached_map_file_single_player_shared)
						{
							long shared_index;

							for (shared_index= 0; shared_index<NUMBER_OF_CACHE_FILE_SHARED_MAPS; shared_index++)
							{
								if (file->header.uses_shared_map[shared_index] && shared_map_valid[shared_index])
								{
									keep= CompareFileTime(&cached_map_file_shared_get(shared_index)->header.times[0],
										&file->header.times[1+shared_index])==0;
								}
							}
						}
					}
				}
			}
		}

		if (!keep)
		{
			memset(&file->header, 0, sizeof(file->header));
			switch (index)
			{
			case _cached_map_file_shared:
				shared_map_valid[_cache_file_shared_map_shared]= 0;
				break;
			case _cached_map_file_single_player_shared:
				shared_map_valid[_cache_file_shared_map_single_player_shared]= 0;
				break;
			case _cached_map_file_main_menu:
				shared_map_valid[_cache_file_shared_map_main_menu]= 0;
				break;
			}
		}
	}
}

/* 0x002136c0: forgets the maps, deletes their copies on the hard disk if
they are of another language or of another build of the game, and opens
them. The name is that of Halo 1's function */
void cache_files_initialize(
	void)
{
	memset(&cached_map_file_globals, 0, sizeof(cached_map_file_globals));
	memset(&cached_map_file_copy_globals, 0, sizeof(cached_map_file_copy_globals));
	cached_map_file_globals.open_map_index= NONE;
	cached_map_file_copy_globals.unknown81C= 1;
	cached_map_file_copy_globals.unknown810= NONE;
	cached_map_file_copy_globals.unknown820= NONE;

	if (global_language==NONE)
	{
		global_language= language_from_xbox_language(XGetLanguage());
	}
	if (cached_map_files_language!=global_language)
	{
		cached_map_files_delete(0, MAXIMUM_CACHED_MAP_FILES);
		cached_map_files_language= global_language;
		cached_map_files_state_changed= 1;
	}
	if (cached_map_files_build_number!=CACHED_MAP_FILES_BUILD_NUMBER)
	{
		cached_map_files_delete(0, MAXIMUM_CACHED_MAP_FILES);
		cached_map_files_build_number= CACHED_MAP_FILES_BUILD_NUMBER;
		cached_map_files_state_changed= 1;
	}

	cached_map_files_open();
}

/* 0x00213760: starts to read a part of a cache file. When the read ends,
the number of bytes read and a flag are set. The two high bits of the offset
tell the file: the map that is loaded, or one of the three that the maps
take resources from. The name is a guess */
void cache_file_read_async(
	uint32_t offset,
	long size,
	void *buffer,
	long *bytes_read,
	uint8_t *done,
	long category,
	long priority)
{
	uint32_t file= cached_map_file_globals.files[cached_map_file_globals.open_map_index].file;

	switch (offset>>CACHE_FILE_OFFSET_FILE_SHIFT)
	{
	case _cache_file_offset_main_menu:
		file= cached_map_file_globals.files[_cached_map_file_main_menu].file;
		break;
	case _cache_file_offset_shared:
		file= cached_map_file_globals.files[_cached_map_file_shared].file;
		break;
	case _cache_file_offset_single_player_shared:
		file= cached_map_file_globals.files[_cached_map_file_single_player_shared].file;
		break;
	}

	async_read_position(file, size, offset&CACHE_FILE_OFFSET_MASK, category, priority, bytes_read, buffer, done);
}

/* 0x00213890: waits for the reads that were started to end. The name is
that of the function that does it in Halo 1 */
void cache_file_block_until_not_busy(
	void)
{
	long categories[5]= {2, 3, 4, 5, 6};
	unsigned long index;

	for (index= 0; index<sizeof(categories)/sizeof(categories[0]); index++)
	{
		while (async_category_busy(categories[index]))
		{
			SwitchToThread();
		}
	}
}

/* 0x00122d60: reads a part of the cache file and waits for it; whether all
of it was read. The two high bits of the offset tell the file, as in a
geometry block's offset. The name is a guess */
uint8_t cache_file_blocking_read(
	long unknown,
	uint32_t offset,
	void *buffer,
	long size)
{
	long bytes_read;
	volatile uint8_t done= 0;

	(void)unknown;

	cache_file_read_async(offset, size, buffer, &bytes_read, (uint8_t *)&done, 2, 6);
	while (!done)
	{
		SwitchToThread();
	}

	return bytes_read==size;
}

static inline uint32_t cache_file_round_up_read_size(
	uint32_t size)
{
	if (size&(CACHE_FILE_SECTOR_SIZE-1))
	{
		size= (size|(CACHE_FILE_SECTOR_SIZE-1))+1;
	}

	return size;
}

/* 0x001228f0: loads the tags of a scenario's map and sets the scenario and
the globals tag; whether the tags were loaded. In Halo 1 these are two
functions, scenario_load and scenario_tags_load */
uint8_t scenario_load(
	const char *scenario_name)
{
	struct cache_file_header *header= &cache_file_globals.header;
	long scenario_index= NONE;
	uint8_t loaded= 0;

	cached_map_file_globals.open_map_index= cached_map_files_find_map(scenario_name);
	memcpy(header, &cached_map_file_globals.files[cached_map_file_globals.open_map_index].header,
		sizeof(struct cache_file_header));

	if (cache_file_header_verify(header) && cache_file_build_verify(header->build))
	{
		cache_file_globals.tag_memory= physical_memory_allocate_low(header->tag_memory_size);
		if (cache_file_globals.tag_memory)
		{
			struct cache_file_tag_header *tag_header= cache_file_globals.tag_memory;

			/* the tag index at the start of the memory and the tag data at
			its end */
			if (cache_file_blocking_read(NONE, header->tag_data_offset, cache_file_globals.tag_memory,
					(long)cache_file_round_up_read_size(header->tag_index_size)) &&
				cache_file_blocking_read(NONE, header->tag_data_offset+header->tag_index_size,
					(uint8_t *)cache_file_globals.tag_memory+header->tag_memory_size-header->tag_data_size,
					(long)cache_file_round_up_read_size(header->tag_data_size)) &&
				tag_header->tag_instances!=0 &&
				tag_header->tag_count>0 &&
				tag_header->signature==CACHE_FILE_TAG_HEADER_SIGNATURE)
			{
				cache_file_globals.tag_header= tag_header;
				loaded= 1;
				cache_file_globals.tags_loaded= 1;
				scenario_index= (long)tag_header->scenario_index;
				global_tag_instances= (struct cache_file_tag_instance *)(uintptr_t)tag_header->tag_instances;
			}
			else
			{
				/* the copy of the map on the hard disk is not good */
				cached_map_files_build_number= 0;
				cached_map_files_state_changed= 1;
			}
		}
	}

	if (!loaded)
	{
		cache_file_globals.tag_memory= NULL;
		if (cached_map_file_globals.open_map_index!=NONE)
		{
			cache_file_block_until_not_busy();
			cached_map_file_globals.open_map_index= NONE;
		}
	}

	global_scenario_index= scenario_index;
	if (scenario_index!=NONE)
	{
		global_scenario= (struct scenario *)(uintptr_t)global_tag_instances[scenario_index&0xFFFF].address;
		global_game_globals= (struct game_globals *)(uintptr_t)
			global_tag_instances[cache_file_globals.tag_header->globals_index&0xFFFF].address;
		return 1;
	}

	return loaded;
}

/* 0x00122af0: forgets the cache file that is loaded. The name is that of
Halo 1's function */
void scenario_tags_unload(
	void)
{
	if (cached_map_file_globals.open_map_index!=NONE)
	{
		cache_file_block_until_not_busy();
		cached_map_file_globals.open_map_index= NONE;
	}

	cache_file_globals.tags_loaded= 0;
	cache_file_globals.tag_memory= NULL;
	memset(&cache_file_globals.header, 0, sizeof(struct cache_file_header));
	global_tag_instances= NULL;
	cache_file_globals.tag_header= NULL;
	cache_file_globals.structure_bsp_header= NULL;
}

/* 0x00122b40: reads a structure bsp of the scenario to its address, and
gives the tags of the structure bsp and of its lightmap their addresses in
it. The name is a guess */
uint8_t cache_file_structure_bsp_load(
	const struct scenario_structure_bsp_reference *reference)
{
	uint32_t size= cache_file_round_up_read_size(reference->block_size);
	struct cache_file_structure_bsp_header *header;

	if (!cache_file_blocking_read(NONE, reference->block_offset, (void *)(uintptr_t)reference->block_address,
		(long)size))
	{
		return 0;
	}
	header= (struct cache_file_structure_bsp_header *)(uintptr_t)reference->block_address;
	if (header->signature!=CACHE_FILE_STRUCTURE_BSP_HEADER_SIGNATURE)
	{
		return 0;
	}

	cache_file_globals.structure_bsp_header= header;
	global_tag_instances[(short)reference->structure_bsp.index].address= header->structure_bsp;
	if (reference->structure_lightmap.index!=(uint32_t)NONE)
	{
		global_tag_instances[(short)reference->structure_lightmap.index].address=
			cache_file_globals.structure_bsp_header->structure_lightmap;
	}

	return 1;
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
