/*
SAVED_GAME_FILES.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The files that the players keep: profiles, variants of the games and
playlists. The names of the functions are guesses. The file goes from
0x00215790 to 0x00217590.
*/

#include "saved_game_files.h"

#include "input/input.h"
#include "saved games/player_profile.h"
#include "saved games/game_state.h"
#include "tag_files/files.h"
#include "text/unicode_string_list.h"
#include "cseries/async.h"
#include "cseries/sort.h"
#include "sound/sound_definitions.h"

#include <stdint.h>
#include <string.h>

/* ---------- prototypes of functions of the console's libraries */

/* 0x002d5f65: how many bytes a cluster of a drive has */
uint32_t XGetDiskClusterSize(const char *root);

/* 0x002d5838 */
int GetDiskFreeSpaceExA(const char *directory, uint64_t *free_bytes_available, uint64_t *total_bytes,
	uint64_t *total_free_bytes);

/* 0x002d2234 */
uint32_t GetCurrentThreadId(void);

/* 0x00331125, 0x0033122f, 0x00331276: go through the saved games of a drive,
and tell of each */
uint32_t XFindFirstSaveGame(const char *root, struct saved_game_find_data *data);
int XFindNextSaveGame(uint32_t handle, struct saved_game_find_data *data);
int XFindClose(uint32_t handle);

/* 0x00331055: deletes the saved game that the console shows with a name */
uint32_t XDeleteSaveGame(const char *root, const wchar *name);

/* 0x00330df2: makes the directory of a saved game, or finds the one that
there is, by the name that the console shows of it. 0 if it does */
uint32_t XCreateSaveGame(const char *root, const wchar *name, uint32_t disposition, uint32_t flags, char *path,
	uint32_t path_size);

/* ---------- prototypes of functions that are not decompiled */


/* ---------- constants */

enum
{
	/* in unknown_0051ea14: the tables of the files, and a number that goes
	into the index of each file: it is where the count of the second table
	is */
	UNKNOWN_0051EA14_TABLES_OFFSET= 0xBEF8,
	UNKNOWN_0051EA14_SALT_OFFSET= 0x4BEFC,
	UNKNOWN_0051EA14_UNKNOWN4BF00_OFFSET= 0x4BF00,

	SAVED_GAME_FILE_RESULT_DESCRIPTION_OFFSET= 8,
	SAVED_GAME_FILE_LANGUAGE_NONE= 0xFF,

	CREATE_NEW= 1,
	OPEN_EXISTING= 3,
	INVALID_HANDLE_VALUE= -1
};

/* ---------- globals */

/* 0x0055c14c: the files of the drives have been looked for (it is main.c's
for now) */
extern boolean unknown_0055c14c;

/* 0x0051ea14: what the game knows of the files; NULL if it is not set up */
extern void *unknown_0051ea14;

/* 0x0055c274 */
struct saved_game_files_operation saved_game_files_operation;

/* 0x0055c14d, 0x0055c150: the files can be looked for, and what there is to
do with them: the first bit, to look for those of the drives (they are
main.c's for now) */
extern boolean unknown_0055c14d;
extern unsigned long unknown_0055c150;

/* 0x0055c14e: the memory of the files was asked for */
boolean saved_game_files_memory_allocated;

/* 0x0055c15c: the salt that the indices of the files get the next time that
the memory is taken: one more each time, of 512 */
long saved_game_files_next_salt;

/* ---------- prototypes of functions that are not decompiled */

/* 0x000531b0: takes memory for what the allocation of the files asks */
void *ALLOCATION_CONVENTION unknown_000531b0(struct allocation *allocation, unsigned long size);

/* 0x00020670: calls a function with each of the elements of an array, as
what makes them */
void __attribute__((stdcall)) array_construct_elements(void *array, unsigned long size, long count,
	void *(ALLOCATION_CONVENTION *constructor)(void *element));

/* 0x004508fc: what the memory of the files is asked of. It gives nothing
back */
const struct allocation_functions saved_game_files_allocation_functions=
{
	unknown_000531b0,
	(void (ALLOCATION_CONVENTION *)(struct allocation *, void *))no_operation1
};

/* what the memory is asked with: the functions, and a number for them */
struct saved_game_files_allocation
{
	struct allocation allocation;
	long unknown;
};

/* 0x0055c154: what the console's library said when a file could not be
read or written */
long saved_game_files_last_error;

/* 0x0055c158: the tag that has the texts that name the types of files; NONE
if it is not known */
long saved_game_files_string_list_index;

/* 0x0055c280: the language that the files are named in */
long saved_game_files_language;

/* 0x00470054: what is between the name of the type of a file and the name
that the player gave it */
const wchar *saved_game_file_name_separator= u": ";

/* 0x00449038: the picture that the console shows next to a variant of a
game: a file of textures of the console (XPR0), of 4096 bytes. It is the
game's, and it is not copied here: what is built from this has an empty
picture */
byte saved_game_image[SAVED_GAME_IMAGE_SIZE];

/* 0x0055c14f, 0x0055c160, 0x0055c164: what is kept of a few files when the
tables are not set up */
boolean saved_game_files_cache_valid;
long cached_saved_game_file_count;
struct cached_saved_game_file cached_saved_game_files[MAXIMUM_CACHED_SAVED_GAME_FILES];

/* ---------- prototypes of functions of the C library */

/* 0x00321ce4, 0x003207ed: of characters of 16 bits */
int wcsncmp(const wchar *a, const wchar *b, unsigned int count);
wchar *wcsncpy(wchar *destination, const wchar *source, unsigned int count);

/* ---------- code */

/* the root of a drive, as "u:\"; empty if the drive is not there. The game
has this, for the hard disk, in the functions that ask how much room it has */
static inline __attribute__((always_inline)) void saved_game_files_get_hard_disk_root(
	char *root)
{
	if (input_globals.unit_mask&1)
	{
		csnzprintf(root, SAVED_GAME_FILE_ROOT_LENGTH, "%c:\\", INPUT_HARD_DISK_DRIVE_LETTER);
	}
	else
	{
		root[0]= 0;
	}
}

/* saved_game_files_get_unit_root, which the game has in itself in some
functions */
static inline __attribute__((always_inline)) void saved_game_files_get_unit_root_inline(
	long unit_index,
	char *root,
	short size)
{
	char letter;

	if (input_get_unit_drive_letter(unit_index, &letter))
	{
		csnzprintf(root, (unsigned long)(long)size, "%c:\\", letter);
	}
	else
	{
		root[0]= 0;
	}
}

/* 0x00216400: the root of a drive, the hard disk or a memory unit; empty if
it is not there */
char *saved_game_files_get_unit_root(
	long unit_index,
	char *root,
	short size)
{
	char letter;

	if (input_get_unit_drive_letter(unit_index, &letter))
	{
		csnzprintf(root, (unsigned long)(long)size, "%c:\\", letter);
	}
	else
	{
		root[0]= 0;
	}

	return root;
}

/* 0x00216a50: the type of the file of a variant. The variant has to be of
one of the seven games: the game has no answer for any other */
long saved_game_file_type_from_variant(
	const struct game_variant *variant)
{
	switch (variant->unknown44)
	{
	case _game_engine_ctf:
		return _saved_game_file_type_ctf;
	case _game_engine_slayer:
		return _saved_game_file_type_slayer;
	case _game_engine_oddball:
		return _saved_game_file_type_oddball;
	case _game_engine_koth:
		return _saved_game_file_type_koth;
	case _game_engine_juggernaut:
		return _saved_game_file_type_juggernaut;
	case _game_engine_territories:
		return _saved_game_file_type_territories;
	case _game_engine_assault:
		return _saved_game_file_type_assault;
	}
	__builtin_unreachable();
}

