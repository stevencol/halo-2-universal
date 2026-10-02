/*
CACHE_FILES.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.
*/

#include "cache_files.h"

#include "cseries/async.h"
#include "cseries/cseries.h"
#include "main/global_preferences.h"
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
long CompareFileTime(const struct file_time *time1, const struct file_time *time2);

/* 0x0033039a */
long XGetLanguage(void);

/* 0x003303f3 */
void GetSystemTimeAsFileTime(struct file_time *time);

/* 0x003314b0 */
long GetTickCount(void);

/* 0x002d1d66 */
void SetLastError(uint32_t error);

/* ---------- prototypes of functions not decompiled yet */

/* 0x0012e150: lets go of what the game keeps for a tag, as its textures */
void unknown_0012e150(long tag_index);

/* ---------- constants */

#define GENERIC_READ 0x80000000
#define OPEN_EXISTING 3
#define GENERIC_WRITE 0x40000000
#define OPEN_ALWAYS 4
#define FILE_FLAG_NO_BUFFERING 0x20000000
#define INVALID_HANDLE_VALUE 0xFFFFFFFF

enum
{
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

/* 0x00448fd4: the priority of the tasks of a copy, by the priority of who
asked for the copy */
static const long cached_map_file_copy_task_priorities[]= {0, 2, 2, 2, 2, 2, 2};

/* 0x00448ff0: the size of the buffer of a copy, the same way */
static const long cached_map_file_copy_buffer_sizes[]=
	{0x40000, 0x100000, 0x100000, 0x100000, 0x100000, 0x100000, 0x100000};

/* ---------- code */

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
	struct file_time now;

	/* the game does nothing with the time */
	GetSystemTimeAsFileTime(&now);

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
	cached_map_file_copy_globals.done= 1;
	cached_map_file_copy_globals.task_id= NONE;
	cached_map_file_copy_globals.destination_index= NONE;

	if (global_language==NONE)
	{
		global_language= language_from_xbox_language(XGetLanguage());
	}
	if (global_preferences_globals.current.data.language!=global_language)
	{
		cached_map_files_delete(0, MAXIMUM_CACHED_MAP_FILES);
		global_preferences_globals.current.data.language= global_language;
		global_preferences_globals.dirty= TRUE;
	}
	if (global_preferences_globals.current.data.cache_build_number!=CACHED_MAP_FILES_BUILD_NUMBER)
	{
		cached_map_files_delete(0, MAXIMUM_CACHED_MAP_FILES);
		global_preferences_globals.current.data.cache_build_number= CACHED_MAP_FILES_BUILD_NUMBER;
		global_preferences_globals.dirty= TRUE;
	}

	cached_map_files_open();
}

/* 0x00123110: a checksum of a buffer: all its 32-bit words combined with
exclusive or, in pairs. A constant is added when the count of pairs is odd.
The name is a guess */
uint32_t cache_file_xor_checksum(
	const void *buffer,
	uint32_t size)
{
	const uint32_t *words= buffer;
	uint32_t count= size>>3;
	uint32_t low= 0;
	uint32_t high= 0;
	uint32_t index;

	if (count&1)
	{
		low= 0x08040201;
		high= 0x80402010;
	}
	for (index= 0; index<count; index++)
	{
		low^= words[2*index];
		high^= words[2*index+1];
	}

	return low^high;
}

/* 0x00214e40: whether two scenario names are of the same map file. The
name is a guess, as those of all the functions from here to
cached_map_file_copy_update */
uint8_t cache_file_map_paths_equal(
	const char *name1,
	const char *name2)
{
	char path1[256];
	char path2[256];

	path1[0]= 0;
	path2[0]= 0;
	cache_file_get_map_path(name1, path1);
	cache_file_get_map_path(name2, path2);

	return strcmp(path1, path2)==0;
}

/* 0x00214c70: the place in cached_map_file_globals of one of the shared
maps, in the order that a map's header has them; NONE if there is no such
map */
long cached_map_file_shared_index(
	long shared_index)
{
	switch (shared_index)
	{
	case _cache_file_shared_map_main_menu:
		return _cached_map_file_main_menu;
	case _cache_file_shared_map_shared:
		return _cached_map_file_shared;
	case _cache_file_shared_map_single_player_shared:
		return _cached_map_file_single_player_shared;
	}

	return NONE;
}

