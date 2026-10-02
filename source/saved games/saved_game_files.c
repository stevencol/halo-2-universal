/*
SAVED_GAME_FILES.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The files that the players keep: profiles, variants of the games and
playlists. The names of the functions are guesses. Not here: most of the
file, which goes from 0x002160b0 to 0x00217590.
*/

#include "saved_game_files.h"

#include "input/input.h"
#include "tag_files/files.h"
#include "cseries/async.h"
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

/* 0x00331125, 0x0033122f, 0x00331276: go through the saved games of a drive.
What they tell of each (XGAME_FIND_DATA) is not looked at */
uint32_t XFindFirstSaveGame(const char *root, void *data);
int XFindNextSaveGame(uint32_t handle, void *data);
int XFindClose(uint32_t handle);

/* ---------- prototypes of functions that are not decompiled */

/* 0x002ada70, 0x002adec0: start to read and to write a file in two parts.
Whether it was started */
boolean unknown_002ada70(const char *path, struct saved_game_file_result *result, void *header, long header_size,
	void *data, long data_size, long unknown);
boolean unknown_002adec0(const char *path, struct saved_game_file_result *result, void *header, long header_size,
	void *data, long data_size, long unknown);

/* ---------- constants */

enum
{
	/* in unknown_0051ea14: the tables of the files, and a number that goes
	into the index of each file: it is where the count of the second table
	is */
	UNKNOWN_0051EA14_TABLES_OFFSET= 0xBEF8,
	UNKNOWN_0051EA14_SALT_OFFSET= 0x4BEFC,

	/* what XFindFirstSaveGame fills */
	XGAME_FIND_DATA_SIZE= 0x33C,
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

/* 0x00321ce4: of characters of 16 bits */
int wcsncmp(const wchar *a, const wchar *b, unsigned int count);

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
boolean saved_game_file_index_after(
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
if it was after it among those of its type, with a new salt */
long saved_game_file_index_after_delete(
	long file,
	long deleted)
{
	const byte *files= unknown_0051ea14;
	long type= (file>>SAVED_GAME_FILE_INDEX_TABLE_SHIFT)&SAVED_GAME_FILE_INDEX_TABLE_MASK;
	long index= (file>>SAVED_GAME_FILE_INDEX_FILE_SHIFT)&SAVED_GAME_FILE_INDEX_FILE_MASK;

	if (!((file>>_saved_game_file_index_built_in_bit)&1) && files &&
		((deleted>>SAVED_GAME_FILE_INDEX_TABLE_SHIFT)&SAVED_GAME_FILE_INDEX_TABLE_MASK)==type &&
		index>((deleted>>SAVED_GAME_FILE_INDEX_FILE_SHIFT)&SAVED_GAME_FILE_INDEX_FILE_MASK))
	{
		unsigned long salt= *(const uint32_t *)(files+UNKNOWN_0051EA14_SALT_OFFSET)&SAVED_GAME_FILE_INDEX_SALT_MASK;

		return (long)((salt<<SAVED_GAME_FILE_INDEX_SALT_SHIFT) |
			((unsigned long)type<<SAVED_GAME_FILE_INDEX_TABLE_SHIFT) |
			(((unsigned long)(index-1)&SAVED_GAME_FILE_INDEX_FILE_MASK)<<SAVED_GAME_FILE_INDEX_FILE_SHIFT) |
			((unsigned long)file&SAVED_GAME_FILE_INDEX_UNIT_MASK));
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

	return unknown_002ada70(path, result, header, header_size, data, data_size, 0);
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

	return unknown_002adec0(path, result, header, header_size, data, data_size, 0);
}

/* 0x00217590: how many saved games a drive has */
long saved_game_files_count_saved_games(
	long unit_index)
{
	char root[SAVED_GAME_FILE_ROOT_LENGTH]= { 0 };
	byte data[XGAME_FIND_DATA_SIZE];
	long count= 0;
	uint32_t handle;

	saved_game_files_get_unit_root(unit_index, root, sizeof(root));
	handle= XFindFirstSaveGame(root, data);
	if (handle!=(uint32_t)INVALID_HANDLE_VALUE)
	{
		do
		{
			count++;
		} while (XFindNextSaveGame(handle, data));
		XFindClose(handle);
	}

	return count;
}

/* the table that a file of a drive is in, and its place in it */
static inline __attribute__((always_inline)) struct saved_game_file_table *saved_game_file_get_table(
	long file)
{
	return (struct saved_game_file_table *)((byte *)unknown_0051ea14+UNKNOWN_0051EA14_TABLES_OFFSET)+
		((file>>SAVED_GAME_FILE_INDEX_TABLE_SHIFT)&SAVED_GAME_FILE_INDEX_TABLE_MASK);
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