/* 0x00216ab0: the type of the files of the variants of a game, by where the
game is in the list. It has to be in it */
long saved_game_file_type_from_listed_game(
	long game)
{
	switch (game)
	{
	case _game_list_slayer:
		return _saved_game_file_type_slayer;
	case _game_list_oddball:
		return _saved_game_file_type_oddball;
	case _game_list_juggernaut:
		return _saved_game_file_type_juggernaut;
	case _game_list_koth:
		return _saved_game_file_type_koth;
	case _game_list_ctf:
		return _saved_game_file_type_ctf;
	case _game_list_assault:
		return _saved_game_file_type_assault;
	case _game_list_territories:
		return _saved_game_file_type_territories;
	}
	__builtin_unreachable();
}

/* 0x00216b00: where in the list is the game that a type of file is of. It
has to be of one of the seven */
long saved_game_file_type_to_listed_game(
	long type)
{
	switch (type)
	{
	case _saved_game_file_type_slayer:
		return _game_list_slayer;
	case _saved_game_file_type_koth:
		return _game_list_koth;
	case _saved_game_file_type_oddball:
		return _game_list_oddball;
	case _saved_game_file_type_juggernaut:
		return _game_list_juggernaut;
	case _saved_game_file_type_ctf:
		return _game_list_ctf;
	case _saved_game_file_type_assault:
		return _game_list_assault;
	case _saved_game_file_type_territories:
		return _game_list_territories;
	}
	__builtin_unreachable();
}

/* 0x00216b60: the name of a type of file: that of the directory where the
files of the type are */
const char *saved_game_file_type_get_name(
	long type)
{
	const char *names[NUMBER_OF_SAVED_GAME_FILE_TYPES]=
	{
		"profile",
		"slayer",
		"koth",
		"race",
		"oddball",
		"juggernaut",
		"headhunter",
		"ctf",
		"assault",
		"territories",
		"playlist"
	};

	if (type>=0 && type<NUMBER_OF_SAVED_GAME_FILE_TYPES)
	{
		return names[type];
	}

	return "unknown";
}

/* 0x00216bd0: the text that names a type of file to the player; 0 if the
type is none */
unsigned long saved_game_file_type_get_string_id(
	long type)
{
	unsigned long string_ids[NUMBER_OF_SAVED_GAME_FILE_TYPES]=
	{
		0x1B0006E9,
		0x130006EA,
		0x110006EB,
		0x110006EC,
		0x140006ED,
		0x170006EE,
		0x170006EF,
		0x100006F0,
		0x140006F1,
		0x180006F2,
		0x150006F3
	};

	if (type>=0 && type<NUMBER_OF_SAVED_GAME_FILE_TYPES)
	{
		return string_ids[type];
	}

	return 0;
}

/* how many bytes a file of a type has */
static inline __attribute__((always_inline)) long saved_game_file_type_get_size_inline(
	long type)
{
	if (type==_saved_game_file_type_profile)
	{
		return SAVED_GAME_FILE_PROFILE_SIZE;
	}

	return type<=_saved_game_file_type_territories ? SAVED_GAME_FILE_VARIANT_SIZE : SAVED_GAME_FILE_PLAYLIST_SIZE;
}

/* 0x00216c40: how many blocks of the hard disk a file of a type takes */
long saved_game_file_type_get_blocks(
	long type)
{
	return saved_game_file_size_to_blocks(saved_game_file_type_get_size_inline(type));
}

/* 0x00216c70: how many blocks of the hard disk a file of so many bytes
takes */
long saved_game_file_size_to_blocks(
	long size)
{
	char root[SAVED_GAME_FILE_ROOT_LENGTH];
	long cluster_size;

	saved_game_files_get_hard_disk_root(root);
	cluster_size= (long)XGetDiskClusterSize(root);

	return (size + cluster_size*SAVED_GAME_FILE_EXTRA_CLUSTERS + SAVED_GAME_FILE_BLOCK_SIZE-1)/SAVED_GAME_FILE_BLOCK_SIZE;
}

/* 0x00216ce0: whether the hard disk has room for so many blocks */
boolean saved_game_files_hard_disk_has_blocks(
	long blocks)
{
	long size= (long)((unsigned long)blocks<<14);
	char root[SAVED_GAME_FILE_ROOT_LENGTH];
	uint64_t free_bytes_available;
	uint64_t total_bytes;
	uint64_t total_free_bytes;

	saved_game_files_get_hard_disk_root(root);

	return GetDiskFreeSpaceExA(root, &free_bytes_available, &total_bytes, &total_free_bytes) &&
		free_bytes_available>=(uint64_t)(int64_t)size;
}

/* 0x00216d70: the directory of the files of a type, in another */
boolean saved_game_file_get_type_path(
	char *path,
	const char *directory,
	long type)
{
	strncpy(path, directory, SAVED_GAME_FILE_PATH_LENGTH);
	path[SAVED_GAME_FILE_PATH_LENGTH-1]= 0;
	csstrcat256(path, saved_game_file_type_get_name(type));

	return TRUE;
}

/* 0x00217480 */
long saved_game_file_type_get_size(
	long type)
{
	return saved_game_file_type_get_size_inline(type);
}

/* 0x002160b0: the directory of a file of a drive. The game waits until the
files of the drives have been looked for */
boolean saved_game_file_get_directory(
	char *path,
	long file)
{
	struct saved_game_file_information information;

	if ((file>>_saved_game_file_index_built_in_bit)&1)
	{
		return FALSE;
	}
	information.path[0]= 0;
	information.name[0]= 0;
	while (!*(volatile boolean *)&unknown_0055c14c)
	{
		SwitchToThread();
		sound_idle();
	}
	if (!saved_game_file_get_information(&information, file))
	{
		return FALSE;
	}
	strncpy(path, information.path, SAVED_GAME_FILE_PATH_LENGTH);
	path[SAVED_GAME_FILE_PATH_LENGTH-1]= 0;

	return TRUE;
}

/* 0x002163d0: whether a file goes after another in a list: those that are
built in go after those of the drives, and the others by their indices */
boolean SORT_CALLBACK saved_game_file_index_after(
	long file,
	long other,
	void *unused)
{
	(void)unused;

	if (((file>>_saved_game_file_index_built_in_bit)^(other>>_saved_game_file_index_built_in_bit))&1)
	{
		return (file>>_saved_game_file_index_built_in_bit)&1;
	}

	return file>other;
}