/* 0x00214ac0: whether a place is better than the best one found so far to
put the copy of a map in: the one whose copy was made first, among those of
the most room */
uint8_t cached_map_file_is_better_place(
	long best_index,
	long index)
{
	struct file_time now;
	struct file_time *best_time;
	struct file_time *time;

	file_time_get_current(&now);
	if (best_index==NONE)
	{
		return 1;
	}
	if (index==NONE)
	{
		return 0;
	}

	best_time= &cached_map_file_globals.files[best_index].header.copy_time;
	time= &cached_map_file_globals.files[index].header.copy_time;
	/* a time that is yet to come is not to be trusted */
	if (CompareFileTime(best_time, &now)>0)
	{
		return 0;
	}
	if (CompareFileTime(time, &now)>0)
	{
		return 1;
	}
	if (cached_map_file_maximum_size(best_index)<cached_map_file_maximum_size(index))
	{
		return 1;
	}

	return CompareFileTime(time, best_time)<0;
}

/* 0x00214b80: the place in cached_map_file_globals to put the copy of a map
in, by the type of its scenario and its length; NONE if there is none. The
place of the map that is loaded is not taken */
long cached_map_file_choose_place(
	long scenario_type,
	uint32_t file_length)
{
	long first_index= 0;
	long last_index= 0;
	long best_index= NONE;
	long index;

	switch ((short)scenario_type)
	{
	case _scenario_type_solo:
		first_index= 4;
		last_index= 5;
		break;
	case _scenario_type_multiplayer:
		first_index= last_index= 3;
		break;
	case _scenario_type_main_menu:
		first_index= last_index= _cached_map_file_main_menu;
		break;
	case _scenario_type_shared:
		first_index= last_index= _cached_map_file_shared;
		break;
	case _scenario_type_single_player_shared:
		first_index= last_index= _cached_map_file_single_player_shared;
		break;
	}

	for (index= first_index; index<=last_index; index++)
	{
		if (index!=cached_map_file_globals.open_map_index &&
			cached_map_file_maximum_size(index)>=file_length &&
			cached_map_file_is_better_place(best_index, index))
		{
			best_index= index;
		}
	}

	return best_index;
}

/* 0x00214060: what the game keeps of the copies of a map that failed; NULL
if none did */
struct cached_map_file_failure *cached_map_file_failure_find(
	const char *name)
{
	char path[256];
	long index;

	path[0]= 0;
	cache_file_get_map_path(name, path);
	for (index= 0; index<MAXIMUM_CACHED_MAP_FILE_FAILURES; index++)
	{
		struct cached_map_file_failure *failure= &cached_map_file_globals.failures[index];

		if (strcmp(path, failure->path)==0)
		{
			return failure;
		}
	}

	return NULL;
}

/* 0x00214100: counts a copy of a map that failed. If the map has no place
in the table, it takes one that is not in use, or that of the map that
failed the longest ago */
void cached_map_file_failure_add(
	const char *name)
{
	struct cached_map_file_failure *failure;
	char path[256];

	path[0]= 0;
	cache_file_get_map_path(name, path);
	failure= cached_map_file_failure_find(path);
	if (!failure)
	{
		long index;

		for (index= 0; index<MAXIMUM_CACHED_MAP_FILE_FAILURES; index++)
		{
			struct cached_map_file_failure *other= &cached_map_file_globals.failures[index];

			if (!other->path[0])
			{
				failure= other;
				break;
			}
			if (!failure || other->time<failure->time)
			{
				failure= other;
			}
		}
		if (!failure)
		{
			return;
		}
		memcpy(failure->path, path, sizeof(failure->path));
		failure->count= 0;
	}

	failure->time= GetTickCount();
	failure->count++;
	/* the game looks for it again and does nothing with it */
	cached_map_file_failure_find(name);
}

/* 0x002141b0: forgets the copies of a map that failed */
void cached_map_file_failure_remove(
	const char *name)
{
	struct cached_map_file_failure *failure= cached_map_file_failure_find(name);

	if (failure)
	{
		memset(failure, 0, sizeof(*failure));
	}
}

/* 0x002141d0: whether the copy of a map failed so many times that the game
does not try again */
uint8_t cached_map_file_failure_is_final(
	const char *name)
{
	struct cached_map_file_failure *failure= cached_map_file_failure_find(name);
	long count= 0;

	if (failure)
	{
		count= failure->count;
	}

	return count>=MAXIMUM_CACHED_MAP_FILE_FAILURE_COUNT;
}

