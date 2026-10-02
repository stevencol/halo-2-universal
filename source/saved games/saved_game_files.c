/*
SAVED_GAME_FILES.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The files that the players keep: profiles, variants of the games and
playlists. The names of the functions are guesses. Not here: most of the
file, from 0x002160b0 to 0x00217590.
*/

#include "saved_game_files.h"

#include "input/input.h"

#include <stdint.h>
#include <string.h>

/* ---------- prototypes of functions of the console's libraries */

/* 0x002d5f65: how many bytes a cluster of a drive has */
uint32_t XGetDiskClusterSize(const char *root);

/* 0x002d5838 */
int GetDiskFreeSpaceExA(const char *directory, uint64_t *free_bytes_available, uint64_t *total_bytes,
	uint64_t *total_free_bytes);

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