/* 0x00217300: the index that a file has once another is deleted: one less
if it was after it among those of its drive, with a new salt */
long saved_game_file_index_after_delete(
	long file,
	long deleted)
{
	const byte *files= unknown_0051ea14;
	long unit= (file>>SAVED_GAME_FILE_INDEX_UNIT_SHIFT)&SAVED_GAME_FILE_INDEX_UNIT_MASK;
	long index= (file>>SAVED_GAME_FILE_INDEX_FILE_SHIFT)&SAVED_GAME_FILE_INDEX_FILE_MASK;

	if (!((file>>_saved_game_file_index_built_in_bit)&1) && files &&
		((deleted>>SAVED_GAME_FILE_INDEX_UNIT_SHIFT)&SAVED_GAME_FILE_INDEX_UNIT_MASK)==unit &&
		index>((deleted>>SAVED_GAME_FILE_INDEX_FILE_SHIFT)&SAVED_GAME_FILE_INDEX_FILE_MASK))
	{
		unsigned long salt= *(const uint32_t *)(files+UNKNOWN_0051EA14_SALT_OFFSET)&SAVED_GAME_FILE_INDEX_SALT_MASK;

		return (long)((salt<<SAVED_GAME_FILE_INDEX_SALT_SHIFT) |
			((unsigned long)unit<<SAVED_GAME_FILE_INDEX_UNIT_SHIFT) |
			(((unsigned long)(index-1)&SAVED_GAME_FILE_INDEX_FILE_MASK)<<SAVED_GAME_FILE_INDEX_FILE_SHIFT) |
			((unsigned long)file&SAVED_GAME_FILE_INDEX_TYPE_MASK));
	}

	return file;
}

/* 0x00217370: waits for what is being done with the files to be over, and
takes the turn for a file */
void saved_game_files_take_operation(
	long file,
	struct saved_game_files_operation **operation)
{
	unsigned long thread= GetCurrentThreadId();

	while (!*(volatile boolean *)&saved_game_files_operation.done)
	{
		SwitchToThread();
		sound_idle();
	}
	saved_game_files_operation.thread= thread;
	saved_game_files_operation.done= FALSE;
	saved_game_files_operation.file= file;
	*operation= &saved_game_files_operation;
}

/* the path of the file that a saved game keeps the state of the game in */
static inline __attribute__((always_inline)) boolean saved_game_auxiliary_get_path(
	char *path,
	long saved_game)
{
	path[0]= 0;
	if (!saved_game_file_get_directory(path, saved_game))
	{
		return FALSE;
	}
	csstrcat256(path, "auxilary.bin");

	return TRUE;
}

/* 0x002174b0: starts to read the file that a saved game of the hard disk
keeps the state of the game in ("auxilary.bin"): what is at its start, and
the rest. Whether it was started */
boolean saved_game_auxiliary_read(
	void *header,
	long header_size,
	void *data,
	long data_size,
	struct saved_game_file_result *result,
	long saved_game)
{
	char path[SAVED_GAME_FILE_PATH_LENGTH];

	if (!saved_game_auxiliary_get_path(path, saved_game))
	{
		return FALSE;
	}

	return saved_game_state_read_task_new(path, result, header, header_size, data, data_size, FALSE);
}

/* 0x00217520: starts to write it */
boolean saved_game_auxiliary_write(
	void *header,
	long header_size,
	void *data,
	long data_size,
	struct saved_game_file_result *result,
	long saved_game)
{
	char path[SAVED_GAME_FILE_PATH_LENGTH];

	if (!saved_game_auxiliary_get_path(path, saved_game))
	{
		return FALSE;
	}

	return saved_game_state_write_task_new(path, result, header, header_size, data, data_size, FALSE);
}

/* 0x00217590: how many saved games a drive has */
long saved_game_files_count_saved_games(
	long unit_index)
{
	char root[SAVED_GAME_FILE_ROOT_LENGTH]= { 0 };
	struct saved_game_find_data data;
	long count= 0;
	uint32_t handle;

	saved_game_files_get_unit_root_inline(unit_index, root, sizeof(root));
	handle= XFindFirstSaveGame(root, &data);
	if (handle!=(uint32_t)INVALID_HANDLE_VALUE)
	{
		do
		{
			count++;
		} while (XFindNextSaveGame(handle, &data));
		XFindClose(handle);
	}

	return count;
}

/* the table that a file of a drive is in, and its place in it */
static inline __attribute__((always_inline)) struct saved_game_file_table *saved_game_file_get_table(
	long file)
{
	return (struct saved_game_file_table *)((byte *)unknown_0051ea14+UNKNOWN_0051EA14_TABLES_OFFSET)+
		((file>>SAVED_GAME_FILE_INDEX_UNIT_SHIFT)&SAVED_GAME_FILE_INDEX_UNIT_MASK);
}

static inline __attribute__((always_inline)) long saved_game_file_get_table_place(
	long file)
{
	return (file>>SAVED_GAME_FILE_INDEX_FILE_SHIFT)&SAVED_GAME_FILE_INDEX_FILE_MASK;
}

/* whether a table has a file at a place */
static inline __attribute__((always_inline)) boolean saved_game_file_table_has(
	const struct saved_game_file_table *table,
	long place)
{
	return place<=(long)((unsigned long)table->count-1);
}

/* what is kept of a file when the tables are not set up; NULL if nothing is */
static inline __attribute__((always_inline)) struct cached_saved_game_file *cached_saved_game_file_get(
	long file)
{
	long index;

	for (index= 0; index<cached_saved_game_file_count; index++)
	{
		if (cached_saved_game_files[index].file!=NONE && cached_saved_game_files[index].file==file)
		{
			return &cached_saved_game_files[index];
		}
	}

	return NULL;
}

/* 0x00216060: whether a file has another name than this one. FALSE too if
the file is not known or the name is empty */
boolean saved_game_file_name_differs(
	long file,
	const wchar *name)
{
	struct saved_game_file_information information;

	information.path[0]= 0;
	information.name[0]= 0;

	return saved_game_file_get_information(&information, file) && name && name[0] &&
		wcsncmp(information.name, name, SAVED_GAME_FILE_NAME_COMPARE_LENGTH)!=0;
}

/* 0x00216800: what the game knows of a file of a drive: FALSE if it knows
nothing */
boolean saved_game_file_get_information(
	struct saved_game_file_information *information,
	long file)
{
	if (unknown_0051ea14)
	{
		const struct saved_game_file_table *table;
		long place;

		if ((file>>_saved_game_file_index_built_in_bit)&1)
		{
			return FALSE;
		}
		table= saved_game_file_get_table(file);
		place= saved_game_file_get_table_place(file);
		if (!saved_game_file_table_has(table, place))
		{
			return FALSE;
		}
		*information= table->files[place];

		return TRUE;
	}
	if (saved_game_files_cache_valid)
	{
		const struct cached_saved_game_file *cached= cached_saved_game_file_get(file);

		if (cached)
		{
			*information= cached->information;

			return TRUE;
		}
	}

	return FALSE;
}

/* 0x002168b0: changes what the game knows of a file. Not while another
thing is being done with the files, unless it is with this one and another
thread asked. It tells FALSE when what changes is what is kept with the
tables not set up */
boolean saved_game_file_set_information(
	const struct saved_game_file_information *information,
	long file)
{
	unsigned long thread= GetCurrentThreadId();

	if (!saved_game_files_operation.done &&
		(saved_game_files_operation.file!=file || saved_game_files_operation.thread==thread))
	{
		return FALSE;
	}
	if (unknown_0051ea14)
	{
		struct saved_game_file_table *table= saved_game_file_get_table(file);
		long place= saved_game_file_get_table_place(file);

		if (!saved_game_file_table_has(table, place))
		{
			return FALSE;
		}
		table->files[place]= *information;

		return TRUE;
	}
	if (saved_game_files_cache_valid)
	{
		struct cached_saved_game_file *cached= cached_saved_game_file_get(file);

		if (cached)
		{
			cached->information= *information;
		}
	}

	return FALSE;
}