/* 0x00213e30: the priority of the copy that is being made, if it is of the
map; NONE otherwise */
long cached_map_file_copy_current_priority(
	const char *name)
{
	char path[256];
	char source_path[256];

	path[0]= 0;
	cache_file_get_map_path(name, path);
	source_path[0]= 0;
	cache_file_get_map_path(cached_map_file_copy_globals.source_path, source_path);

	return csstrcasecmp(path, source_path)==0 ? cached_map_file_copy_globals.current.priority : NONE;
}

/* 0x00213e90: the priority of the copy that waits for the one that is being
made, if it is of the map; NONE otherwise */
long cached_map_file_copy_pending_priority(
	const char *name)
{
	char path[256];
	char pending_path[256];

	path[0]= 0;
	cache_file_get_map_path(name, path);
	pending_path[0]= 0;
	cache_file_get_map_path(cached_map_file_copy_globals.pending.name, pending_path);

	return csstrcasecmp(path, pending_path)==0 ? cached_map_file_copy_globals.pending.priority : NONE;
}

/* 0x00213ef0: the priority that a copy of the map was asked for with; NONE
if none was */
long cached_map_file_copy_priority(
	const char *name)
{
	long priority= cached_map_file_copy_current_priority(name);

	if (priority==NONE)
	{
		priority= cached_map_file_copy_pending_priority(name);
	}

	return priority;
}

/* 0x00213f10: what there is of the copy of a map on the hard disk */
long cached_map_file_status(
	const char *name)
{
	struct cached_map_file_failure *failure;
	char path[256];

	path[0]= 0;
	cache_file_get_map_path(name, path);
	if (cached_map_files_find_map(path)!=NONE)
	{
		return _cached_map_file_status_cached;
	}

	failure= cached_map_file_failure_find(name);
	if (failure && failure->count>=MAXIMUM_CACHED_MAP_FILE_FAILURE_COUNT)
	{
		return _cached_map_file_status_failed;
	}
	if (cached_map_file_copy_current_priority(name)!=NONE)
	{
		return _cached_map_file_status_copying;
	}

	return cached_map_file_copy_pending_priority(name)!=NONE;
}

/* 0x00214990: stops the copy that is being made */
void cached_map_file_copy_cancel(
	void)
{
	if (cached_map_file_copy_globals.state!=_cached_map_file_copy_idle &&
		cached_map_file_copy_globals.state<_cached_map_file_copy_end)
	{
		cached_map_file_copy_globals.result= _cached_map_file_copy_result_cancelled;
		cached_map_file_copy_globals.state= _cached_map_file_copy_end;
	}
}

/* 0x00213d20: asks for the copy of a map to the hard disk; whether the map
is copied or will be. Only one map can wait for the copy that is being made:
the one asked for with the most priority, or the last one. A copy with less
priority than the one that waits is stopped */
uint8_t cached_map_file_request(
	const char *name,
	long priority)
{
	struct cached_map_file_request *current= &cached_map_file_copy_globals.current;
	struct cached_map_file_request *pending= &cached_map_file_copy_globals.pending;
	uint8_t result= 0;
	long status= cached_map_file_status(name);
	char path[256];

	path[0]= 0;
	cache_file_get_map_path(name, path);
	switch (status)
	{
	case _cached_map_file_status_none:
		if (!pending->name[0] || priority>=pending->priority)
		{
			memset(pending, 0, sizeof(*pending));
			strncpy(pending->name, path, sizeof(pending->name));
			pending->name[sizeof(pending->name)-1]= 0;
			pending->priority= priority;
			result= 1;
		}
		break;
	case _cached_map_file_status_copying:
		if (cache_file_map_paths_equal(path, current->name))
		{
			current->priority= priority;
		}
		result= 1;
		break;
	case _cached_map_file_status_pending:
		if (cache_file_map_paths_equal(path, pending->name))
		{
			pending->priority= priority;
		}
		result= 1;
		break;
	case _cached_map_file_status_cached:
		result= 1;
		break;
	}

	if (current->name[0] && pending->name[0] && pending->priority>=current->priority)
	{
		cached_map_file_copy_cancel();
	}

	return result;
}

/* 0x00213fa0: what the game calls to have the memory of the buffer back:
goes on with the copy until it gives the buffer up */
void PHYSICAL_MEMORY_CALLBACK cached_map_file_copy_buffer_reclaim(
	void *buffer,
	long unknown)
{
	(void)buffer;
	(void)unknown;

	while (cached_map_file_copy_globals.buffer)
	{
		cached_map_file_copy_globals.release_requested= 1;
		if (cached_map_file_copy_globals.done)
		{
			cached_map_file_copy_update();
		}
	}
}

