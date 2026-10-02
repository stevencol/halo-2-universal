/*
SAVED_GAME_FILES.H

The files that the players keep on the hard disk and on the memory units:
profiles, game variants and playlists. The name of the file is Halo 1's; the
names of the functions are guesses.
*/

#ifndef __SAVED_GAME_FILES_H
#define __SAVED_GAME_FILES_H

#include "cseries/cseries.h"
#include <stdint.h>

#include "cseries/async.h"
#include "cseries/sort.h"
#include "game/game.h"
#include "memory/data.h"
#include "text/unicode.h"

/* ---------- constants */

/* what a file is: a profile, a variant of one of the games, or a playlist */
enum
{
	_saved_game_file_type_profile,
	_saved_game_file_type_slayer,
	_saved_game_file_type_koth,
	_saved_game_file_type_race,
	_saved_game_file_type_oddball,
	_saved_game_file_type_juggernaut,
	_saved_game_file_type_headhunter,
	_saved_game_file_type_ctf,
	_saved_game_file_type_assault,
	_saved_game_file_type_territories,
	_saved_game_file_type_playlist,
	NUMBER_OF_SAVED_GAME_FILE_TYPES
};

/* the game of a variant, as the variant has it */
enum
{
	_game_engine_ctf= 1,
	_game_engine_slayer,
	_game_engine_oddball,
	_game_engine_koth,
	_game_engine_juggernaut= 7,
	_game_engine_territories,
	_game_engine_assault
};

/* the games in the order that they are listed in: the seven that have
variants */
enum
{
	_game_list_slayer,
	_game_list_oddball,
	_game_list_juggernaut,
	_game_list_koth,
	_game_list_ctf,
	_game_list_assault,
	_game_list_territories,
	NUMBER_OF_LISTED_GAMES
};

enum
{
	/* how many bytes a file of each kind has */
	SAVED_GAME_FILE_PROFILE_SIZE= 0x1E0,
	SAVED_GAME_FILE_VARIANT_SIZE= 0x130,
	SAVED_GAME_FILE_PLAYLIST_SIZE= 0x15CB8,

	/* what the console counts the room of a drive in */
	SAVED_GAME_FILE_BLOCK_SIZE= 0x4000,
	/* a file takes six clusters more than its bytes: those of its directory,
	most likely */
	SAVED_GAME_FILE_EXTRA_CLUSTERS= 6,

	SAVED_GAME_FILE_ROOT_LENGTH= 8,
	SAVED_GAME_FILE_PATH_LENGTH= 256
};

/* how a file is told: the bits of its index: its type, the drive it is on
(each drive has a table of what the game knows of its files), and its place
in the table. The files with the built-in flag are not on a drive: they are
in the memory of the game */
enum
{
	SAVED_GAME_FILE_INDEX_TYPE_MASK= 0xF,
	SAVED_GAME_FILE_INDEX_UNIT_SHIFT= 4,
	SAVED_GAME_FILE_INDEX_UNIT_MASK= 0xF,
	SAVED_GAME_FILE_INDEX_FILE_SHIFT= 8,
	SAVED_GAME_FILE_INDEX_FILE_MASK= 0x1FFF,
	_saved_game_file_index_built_in_bit= 21,
	SAVED_GAME_FILE_INDEX_SALT_SHIFT= 22,
	SAVED_GAME_FILE_INDEX_SALT_MASK= 0x1FF,

	SAVED_GAME_FILE_INFORMATION_SIZE= 0x40,
	SAVED_GAME_FILE_INFORMATION_NAME_OFFSET= 0x14,

	MAXIMUM_SAVED_GAME_FILES_PER_TABLE= 4096,
	/* the names that the game gives are numbered: so many are tried */
	MAXIMUM_SAVED_GAME_FILE_NAME_NUMBER= 4096,
	/* why a file was not made: saved_game_files_last_error. There are as
	many files, or names, as there can be; the hard disk has no room for the
	file; or it has, and something else failed */
	_saved_game_files_error_none= 0,
	_saved_game_files_error_too_many= 1,
	_saved_game_files_error_no_room= 2,
	_saved_game_files_error_failed= 4,