/* 0x00216990: a file more in a table: its place in it */
boolean saved_game_file_add_information(
	long table_index,
	const struct saved_game_file_information *information,
	long *index)
{
	struct saved_game_file_table *table;

	if (!unknown_0051ea14)
	{
		return FALSE;
	}
	table= (struct saved_game_file_table *)((byte *)unknown_0051ea14+UNKNOWN_0051EA14_TABLES_OFFSET)+table_index;
	if (table->count==MAXIMUM_SAVED_GAME_FILES_PER_TABLE)
	{
		return FALSE;
	}
	table->files[table->count++]= *information;
	*index= table->count-1;

	return TRUE;
}

/* 0x002169e0: a file less in its table: those after it go down a place */
void saved_game_file_delete_information(
	long file)
{
	if (unknown_0051ea14)
	{
		struct saved_game_file_table *table= saved_game_file_get_table(file);
		long place= saved_game_file_get_table_place(file);

		if (saved_game_file_table_has(table, place))
		{
			if (place<table->count-1)
			{
				memmove(&table->files[place], &table->files[place+1],
					(unsigned long)(table->count-place-1)*sizeof(struct saved_game_file_information));
			}
			table->count--;
		}
	}
}

/* 0x00216f80: writes the picture of a variant of a game in its directory
("SaveImage.xbx"). The files of the other types have none: nothing is
written, and that is not to fail */
boolean saved_game_file_write_image(
	long type,
	const char *directory)
{
	char path[SAVED_GAME_FILE_PATH_LENGTH];
	struct file_reference file;
	long error;

	if (type<_saved_game_file_type_slayer || type>_saved_game_file_type_territories)
	{
		return TRUE;
	}
	path[0]= 0;
	strncpy(path, directory, SAVED_GAME_FILE_PATH_LENGTH);
	path[SAVED_GAME_FILE_PATH_LENGTH-1]= 0;
	csstrcat256(path, "SaveImage.xbx");
	file_reference_set_path(&file, path, FALSE);

	return file_create(&file) && file_open(&file, FLAG(_permission_write_bit), &error) &&
		file_write(&file, SAVED_GAME_IMAGE_SIZE, saved_game_image) && file_close(&file);
}

/* 0x00216da0: the name that the console shows of a file: the name of its
type in a language, a separator and the name that the player gave it. Only
the last, if there is no language or the text of the type is not there */
boolean saved_game_file_get_display_name(
	long type,
	wchar *display_name,
	const wchar *name,
	long language)
{
	wchar type_name[UNICODE_STRING_LENGTH];

	display_name[0]= 0;
	type_name[0]= 0;
	if (language!=NONE && saved_game_files_string_list_index!=NONE)
	{
		long string_list_index= saved_game_files_string_list_index;
		unsigned long string_id= saved_game_file_type_get_string_id(type);

		if (unicode_string_list_has_string(string_list_index, string_id))
		{
			saved_game_file_get_default_name(string_id, language, type_name);
			wcsncpy(display_name, type_name, SAVED_GAME_FILE_DISPLAY_NAME_LENGTH-1);
			display_name[SAVED_GAME_FILE_DISPLAY_NAME_LENGTH-1]= 0;
			ustrnzcat(display_name, saved_game_file_name_separator, SAVED_GAME_FILE_DISPLAY_NAME_LENGTH);
			display_name[SAVED_GAME_FILE_DISPLAY_NAME_LENGTH-1]= 0;
		}
	}
	ustrnzcat(display_name, name, SAVED_GAME_FILE_DISPLAY_NAME_LENGTH);
	display_name[SAVED_GAME_FILE_DISPLAY_NAME_LENGTH-1]= 0;

	return TRUE;
}

/* 0x00216e50: the name that the player gave a file, from the name that the
console shows of it, and the language that the name of its type is in: 0xFF
if it starts with the name of the type in no language, and then all of it
is the name. The language of the files is tried first */
void saved_game_file_parse_display_name(
	const wchar *display_name,
	long type,
	byte *language,
	wchar *name)
{
	unsigned long string_id;
	long first_language;
	long index;

	*language= 0xFF;
	wcsncpy(name, display_name, SAVED_GAME_FILE_NAME_LENGTH);
	name[SAVED_GAME_FILE_NAME_LENGTH]= 0;
	string_id= saved_game_file_type_get_string_id(type);
	first_language= saved_game_files_language;
	for (index= 0; index<SAVED_GAME_FILE_LANGUAGES; index++)
	{
		long try_language= (first_language+index)%SAVED_GAME_FILE_LANGUAGES;
		wchar type_name[UNICODE_STRING_LENGTH];
		long length;

		type_name[0]= 0;
		saved_game_file_get_default_name(string_id, try_language, type_name);
		ustrnzcat(type_name, saved_game_file_name_separator, UNICODE_STRING_LENGTH);
		type_name[UNICODE_STRING_LENGTH-1]= 0;
		length= saved_game_file_name_length(type_name);
		if (!wcsncmp(display_name, type_name, (unsigned int)length))
		{
			*language= (byte)try_language;
			wcsncpy(name, display_name+length, SAVED_GAME_FILE_NAME_LENGTH);
			name[SAVED_GAME_FILE_NAME_LENGTH]= 0;

			return;
		}
	}
}

/* waits for a file to be read or written, which another thread does */
static inline __attribute__((always_inline)) void saved_game_file_wait(
	const struct saved_game_file_result *result)
{
	while (!result->done)
	{
		SwitchToThread();
		sound_idle();
	}
}

/* 0x00216120: whether no file of a type on the hard disk has this name:
the console is asked for the saved game that is shown with it, and it has
none. FALSE too with no name, and with no hard disk. The name is Halo 1's */
boolean saved_game_file_name_unique(
	const wchar *name,
	long type)
{
	char path[SAVED_GAME_FILE_PATH_LENGTH];
	wchar display_name[SAVED_GAME_FILE_DISPLAY_NAME_LENGTH];
	char root[SAVED_GAME_FILE_ROOT_LENGTH];
	char letter;

	if (!name || !name[0] || !unknown_0051ea14 || !input_get_unit_drive_letter(0, &letter))
	{
		return FALSE;
	}
	memset(path, 0, sizeof(path));
	display_name[0]= 0;
	if (!saved_game_file_get_display_name(type, display_name, name, saved_game_files_language))
	{
		return FALSE;
	}

	return XCreateSaveGame(saved_game_files_get_unit_root(0, root, sizeof(root)), display_name, OPEN_EXISTING, 0,
		path, sizeof(path))!=0;
}

/* 0x002161d0: reads a file of the players, and waits for it. If it does
not go well, why is kept */
boolean saved_game_file_read(
	long file,
	void *buffer,
	unsigned long size)
{
	struct saved_game_file_result result;

	if (!saved_game_file_read_async(size, file, buffer, &result))
	{
		return FALSE;
	}
	saved_game_file_wait(&result);
	if (!result.success)
	{
		saved_game_files_last_error= result.error;
	}

	return result.success;
}

