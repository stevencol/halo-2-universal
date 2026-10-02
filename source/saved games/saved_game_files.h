/*
SAVED_GAME_FILES.H

The files that the players keep on the hard disk and on the memory units:
profiles, game variants and playlists. The name of the file is Halo 1's; the
names of the functions are guesses.
*/

#ifndef __SAVED_GAME_FILES_H
#define __SAVED_GAME_FILES_H

#include "cseries/cseries.h"
#include "game/game.h"

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

/* how a file is told: the bits of its index. The files with the built-in
flag are not on a drive: they are in the memory of the game */
enum
{
	SAVED_GAME_FILE_INDEX_UNIT_MASK= 0xF,
	SAVED_GAME_FILE_INDEX_TYPE_SHIFT= 4,
	SAVED_GAME_FILE_INDEX_TYPE_MASK= 0xF,
	SAVED_GAME_FILE_INDEX_FILE_SHIFT= 8,
	SAVED_GAME_FILE_INDEX_FILE_MASK= 0x1FFF,
	_saved_game_file_index_built_in_bit= 21,
	SAVED_GAME_FILE_INDEX_SALT_SHIFT= 22,
	SAVED_GAME_FILE_INDEX_SALT_MASK= 0x1FF,

	/* what is told of a file, by unknown_00216800: its directory first */
	SAVED_GAME_FILE_INFORMATION_SIZE= 0x40,
	SAVED_GAME_FILE_INFORMATION_NAME_OFFSET= 0x14
};

/* ---------- structures */

struct saved_game_file_result;

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
boolean saved_game_file_index_after(long file, long other, void *unused);
long saved_game_file_index_after_delete(long file, long deleted);
void saved_game_files_take_operation(long file, struct saved_game_files_operation **operation);
long saved_game_files_count_saved_games(long unit_index);
boolean saved_game_auxiliary_read(void *header, long header_size, void *data, long data_size,
	struct saved_game_file_result *result, long saved_game);
boolean saved_game_auxiliary_write(void *header, long header_size, void *data, long data_size,
	struct saved_game_file_result *result, long saved_game);

/* ---------- globals */

extern struct saved_game_files_operation saved_game_files_operation;

#endif