	/* the memory of the files: those that come with the game, one table of
	those of a drive, the salt of the indices, and the buffer that a file is
	copied with */
	SAVED_GAME_FILES_MEMORY_SIZE= 0x6BF00,
	/* the configuration of the memory of the game that has it */
	_saved_game_files_memory_configuration= 1,
	/* what the allocation of the memory is told with */
	SAVED_GAME_FILES_ALLOCATION_UNKNOWN= 2,
	/* what there is to do with the files when their memory is taken */
	SAVED_GAME_FILES_INITIAL_FLAGS= 3,
	_saved_game_files_enumerate_bit= 0,
	/* a game variant has its name 4 bytes in, and a profile 8 */
	GAME_VARIANT_NAME_OFFSET= 4,
	MAXIMUM_BUILT_IN_SAVED_GAME_FILES= 101,

	/* the name that the console shows of a file, with its end: that of its
	type, a separator and the name that the player gave it */
	SAVED_GAME_FILE_DISPLAY_NAME_LENGTH= 128,
	/* the name that the player gave, with no end */
	SAVED_GAME_FILE_NAME_LENGTH= 16,
	SAVED_GAME_FILE_LANGUAGES= 9,
	SAVED_GAME_IMAGE_SIZE= 0x1000,
	MAXIMUM_CACHED_SAVED_GAME_FILES= 4,
	SAVED_GAME_FILE_MESSAGE_LENGTH= 256,
	/* the drives: the hard disk, and two memory units for each controller */
	MEMORY_UNITS_PER_CONTROLLER= 2,
	/* the files of the hard disk alone are asked for with this, or NONE */
	SAVED_GAME_FILES_NO_CONTROLLER= 0xFF,
	SAVED_GAME_FILE_NAME_COMPARE_LENGTH= 128
};

/* ---------- structures */

struct saved_game_file_result;
struct file_reference;

/* what the console tells of a saved game of a drive (XGAME_FIND_DATA): of
its directory, as of any file (WIN32_FIND_DATA), the path of the directory,
and the name that the console shows */
struct saved_game_find_data
{
	byte unknown0[0x2C];
	char file_name[0x104];
	char alternate_file_name[14];
	byte pad13E[2];
	char directory[0x104];
	wchar name[SAVED_GAME_FILE_DISPLAY_NAME_LENGTH];
};

typedef char verify_saved_game_find_data_size[
	sizeof(struct saved_game_find_data) == 0x344 ? 1 : -1];

/* what the game knows of a file */
struct saved_game_file_information
{
	char path[0x14];
	wchar name[0x12];
	long type;
	byte unknown3C;
	/* the language that the name of its type is in, in the name that the
	console shows of it; 0xFF if it is in none */
	byte language;
	byte unknown3E[2];
};

/* a file that comes with the game: it is in memory */
struct built_in_saved_game_file
{
	long type;
	byte data[SAVED_GAME_FILE_PROFILE_SIZE];
};

/* how far giving a file another name is: each time the thread is given the
work, a step is done */
enum
{
	/* the saved game with the new name is made */
	_saved_game_file_rename_create,
	/* the file of the state of the game, if the old one has it, is opened
	in both */
	_saved_game_file_rename_open_copy,
	_saved_game_file_rename_copy_read,
	_saved_game_file_rename_copy_write,
	/* the picture is written in the new one */
	_saved_game_file_rename_write_image,
	/* the old one is deleted */
	_saved_game_file_rename_finish,
	_saved_game_file_rename_done,
	NUMBER_OF_SAVED_GAME_FILE_RENAME_STATES
};

/* what is told of a file that is written, to the thread that writes it. It
is in the result of the write, after its first 8 bytes */
struct saved_game_file_description
{
	long file;
	struct saved_game_file_information information;
	/* if the file gets another name */
	wchar new_name[SAVED_GAME_FILE_NAME_LENGTH+1];
	char old_path[0x14];
	boolean renamed;
	byte pad7B;
	/* the file is written to a place of the debug game */
	boolean debug;
	byte pad7D[3];
	struct saved_game_files_operation *operation;
	/* a file with another name is copied to a new one: the two, open; NONE
	if they are not */
	uint32_t source_file;
	uint32_t destination_file;
	/* what is read from one and written to the other, a piece at a time */
	void *copy_buffer;
	unsigned long size;
	unsigned long read_position;
	unsigned long write_position;
	/* how far giving the file another name is */
	long state;
};