/* 0x00216240: writes a file of the players, and waits for it */
boolean saved_game_file_write(
	long file,
	unsigned long size,
	void *unknown,
	const void *buffer)
{
	struct saved_game_file_result result;

	if (!saved_game_file_write_async(buffer, file, unknown, size, &result))
	{
		return FALSE;
	}
	saved_game_file_wait(&result);
	if (!result.success)
	{
		saved_game_files_last_error= result.error;
	}

	return result.success;
}

/* 0x002162b0: starts to read a file of the players. One that comes with the
game is in memory, and is read at once; one of a drive is read from the file
of its type in its directory, by another thread. Whether it was started */
boolean saved_game_file_read_async(
	unsigned long size,
	long file,
	void *buffer,
	struct saved_game_file_result *result)
{
	struct saved_game_file_information information;
	char path[SAVED_GAME_FILE_PATH_LENGTH];

	if (((file>>_saved_game_file_index_built_in_bit)&1) && unknown_0051ea14)
	{
		const struct built_in_saved_game_files *files= unknown_0051ea14;
		long place= (file>>SAVED_GAME_FILE_INDEX_FILE_SHIFT)&SAVED_GAME_FILE_INDEX_FILE_MASK;

		if (place>(long)((unsigned long)files->count-1) || size>SAVED_GAME_FILE_PROFILE_SIZE)
		{
			return FALSE;
		}
		memcpy(buffer, files->files[place].data, size);
		result->done= TRUE;
		result->success= TRUE;
		result->error= 0;
		result->progress= 1.0f;

		return TRUE;
	}

	information.path[0]= 0;
	information.name[0]= 0;
	path[0]= 0;
	if (!saved_game_file_get_information(&information, file) || information.language==SAVED_GAME_FILE_LANGUAGE_NONE)
	{
		return FALSE;
	}
	strncpy(path, information.path, SAVED_GAME_FILE_PATH_LENGTH);
	path[SAVED_GAME_FILE_PATH_LENGTH-1]= 0;
	csstrcat256(path, saved_game_file_type_get_name(information.type));

	return saved_game_file_read_task_new(buffer, size, FALSE, result, path);
}

/* 0x002173d0: starts to write a file of the players, which another thread
does. What is written starts with the name of the file: if it is not the
name that the file has, the file is given the new one, and for that the turn
is taken. Whether it was started */
boolean saved_game_file_write_async(
	const void *buffer,
	long file,
	void *unknown,
	unsigned long size,
	struct saved_game_file_result *result)
{
	struct saved_game_file_information information;
	struct saved_game_file_description *description=
		(struct saved_game_file_description *)((byte *)result+SAVED_GAME_FILE_RESULT_DESCRIPTION_OFFSET);
	struct saved_game_file_description *filled;

	information.path[0]= 0;
	information.name[0]= 0;
	if (!saved_game_file_get_information(&information, file))
	{
		return FALSE;
	}
	if (saved_game_file_name_differs(file, buffer))
	{
		struct saved_game_files_operation *operation;

		if (!unknown_0051ea14)
		{
			return FALSE;
		}
		saved_game_files_take_operation(file, &operation);
		filled= saved_game_file_description_new_renamed(file, buffer, description, &information,
			(byte *)unknown_0051ea14+UNKNOWN_0051EA14_UNKNOWN4BF00_OFFSET, operation);
	}
	else
	{
		filled= saved_game_file_description_new(description, file, &information);
	}
	if (!filled)
	{
		return FALSE;
	}

	return saved_game_file_write_task_new(unknown, size, FALSE, result);
}

/* 0x00216440: makes a directory, if it is not there. Whether it is there
in the end */
boolean saved_game_files_make_directory(
	const char *path)
{
	struct file_reference directory;

	file_reference_set_path(&directory, path, TRUE);
	if (file_exists(&directory))
	{
		return TRUE;
	}

	return file_create(&directory);
}

/* 0x00216760: one more of the files that come with the game: a profile as
the game gives it to a player who has none, with the name that the game has
for it in the language of the files. Nothing is done if there is no room */
void saved_game_files_add_default_profile(
	void)
{
	struct built_in_saved_game_files *files= unknown_0051ea14;

	if (files && files->count!=MAXIMUM_BUILT_IN_SAVED_GAME_FILES)
	{
		struct built_in_saved_game_file file;
		struct player_profile *profile= (struct player_profile *)file.data;
		wchar name[UNICODE_STRING_LENGTH];

		file.type= _saved_game_file_type_profile;
		name[0]= 0;
		wcsncpy(name, saved_game_file_default_name_unknown(saved_game_files_language), UNICODE_STRING_LENGTH-1);
		name[UNICODE_STRING_LENGTH-1]= 0;
		player_profile_set_defaults(profile);
		wcsncpy(profile->name, name, PLAYER_PROFILE_NAME_LENGTH);
		profile->name[PLAYER_PROFILE_NAME_LENGTH]= 0;

		files= unknown_0051ea14;
		files->files[files->count++]= file;
	}
}

/* 0x00217080: one more of the files that come with the game: a variant of a
game. What follows the variant in the file is whatever was in the stack: the
game does not set it */
void saved_game_files_add_built_in_variant(
	const struct game_variant *variant)
{
	struct built_in_saved_game_files *files= unknown_0051ea14;

	if (files && files->count!=MAXIMUM_BUILT_IN_SAVED_GAME_FILES)
	{
		struct built_in_saved_game_file file;

		memcpy(file.data, variant, sizeof(*variant));
		file.type= saved_game_file_type_from_variant((const struct game_variant *)file.data);
		files->files[files->count++]= file;
	}
}

/* 0x002172a0: forgets what is kept of a file with the tables not set up:
the last of those kept takes its place */
void cached_saved_game_file_delete(
	long file)
{
	long index;

	for (index= 0; index<cached_saved_game_file_count; index++)
	{
		if (cached_saved_game_files[index].file!=NONE && cached_saved_game_files[index].file==file)
		{
			long last= cached_saved_game_file_count-1;

			if (index!=last)
			{
				cached_saved_game_files[index]= cached_saved_game_files[last];
			}
			cached_saved_game_file_count--;

			return;
		}
	}
}

/* 0x00215790: looks for the files of the drives, if it is to be done. The
main loop has this in itself */
void saved_game_files_update(
	void)
{
	if (unknown_0055c14d && unknown_0051ea14 && (unknown_0055c150&FLAG(_saved_game_files_enumerate_bit)))
	{
		saved_game_files_find_unit_files(0,
			(struct saved_game_file_table *)((byte *)unknown_0051ea14+UNKNOWN_0051EA14_TABLES_OFFSET));
		unknown_0055c150&= ~(unsigned long)FLAG(_saved_game_files_enumerate_bit);
	}
}

/* 0x002157c0: tells which tag has the texts that name the types of files,
if it is one, and has the files that come with the game be only the profile
that the game gives a player who has none */
void saved_game_files_set_string_list(
	long string_list_index)
{
	if (string_list_index!=NONE)
	{
		saved_game_files_string_list_index= string_list_index;
	}
	if (unknown_0051ea14)
	{
		((struct built_in_saved_game_files *)unknown_0051ea14)->count= 0;
		saved_game_files_add_default_profile();
	}
}