/* 0x00213fe0: takes the memory of the buffer, of a size; the buffer stays
NULL if there is none */
void cached_map_file_copy_buffer_acquire(
	long size)
{
	if (cached_map_file_copy_globals.buffer_size!=size)
	{
		if (cached_map_file_copy_globals.buffer)
		{
			physical_memory_buffer_free(cached_map_file_copy_globals.buffer);
			cached_map_file_copy_globals.buffer= NULL;
			cached_map_file_copy_globals.buffer_size= 0;
		}
		cached_map_file_copy_globals.buffer= physical_memory_buffer_allocate(0, size, 0, NULL,
			cached_map_file_copy_buffer_reclaim);
		if (cached_map_file_copy_globals.buffer)
		{
			cached_map_file_copy_globals.buffer_size= size;
		}
	}
}

/* 0x00214030: gives it back */
void cached_map_file_copy_buffer_release(
	void)
{
	if (cached_map_file_copy_globals.buffer)
	{
		physical_memory_buffer_free(cached_map_file_copy_globals.buffer);
		cached_map_file_copy_globals.buffer= NULL;
		cached_map_file_copy_globals.buffer_size= 0;
	}
}

/* 0x002148b0: how much of the copy is made, from 0 to 1; whether there is a
copy to make */
uint8_t cached_map_file_copy_get_progress(
	float *progress_reference)
{
	long state= cached_map_file_copy_globals.state;
	float progress= 0.0f;
	uint8_t result= 0;

	if (state==_cached_map_file_copy_idle)
	{
		if (cached_map_file_copy_globals.pending.name[0])
		{
			result= 1;
		}
	}
	else
	{
		if (state>=_cached_map_file_copy_copy_data)
		{
			if (state>_cached_map_file_copy_check_data)
			{
				progress= 1.0f;
			}
			else if (cached_map_file_copy_globals.file_size>0)
			{
				/* a division of the x87 registers, in the precision that
				the game sets them to */
				progress= (float)((long double)(long)cached_map_file_copy_globals.offset/
					(long double)cached_map_file_copy_globals.file_size);
			}
		}

		result= 1;
		if (0.0f>progress)
		{
			progress= 0.0f;
		}
		else if (progress>1.0f)
		{
			progress= 1.0f;
		}
	}

	if (progress_reference)
	{
		*progress_reference= progress;
	}

	return result;
}

/* 0x00214940: how much of the copy of a map to the hard disk is made, from 0
to 1 */
float cached_map_file_get_progress(
	const char *name)
{
	float progress= 0.0f;
	long status= cached_map_file_status(name);

	if (status==_cached_map_file_status_cached)
	{
		return 1.0f;
	}
	if (status==_cached_map_file_status_copying)
	{
		if (cache_file_map_paths_equal(cached_map_file_copy_globals.source_path, name))
		{
			cached_map_file_copy_get_progress(&progress);
		}
		return progress;
	}

	return 0.0f;
}