typedef char verify_saved_game_file_description_size[
	sizeof(struct saved_game_file_description) == 0xA0 ? 1 : -1];

/* the first of what the game knows of the files: those that come with it */
struct built_in_saved_game_files
{
	long count;
	struct built_in_saved_game_file files[1];
};

/* the files of a kind */
struct saved_game_file_table
{
	long count;
	struct saved_game_file_information files[MAXIMUM_SAVED_GAME_FILES_PER_TABLE];
};

/* what is kept of a file when the tables are not set up */
struct cached_saved_game_file
{
	/* NONE if there is none */
	long file;
	struct saved_game_file_information information;
};

typedef char verify_saved_game_file_information_size[
	sizeof(struct saved_game_file_information) == 0x40 ? 1 : -1];
typedef char verify_saved_game_file_table_size[
	sizeof(struct saved_game_file_table) == 0x40004 ? 1 : -1];

/* 0x0055c274: something that is done with the files, one thing at a time:
whether it is over, of which file it is, and the thread that asked */
struct saved_game_files_operation
{
	boolean done;
	byte pad1[3];
	long file;
	unsigned long thread;
};

/* ---------- prototypes/SAVED_GAME_FILES.C */

#define SAVED_GAME_FILES_CALLBACK __attribute__((stdcall))

void saved_game_files_update(void);
void saved_game_files_set_string_list(long string_list_index);
void SAVED_GAME_FILES_CALLBACK saved_game_files_allocate_memory(long configuration);
void saved_game_files_free_memory(void);
void saved_game_files_new(struct allocation *allocation);
void saved_game_files_find_unit_files(long unit_index, struct saved_game_file_table *table);
void saved_game_files_enumerate(long controller_index, long type, word *count, long *files, boolean built_in);
struct saved_game_file_information *ALLOCATION_CONVENTION saved_game_file_information_new(
	struct saved_game_file_information *information);
wchar *saved_game_file_get_name(long file, wchar *name);
long saved_game_file_create(long type, const wchar *name);
void saved_game_file_delete(long file);
long saved_game_file_get_current_index(long file);
boolean saved_game_file_get_unused_name(long type, wchar *name);
char *saved_game_files_get_unit_root(long unit_index, char *root, short size);
long saved_game_file_type_from_variant(const struct game_variant *variant);
long saved_game_file_type_from_listed_game(long game);
long saved_game_file_type_to_listed_game(long type);
const char *saved_game_file_type_get_name(long type);
unsigned long saved_game_file_type_get_string_id(long type);
long saved_game_file_type_get_blocks(long type);
long saved_game_file_size_to_blocks(long size);
boolean saved_game_files_hard_disk_has_blocks(long blocks);
boolean saved_game_file_get_type_path(char *path, const char *directory, long type);
long saved_game_file_type_get_size(long type);
boolean saved_game_file_get_directory(char *path, long file);
boolean saved_game_files_make_directory(const char *path);
void saved_game_files_add_default_profile(void);
void saved_game_files_add_built_in_variant(const struct game_variant *variant);
void cached_saved_game_file_delete(long file);
boolean saved_game_file_name_unique(const wchar *name, long type);
boolean saved_game_file_read(long file, void *buffer, unsigned long size);
boolean saved_game_file_read_async(unsigned long size, long file, void *buffer,
	struct saved_game_file_result *result);
boolean saved_game_file_write_async(const void *buffer, long file, void *unknown, unsigned long size,
	struct saved_game_file_result *result);
boolean saved_game_file_write(long file, unsigned long size, void *unknown, const void *buffer);
boolean saved_game_file_get_display_name(long type, wchar *display_name, const wchar *name, long language);
void saved_game_file_parse_display_name(const wchar *display_name, long type, byte *language, wchar *name);
boolean saved_game_file_write_image(long type, const char *directory);
boolean saved_game_file_name_differs(long file, const wchar *name);
boolean saved_game_file_get_information(struct saved_game_file_information *information, long file);
boolean saved_game_file_set_information(const struct saved_game_file_information *information, long file);
boolean saved_game_file_add_information(long table_index, const struct saved_game_file_information *information,
	long *index);