/* 0x002157e0: takes the memory of the files, in the configuration of the
memory of the game that has it. It is one of the users of the memory of
main_game.c */
void SAVED_GAME_FILES_CALLBACK saved_game_files_allocate_memory(
	long configuration)
{
	if (configuration==_saved_game_files_memory_configuration)
	{
		struct saved_game_files_allocation allocation;

		allocation.allocation.functions= &saved_game_files_allocation_functions;
		allocation.unknown= SAVED_GAME_FILES_ALLOCATION_UNKNOWN;
		saved_game_files_new(&allocation.allocation);
	}
	saved_game_files_memory_allocated= TRUE;
}

/* 0x00215810: lets go of the memory of the files. What it has of the few
files that are kept apart is kept first */
void saved_game_files_free_memory(
	void)
{
	void *files= unknown_0051ea14;
	struct saved_game_files_allocation allocation;

	if (files)
	{
		long index;

		for (index= 0; index<cached_saved_game_file_count; index++)
		{
			saved_game_file_get_information(&cached_saved_game_files[index].information,
				cached_saved_game_files[index].file);
		}
		saved_game_files_cache_valid= TRUE;
	}
	allocation.allocation.functions= &saved_game_files_allocation_functions;
	allocation.unknown= SAVED_GAME_FILES_ALLOCATION_UNKNOWN;
	if (files)
	{
		saved_game_files_allocation_functions.deallocate(&allocation.allocation, files);
		unknown_0051ea14= NULL;
	}
	saved_game_files_memory_allocated= FALSE;
}

/* 0x00215880: takes the memory of the files and sets it up: no file that
comes with the game, none of a drive, and the salt of the indices. The game
does not see whether there was memory before it writes the salt in it */
void saved_game_files_new(
	struct allocation *allocation)
{
	byte *files= allocation->functions->allocate(allocation, SAVED_GAME_FILES_MEMORY_SIZE);

	if (files)
	{
		struct saved_game_file_table *table= (struct saved_game_file_table *)(files+UNKNOWN_0051EA14_TABLES_OFFSET);

		((struct built_in_saved_game_files *)files)->count= 0;
		table->count= 0;
		array_construct_elements(table->files, sizeof(struct saved_game_file_information), MAXIMUM_SAVED_GAME_FILES_PER_TABLE,
			(void *(ALLOCATION_CONVENTION *)(void *))saved_game_file_information_new);
	}
	else
	{
		files= NULL;
	}
	*(long *)(files+UNKNOWN_0051EA14_SALT_OFFSET)= saved_game_files_next_salt;
	unknown_0051ea14= files;
	saved_game_files_next_salt= (saved_game_files_next_salt+1)%(SAVED_GAME_FILE_INDEX_SALT_MASK+1);
	unknown_0055c150= SAVED_GAME_FILES_INITIAL_FLAGS;
}

/* 0x00215900: the files of a type: those of the hard disk and, if a
controller is given, those of its two memory units, and then those that come
with the game, if they are asked for. The count comes with how many there is
room for, and leaves with how many there are. With those that come with the
game they are sorted: those first, and each kind by its index.

Each drive has its table after that of the one before, though the memory has
room for only one: those of the memory units are in what is after it. The
controller is one of the four, NONE or 0xFF: no other is expected */
void saved_game_files_enumerate(
	long controller_index,
	long type,
	word *count,
	long *files,
	boolean built_in)
{
	long found= 0;

	if (unknown_0051ea14)
	{
		long unit_count= (controller_index==NONE || controller_index==SAVED_GAME_FILES_NO_CONTROLLER) ? 1 :
			1+MEMORY_UNITS_PER_CONTROLLER;
		long unit_index= 0;
		long unit;

		for (unit= 0; unit<unit_count && found<*count; unit++)
		{
			char letter;

			if (input_get_unit_drive_letter(unit_index, &letter))
			{
				const struct saved_game_file_table *table=
					(const struct saved_game_file_table *)((const byte *)unknown_0051ea14+UNKNOWN_0051EA14_TABLES_OFFSET)+
					unit_index;
				long index;

				for (index= 0; index<table->count; index++)
				{
					struct saved_game_file_information information= table->files[index];

					if (information.type==type)
					{
						unsigned long salt= *(const uint32_t *)((const byte *)unknown_0051ea14+UNKNOWN_0051EA14_SALT_OFFSET)&
							SAVED_GAME_FILE_INDEX_SALT_MASK;

						files[found++]= (long)((salt<<SAVED_GAME_FILE_INDEX_SALT_SHIFT) |
							(((unsigned long)index&SAVED_GAME_FILE_INDEX_FILE_MASK)<<SAVED_GAME_FILE_INDEX_FILE_SHIFT) |
							(((unsigned long)unit_index&SAVED_GAME_FILE_INDEX_UNIT_MASK)<<SAVED_GAME_FILE_INDEX_UNIT_SHIFT) |
							((unsigned long)type&SAVED_GAME_FILE_INDEX_TYPE_MASK));
					}
					if (found>=*count)
					{
						break;
					}
				}
			}

			/* the next drive: a memory unit of the controller, if there is
			one */
			if (controller_index==NONE || controller_index==SAVED_GAME_FILES_NO_CONTROLLER)
			{
				unit_index++;
			}
			else
			{
				unit_index= unit+1+controller_index*MEMORY_UNITS_PER_CONTROLLER;
			}
		}

		if (built_in)
		{
			const struct built_in_saved_game_files *built_in_files= unknown_0051ea14;
			long built_in_count= built_in_files->count;
			long index;

			for (index= 0; found<*count && index<built_in_count; index++)
			{
				if (built_in_files->files[index].type==type)
				{
					unsigned long salt= *(const uint32_t *)((const byte *)unknown_0051ea14+UNKNOWN_0051EA14_SALT_OFFSET)&
						SAVED_GAME_FILE_INDEX_SALT_MASK;

					files[found++]= (long)((salt<<SAVED_GAME_FILE_INDEX_SALT_SHIFT) |
						(1UL<<_saved_game_file_index_built_in_bit) |
						(((unsigned long)index&SAVED_GAME_FILE_INDEX_FILE_MASK)<<SAVED_GAME_FILE_INDEX_FILE_SHIFT) |
						((unsigned long)type&SAVED_GAME_FILE_INDEX_TYPE_MASK));
				}
			}
			qsort_4byte(files, (unsigned long)found, &built_in_count, saved_game_file_index_after, NULL);
		}
	}
	*count= (word)found;
}

/* 0x00215b40: what the game knows of a file, before it knows anything */
struct saved_game_file_information *ALLOCATION_CONVENTION saved_game_file_information_new(
	struct saved_game_file_information *information)
{
	information->path[0]= 0;
	information->name[0]= 0;

	return information;
}