/* 0x002141f0: the next step of the copy of a map to the hard disk; whether
the step went well. To be called when the task of the step before is done.

The map is read through a buffer and written to the place of a copy that is
not in use. The header of the copy is erased first and written last, so that
a copy that was not finished has no header. What is copied is checked against
the checksum of the header */
uint8_t cached_map_file_copy_update(
	void)
{
	struct cached_map_file_copy_globals *copy= &cached_map_file_copy_globals;
	long priority= cached_map_file_copy_task_priorities[copy->current.priority];
	long state= copy->state;
	uint8_t success= 1;

	switch (state)
	{
	case _cached_map_file_copy_idle:
		if (copy->pending.name[0] &&
			!cached_map_file_failure_is_final(copy->pending.name) &&
			cached_map_file_status(copy->pending.name)!=_cached_map_file_status_cached)
		{
			cache_file_get_map_path(copy->pending.name, copy->source_path);
			memcpy(&copy->current, &copy->pending, sizeof(copy->current));
			state= _cached_map_file_copy_open_source;
		}
		memset(&copy->pending, 0, sizeof(copy->pending));
		copy->result= _cached_map_file_copy_result_none;
		copy->state= state;
		return success;

	case _cached_map_file_copy_open_source:
		copy->task_id= async_create_file(_async_file_open_existing, copy->source_path, 1, 1, 1, priority,
			&copy->source_file, &copy->done);
		copy->state= _cached_map_file_copy_check_source;
		return success;

	case _cached_map_file_copy_check_source:
		success= copy->source_file!=INVALID_HANDLE_VALUE;
		state= _cached_map_file_copy_read_header;
		break;

	case _cached_map_file_copy_read_header:
		memset(&copy->header, 0, sizeof(copy->header));
		copy->task_id= async_read_position(copy->source_file, sizeof(copy->header), 0, 1, priority,
			(long *)&copy->bytes_transferred, &copy->header, &copy->done);
		copy->state= _cached_map_file_copy_check_header;
		return success;

	case _cached_map_file_copy_check_header:
		file_time_get_current(&copy->header.copy_time);
		strncpy(copy->header.path, copy->source_path, sizeof(copy->header.path));
		copy->header.path[sizeof(copy->header.path)-1]= 0;
		if (copy->bytes_transferred==sizeof(copy->header) && cache_file_header_verify(&copy->header))
		{
			long shared_index;

			/* the shared maps that the map takes resources from have to be
			on the hard disk, and be those that the map was made with */
			for (shared_index= 0; shared_index<NUMBER_OF_CACHE_FILE_SHARED_MAPS; shared_index++)
			{
				if (copy->header.uses_shared_map[shared_index])
				{
					struct cached_map_file *shared=
						&cached_map_file_globals.files[cached_map_file_shared_index(shared_index)];

					if (!shared->header.path[0] ||
						CompareFileTime(&shared->header.times[0], &copy->header.times[1+shared_index])!=0)
					{
						success= 0;
					}
				}
			}
		}
		else
		{
			success= 0;
		}
		state= _cached_map_file_copy_get_size;
		break;

	case _cached_map_file_copy_get_size:
		copy->task_id= async_get_file_size(copy->source_file, 1, priority, &copy->file_size, &copy->done);
		copy->state= _cached_map_file_copy_check_size;
		return success;

	case _cached_map_file_copy_check_size:
		success= copy->file_size!=0xFFFFFFFF && copy->file_size>sizeof(copy->header);
		copy->checksum= 0;
		state= _cached_map_file_copy_erase_header;
		break;

	case _cached_map_file_copy_erase_header:
	{
		long index= cached_map_file_choose_place(copy->header.scenario_type, (uint32_t)copy->header.file_length);
		struct cached_map_file *file;

		if (index==NONE)
		{
			/* waits for a place */
			copy->state= state;
			return success;
		}
		copy->destination_index= index;
		file= &cached_map_file_globals.files[index];
		memset(&file->header, 0, sizeof(file->header));
		copy->task_id= async_write_position(file->file, &file->header, sizeof(file->header), 0, 0, 1, priority,
			&copy->bytes_transferred, &copy->done);
		copy->state= _cached_map_file_copy_check_erase;
		return success;
	}

	case _cached_map_file_copy_check_erase:
		success= copy->bytes_transferred==sizeof(copy->header);
		state= _cached_map_file_copy_flush_erase;
		break;

	case _cached_map_file_copy_flush_erase:
		copy->task_id= async_flush_file(cached_map_file_globals.files[copy->destination_index].file, 1, priority,
			&copy->done);
		copy->state= _cached_map_file_copy_start_data;
		return success;

	case _cached_map_file_copy_start_data:
		copy->bytes_transferred= 0;
		copy->offset= sizeof(copy->header);
		copy->state= _cached_map_file_copy_copy_data;
		return success;

	case _cached_map_file_copy_copy_data:
	{
		long size;

		cached_map_file_copy_buffer_acquire(cached_map_file_copy_buffer_sizes[copy->current.priority]);
		if (!copy->buffer)
		{
			/* waits for the memory */
			copy->state= _cached_map_file_copy_copy_data;
			return success;
		}
		size= (long)(copy->file_size-copy->offset);
		if (copy->buffer_size<=size)
		{
			size= copy->buffer_size;
		}
		copy->bytes_requested= size;
		copy->task_id= async_copy_position(copy->source_file,
			cached_map_file_globals.files[copy->destination_index].file, copy->buffer, (uint32_t)size, copy->offset,
			copy->offset, 1, priority, &copy->bytes_transferred, &copy->done);
		copy->state= _cached_map_file_copy_check_data;
		return success;
	}

	case _cached_map_file_copy_check_data:
		success= (uint32_t)copy->bytes_requested==copy->bytes_transferred;
		copy->checksum^= cache_file_xor_checksum(copy->buffer, copy->bytes_transferred);
		if (success)
		{
			copy->offset+= copy->bytes_transferred;
			if (copy->offset>=copy->file_size)
			{
				success= copy->checksum==copy->header.checksum;
				state= _cached_map_file_copy_flush_data;
			}
			else
			{
				state= _cached_map_file_copy_copy_data;
			}
		}
		if (copy->release_requested)
		{
			cached_map_file_copy_buffer_release();
			copy->release_requested= 0;
		}
		break;

	case _cached_map_file_copy_flush_data:
		copy->task_id= async_flush_file(cached_map_file_globals.files[copy->destination_index].file, 1, priority,
			&copy->done);
		copy->state= _cached_map_file_copy_write_header;
		return success;

	case _cached_map_file_copy_write_header:
	{
		struct cached_map_file *file= &cached_map_file_globals.files[copy->destination_index];

		file_time_get_current(&copy->header.copy_time);
		copy->task_id= async_write_position(file->file, &copy->header, sizeof(copy->header), 0, 0, 1, 0,
			&copy->bytes_transferred, &copy->done);
		copy->state= _cached_map_file_copy_check_write;
		return success;
	}

	case _cached_map_file_copy_check_write:
		success= copy->bytes_transferred==sizeof(copy->header);
		state= _cached_map_file_copy_finish;
		break;

	case _cached_map_file_copy_finish:
		memcpy(&cached_map_file_globals.files[copy->destination_index].header, &copy->header, sizeof(copy->header));
		memset(&copy->header, 0, sizeof(copy->header));
		copy->result= _cached_map_file_copy_result_success;
		copy->state= _cached_map_file_copy_end;
		return success;

	case _cached_map_file_copy_end:
		copy->state= _cached_map_file_copy_close_source;
		return success;

	case _cached_map_file_copy_close_source:
		if (copy->source_file!=INVALID_HANDLE_VALUE)
		{
			copy->task_id= async_close_file(copy->source_file, 0, CACHED_MAP_FILE_TASK_PRIORITY, &copy->done);
		}
		copy->state= _cached_map_file_copy_clean_up;
		return success;

	case _cached_map_file_copy_clean_up:
	{
		struct cached_map_file_request request= copy->current;

		cached_map_file_copy_buffer_release();
		copy->destination_index= NONE;
		memset(&copy->header, 0, sizeof(copy->header));
		memset(&copy->current, 0, sizeof(copy->current));
		copy->source_path[0]= 0;
		copy->source_file= INVALID_HANDLE_VALUE;
		switch (copy->result)
		{
		case _cached_map_file_copy_result_success:
			cached_map_file_failure_remove(request.name);
			break;
		case _cached_map_file_copy_result_failure:
			/* tries again */
			if (!cached_map_file_failure_is_final(request.name))
			{
				cached_map_file_request(request.name, request.priority);
			}
			break;
		}
		copy->state= _cached_map_file_copy_idle;
		return success;
	}

	default:
		return success;
	}

	/* after a step that checks what a task did */
	if (!success)
	{
		cached_map_file_failure_add(copy->source_path);
		copy->state= _cached_map_file_copy_end;
		copy->result= _cached_map_file_copy_result_failure;
		return success;
	}
	copy->state= state;

	return success;
}

/* 0x00213760: starts to read a part of a cache file. When the read ends,
the number of bytes read and a flag are set. The two high bits of the offset
tell the file: the map that is loaded, or one of the three that the maps
take resources from; the task's number. The name is a guess */
long cache_file_read_async(
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

	return async_read_position(file, size, offset&CACHE_FILE_OFFSET_MASK, category, priority, bytes_read, buffer,
		done);
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
				global_preferences_globals.current.data.cache_build_number= 0;
				global_preferences_globals.dirty= TRUE;
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

/* 0x00122bc0: forgets the structure bsp of the scenario that is loaded: its
tag and that of its lightmap have no address. The name is a guess */
void cache_file_structure_bsp_unload(
	const struct scenario_structure_bsp_reference *reference)
{
	unknown_0012e150((long)reference->structure_bsp.index);
	global_tag_instances[(short)reference->structure_bsp.index].address= 0;
	if (reference->structure_lightmap.index!=(uint32_t)NONE)
	{
		unknown_0012e150((long)reference->structure_lightmap.index);
		global_tag_instances[(short)reference->structure_lightmap.index].address= 0;
	}
	cache_file_globals.structure_bsp_header= NULL;
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