void saved_game_file_delete_information(long file);
boolean SORT_CALLBACK saved_game_file_index_after(long file, long other, void *unused);
long saved_game_file_index_after_delete(long file, long deleted);
void saved_game_files_take_operation(long file, struct saved_game_files_operation **operation);
long saved_game_files_count_saved_games(long unit_index);
boolean saved_game_auxiliary_read(void *header, long header_size, void *data, long data_size,
	struct saved_game_file_result *result, long saved_game);
boolean saved_game_auxiliary_write(void *header, long header_size, void *data, long data_size,
	struct saved_game_file_result *result, long saved_game);

/* ---------- prototypes/SAVED_GAME_FILE_TASKS.C */

long ASYNC_CALLBACK saved_game_file_read_work(struct async_custom_task *task, void *data, long size);
long ASYNC_CALLBACK saved_game_file_write_work(struct async_custom_task *task, void *data, long size);
boolean saved_game_file_read_task_new(void *buffer, unsigned long size, boolean unknown,
	struct saved_game_file_result *result, const char *path);
boolean saved_game_file_write_task_new(void *unknown, unsigned long size, boolean unknown2,
	struct saved_game_file_result *result);
void saved_game_file_read_signed(unsigned long size, struct file_reference *file, void *buffer,
	boolean non_roamable, struct saved_game_file_result *result);
void saved_game_file_write_signed(struct file_reference *file, const void *buffer, unsigned long size,
	boolean non_roamable, struct saved_game_file_result *result);
boolean saved_game_state_read_task_new(const char *path, struct saved_game_file_result *result, void *header,
	long header_size, void *data, long data_size, boolean unknown);
boolean saved_game_state_write_task_new(const char *path, struct saved_game_file_result *result, void *header,
	long header_size, void *data, long data_size, boolean unknown);
boolean saved_game_file_rename_create(struct saved_game_file_description *description);
boolean saved_game_file_rename_open_copy(struct saved_game_file_description *description, boolean *copy);
boolean saved_game_file_rename_step(struct saved_game_file_description *description, long *error);
void saved_game_file_description_get_path(const struct saved_game_file_description *description, char *path);
boolean saved_game_file_copy_read(struct saved_game_file_description *description);
boolean saved_game_file_copy_write(struct saved_game_file_description *description);
void saved_game_file_rename_finish(struct saved_game_file_description *description, boolean renamed);
void saved_game_file_copy_close(struct saved_game_file_description *description);
struct saved_game_file_description *saved_game_file_description_new(struct saved_game_file_description *description,
	long file, const struct saved_game_file_information *information);
struct saved_game_file_description *saved_game_file_description_new_renamed(long file, const wchar *name,
	struct saved_game_file_description *description, const struct saved_game_file_information *information,
	void *unknown, struct saved_game_files_operation *operation);

/* ---------- prototypes/SAVED_GAME_FILE_NAMES.C */

const wchar *saved_game_file_default_name_profile(long language);
const wchar *saved_game_file_default_name_slayer(long language);
const wchar *saved_game_file_default_name_koth(long language);
const wchar *saved_game_file_default_name_race(long language);
const wchar *saved_game_file_default_name_oddball(long language);
const wchar *saved_game_file_default_name_juggernaut(long language);
const wchar *saved_game_file_default_name_headhunter(long language);
const wchar *saved_game_file_default_name_ctf(long language);
const wchar *saved_game_file_default_name_assault(long language);
const wchar *saved_game_file_default_name_territories(long language);
const wchar *saved_game_file_default_name_playlist(long language);
const wchar *saved_game_file_default_name_unknown(long language);
void saved_game_file_get_default_name(unsigned long string_id, long language, wchar *name);
long saved_game_file_name_length(const wchar *name);

/* ---------- globals */

extern struct saved_game_files_operation saved_game_files_operation;
extern boolean saved_game_files_cache_valid;
extern boolean saved_game_files_memory_allocated;
extern long saved_game_files_next_salt;
extern const struct allocation_functions saved_game_files_allocation_functions;
extern long saved_game_files_last_error;
extern long saved_game_files_string_list_index;
extern long saved_game_files_language;
extern const wchar *saved_game_file_name_separator;
extern byte saved_game_image[SAVED_GAME_IMAGE_SIZE];
extern long cached_saved_game_file_count;
extern struct cached_saved_game_file cached_saved_game_files[MAXIMUM_CACHED_SAVED_GAME_FILES];

#endif