/* 0x00215b50: the name that the player gave a file: empty if the file is
not known. Of one that comes with the game, the name is in the file */
wchar *saved_game_file_get_name(
	long file,
	wchar *name)
{
	name[0]= 0;
	if (unknown_0051ea14 && ((file>>_saved_game_file_index_built_in_bit)&1))
	{
		const struct built_in_saved_game_files *files= unknown_0051ea14;
		long type= file&SAVED_GAME_FILE_INDEX_TYPE_MASK;
		long place= (file>>SAVED_GAME_FILE_INDEX_FILE_SHIFT)&SAVED_GAME_FILE_INDEX_FILE_MASK;
		const byte *data;

		if (place>(long)((unsigned long)files->count-1))
		{
			return name;
		}
		data= files->files[place].data;
		if (type==_saved_game_file_type_profile)
		{
			data+= offsetof(struct player_profile, name);
		}
		else if (type>=_saved_game_file_type_slayer && type<=_saved_game_file_type_territories)
		{
			data+= GAME_VARIANT_NAME_OFFSET;
		}
		else
		{
			return name;
		}
		wcsncpy(name, (const wchar *)data, SAVED_GAME_FILE_DISPLAY_NAME_LENGTH-1);
		name[SAVED_GAME_FILE_DISPLAY_NAME_LENGTH-1]= 0;
	}
	else
	{
		struct saved_game_file_information information;

		information.path[0]= 0;
		information.name[0]= 0;
		if (saved_game_file_get_information(&information, file))
		{
			wcsncpy(name, information.name, SAVED_GAME_FILE_DISPLAY_NAME_LENGTH-1);
			name[SAVED_GAME_FILE_DISPLAY_NAME_LENGTH-1]= 0;
		}
	}

	return name;
}

/* 0x00215c00: makes a file of a type on the hard disk, with the name that
the player gives it: the saved game that the console shows with its name,
the file of the type in it, empty, its picture, and what the game knows of
it. The index of the file; NONE if it is not made, and then why is left in
saved_game_files_last_error: there are as many files as there can be, the
hard disk has no room, or something else. The game waits until the files of
the drives have been looked for */
long saved_game_file_create(
	long type,
	const wchar *name)
{
	wchar display_name[SAVED_GAME_FILE_DISPLAY_NAME_LENGTH];
	char path[SAVED_GAME_FILE_PATH_LENGTH];
	char root[SAVED_GAME_FILE_ROOT_LENGTH];
	long file= NONE;
	long saved_games;
	long language;

	while (!*(volatile boolean *)&unknown_0055c14c)
	{
		SwitchToThread();
		sound_idle();
	}
	if (!unknown_0051ea14)
	{
		return NONE;
	}
	saved_games= saved_game_files_count_saved_games(0);
	if ((unknown_0051ea14 &&
		((const struct saved_game_file_table *)((const byte *)unknown_0051ea14+UNKNOWN_0051EA14_TABLES_OFFSET))->count>=
		MAXIMUM_SAVED_GAME_FILES_PER_TABLE) || saved_games>=MAXIMUM_SAVED_GAME_FILES_PER_TABLE)
	{
		saved_game_files_last_error= _saved_game_files_error_too_many;
		return NONE;
	}
	language= saved_game_files_language;
	display_name[0]= 0;
	if (!saved_game_file_get_display_name(type, display_name, name, language))
	{
		return NONE;
	}
	memset(path, 0, sizeof(path));
	saved_game_files_get_unit_root(0, root, sizeof(root));
	if (!XCreateSaveGame(root, display_name, CREATE_NEW, 0, path, sizeof(path)))
	{
		struct saved_game_file_information information;
		char type_path[SAVED_GAME_FILE_PATH_LENGTH];
		struct file_reference reference;
		long index;

		memset(&information, 0, sizeof(information));
		type_path[0]= 0;
		wcsncpy(information.name, name, SAVED_GAME_FILE_NAME_LENGTH);
		information.name[SAVED_GAME_FILE_NAME_LENGTH]= 0;
		information.type= type;
		information.language= (byte)language;
		strncpy(information.path, path, sizeof(information.path));
		information.path[sizeof(information.path)-1]= 0;
		saved_game_file_get_type_path(type_path, information.path, type);
		if (file_reference_create_from_path(&reference, type_path, FALSE) && file_create(&reference) &&
			saved_game_file_write_image(type, path) && saved_game_file_add_information(0, &information, &index))
		{
			unsigned long salt= *(const uint32_t *)((const byte *)unknown_0051ea14+UNKNOWN_0051EA14_SALT_OFFSET)&
				SAVED_GAME_FILE_INDEX_SALT_MASK;

			file= (long)((salt<<SAVED_GAME_FILE_INDEX_SALT_SHIFT) |
				(((unsigned long)index&SAVED_GAME_FILE_INDEX_FILE_MASK)<<SAVED_GAME_FILE_INDEX_FILE_SHIFT) |
				((unsigned long)type&SAVED_GAME_FILE_INDEX_TYPE_MASK));
			if (file!=NONE)
			{
				saved_game_files_last_error= _saved_game_files_error_none;
				return file;
			}
		}
		else
		{
			/* what was made of the saved game is deleted */
			wchar delete_name[SAVED_GAME_FILE_DISPLAY_NAME_LENGTH];

			delete_name[0]= 0;
			saved_game_file_get_display_name(type, delete_name, name, (char)information.language);
			XDeleteSaveGame(root, delete_name);
		}
	}
	saved_game_files_last_error= saved_game_files_hard_disk_has_blocks(saved_game_file_type_get_blocks(type)) ?
		_saved_game_files_error_failed : _saved_game_files_error_no_room;

	return file;
}

/* 0x002164c0: looks for the files of a drive: each saved game of it that
has the file of one of the types is one, and what the game knows of it goes
to the table: its directory, its type, and the name that the player gave it,
from the name that the console shows. It stops when the table is full. A
saved game with no file of a type is none of the game's: a message is made
for it, and nothing is done with the message */
void saved_game_files_find_unit_files(
	long unit_index,
	struct saved_game_file_table *table)
{
	char root[SAVED_GAME_FILE_ROOT_LENGTH];
	struct saved_game_find_data data;
	uint32_t handle;
	char letter;

	if (!input_get_unit_drive_letter(unit_index, &letter))
	{
		return;
	}
	memset(root, 0, sizeof(root));
	saved_game_files_get_unit_root_inline(unit_index, root, sizeof(root));
	handle= XFindFirstSaveGame(root, &data);
	if (handle==(uint32_t)INVALID_HANDLE_VALUE)
	{
		return;
	}
	do
	{
		struct saved_game_file_information information;
		long type;

		memset(&information, 0, sizeof(information));
		strncpy(information.path, data.directory, sizeof(information.path));
		information.path[sizeof(information.path)-1]= 0;
		for (type= 0; type<NUMBER_OF_SAVED_GAME_FILE_TYPES; type++)
		{
			char path[SAVED_GAME_FILE_PATH_LENGTH];
			struct file_reference file;

			path[0]= 0;
			strncpy(path, information.path, SAVED_GAME_FILE_PATH_LENGTH);
			path[SAVED_GAME_FILE_PATH_LENGTH-1]= 0;
			csstrcat256(path, saved_game_file_type_get_name(type));
			file_reference_set_path(&file, path, FALSE);
			if (file_exists(&file))
			{
				break;
			}
		}
		if (type<NUMBER_OF_SAVED_GAME_FILE_TYPES)
		{
			information.type= type;
			saved_game_file_parse_display_name(data.name, type, &information.language, information.name);
			if (table->count==MAXIMUM_SAVED_GAME_FILES_PER_TABLE)
			{
				break;
			}
			table->files[table->count++]= information;
		}
		else
		{
			wchar wide_message[SAVED_GAME_FILE_MESSAGE_LENGTH];
			char message[SAVED_GAME_FILE_MESSAGE_LENGTH];
			const wchar *source= wide_message;
			char *destination= message;
			long count= SAVED_GAME_FILE_MESSAGE_LENGTH;
			wchar character;

			usnzprintf(wide_message, SAVED_GAME_FILE_MESSAGE_LENGTH,
				u"unrecognized saved game file found by XFindNextSaveGame(): display name= '%s' path= '%hs'",
				data.name, data.file_name);
			/* wchar_string_to_ascii_string, which the game has in itself
			here */
			do
			{
				character= *source;
				if (count==1)
				{
					*destination= 0;
				}
				else if (character<0x80)
				{
					*destination= (char)character;
				}
				else
				{
					*destination= '?';
				}
				source++;
				destination++;
				count--;
			} while (character && count>0);
		}
	} while (XFindNextSaveGame(handle, &data));
	XFindClose(handle);
}

/* 0x002170e0: the index that a file has now, from one that it had; NONE if
it is not known any more.

While the memory of the files is the one that the index was made with, the
index is good as long as its salt is that of the memory. When the memory was
given back and taken again, and the files that were kept are still to be
looked for: a file of the hard disk is the one of its type that has the
directory that was kept of it, and it is kept again with its new index. A
file that was not kept is not expected here: the game looks at what was kept
of it without seeing whether there is anything */
long saved_game_file_get_current_index(
	long file)
{
	const byte *files= unknown_0051ea14;
	long type= file&SAVED_GAME_FILE_INDEX_TYPE_MASK;
	long current= NONE;

	if (!saved_game_files_cache_valid)
	{
		unsigned long salt= *(const uint32_t *)(files+UNKNOWN_0051EA14_SALT_OFFSET);

		if (salt!=(unsigned long)((file>>SAVED_GAME_FILE_INDEX_SALT_SHIFT)&SAVED_GAME_FILE_INDEX_SALT_MASK))
		{
			return NONE;
		}
		if (!((file>>_saved_game_file_index_built_in_bit)&1))
		{
			return file;
		}

		return (long)(((salt&SAVED_GAME_FILE_INDEX_SALT_MASK)<<SAVED_GAME_FILE_INDEX_SALT_SHIFT) |
			(1UL<<_saved_game_file_index_built_in_bit) |
			(((unsigned long)(file>>SAVED_GAME_FILE_INDEX_FILE_SHIFT)&SAVED_GAME_FILE_INDEX_FILE_MASK)<<
				SAVED_GAME_FILE_INDEX_FILE_SHIFT) |
			(unsigned long)type);
	}
	else
	{
		const struct cached_saved_game_file *cached= cached_saved_game_file_get(file);
		char letter;

		if ((file>>_saved_game_file_index_built_in_bit)&1)
		{
			unsigned long salt= *(const uint32_t *)((const byte *)unknown_0051ea14+UNKNOWN_0051EA14_SALT_OFFSET)&
				SAVED_GAME_FILE_INDEX_SALT_MASK;

			current= (long)((salt<<SAVED_GAME_FILE_INDEX_SALT_SHIFT) |
				(1UL<<_saved_game_file_index_built_in_bit) |
				(((unsigned long)(file>>SAVED_GAME_FILE_INDEX_FILE_SHIFT)&SAVED_GAME_FILE_INDEX_FILE_MASK)<<
					SAVED_GAME_FILE_INDEX_FILE_SHIFT) |
				(unsigned long)type);
		}
		else if (files && input_get_unit_drive_letter(0, &letter))
		{
			const struct saved_game_file_table *table=
				(const struct saved_game_file_table *)(files+UNKNOWN_0051EA14_TABLES_OFFSET);
			struct saved_game_file_information information;
			long index;

			information.path[0]= 0;
			information.name[0]= 0;
			for (index= 0; index<table->count; index++)
			{
				information= table->files[index];
				if (information.type==type &&
					!strncmp(cached->information.path, information.path, sizeof(information.path)))
				{
					unsigned long salt= *(const uint32_t *)((const byte *)unknown_0051ea14+UNKNOWN_0051EA14_SALT_OFFSET)&
						SAVED_GAME_FILE_INDEX_SALT_MASK;

					current= (long)((salt<<SAVED_GAME_FILE_INDEX_SALT_SHIFT) |
						(((unsigned long)index&SAVED_GAME_FILE_INDEX_FILE_MASK)<<SAVED_GAME_FILE_INDEX_FILE_SHIFT) |
						(unsigned long)type);
					break;
				}
			}
		}
		cached_saved_game_file_delete(file);
		if (current!=NONE)
		{
			cached_saved_game_file_count++;
			cached_saved_game_files[cached_saved_game_file_count-1].file= current;
		}

		return current;
	}
}

/* 0x00215e60: deletes a file: the saved game that the console shows with
its name, if it is of a drive that is there, and what the game knows of it.
The game waits until the files of the drives have been looked for */
void saved_game_file_delete(
	long file)
{
	struct saved_game_file_information information;
	long unit_index;

	if (file==NONE)
	{
		return;
	}
	while (!*(volatile boolean *)&unknown_0055c14c)
	{
		SwitchToThread();
		sound_idle();
	}
	unit_index= (file>>SAVED_GAME_FILE_INDEX_UNIT_SHIFT)&SAVED_GAME_FILE_INDEX_UNIT_MASK;
	information.path[0]= 0;
	information.name[0]= 0;
	if (!saved_game_file_get_information(&information, file))
	{
		return;
	}
	if (!((file>>_saved_game_file_index_built_in_bit)&1))
	{
		char root[SAVED_GAME_FILE_ROOT_LENGTH]= { 0 };
		char letter;

		if (input_get_unit_drive_letter(unit_index, &letter))
		{
			wchar display_name[SAVED_GAME_FILE_DISPLAY_NAME_LENGTH];

			display_name[0]= 0;
			saved_game_file_get_display_name(file&SAVED_GAME_FILE_INDEX_TYPE_MASK, display_name, information.name,
				(char)information.language);
			XDeleteSaveGame(saved_game_files_get_unit_root(unit_index, root, sizeof(root)), display_name);
		}
	}
	saved_game_file_delete_information(file);
}

/* 0x00215f40: a name for a new file of a type that no saved game of the
hard disk has: "Halo" and a number, the first from 1 that is free. Whether
there is one: if none of the 4096 is free, or there is no hard disk, the
name is left empty */
boolean saved_game_file_get_unused_name(
	long type,
	wchar *name)
{
	name[0]= 0;
	if (input_globals.unit_mask&1)
	{
		char root[SAVED_GAME_FILE_ROOT_LENGTH]= { 0 };
		char path[SAVED_GAME_FILE_PATH_LENGTH];
		long number= 0;

		memset(path, 0, sizeof(path));
		saved_game_files_get_hard_disk_root(root);
		do
		{
			wchar display_name[SAVED_GAME_FILE_DISPLAY_NAME_LENGTH];
			long next= number+1;

			usnzprintf(name, SAVED_GAME_FILE_DISPLAY_NAME_LENGTH, u"Halo%04d", next);
			display_name[0]= 0;
			saved_game_file_get_display_name(type, display_name, name, saved_game_files_language);
			if (XCreateSaveGame(root, display_name, OPEN_EXISTING, 0, path, sizeof(path)))
			{
				break;
			}
			number= next;
		} while (number<MAXIMUM_SAVED_GAME_FILE_NAME_NUMBER);
		if (number==MAXIMUM_SAVED_GAME_FILE_NAME_NUMBER)
		{
			saved_game_files_last_error= _saved_game_files_error_too_many;
			name[0]= 0;
		}
	}

	return name[0]!=0;
}
