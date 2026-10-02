/*
FILES.H

The files of the console's disc and hard disk. As Halo 1's, with the handle
and the position kept in the reference; the names are those of Halo 1's
files.h where it has them.
*/

#ifndef __FILES_H
#define __FILES_H

#include <stdint.h>

#include "cseries/cseries.h"

#include <string.h>

/* ---------- constants */

#define FILE_REFERENCE_SIGNATURE 'filo'

enum
{
	MAXIMUM_FILENAME_LENGTH= 255,
	MAXIMUM_FIND_FILES_DEPTH= 16,
	/* MAX_PATH */
	FIND_FILES_NAME_LENGTH= 260,
	/* ERROR_HANDLE_EOF: what is set when a read gets less than it asked for */
	FILE_ERROR_END_OF_FILE= 0x26
};

/* the flags of a reference */
enum
{
	_has_filename_bit
};

/* the flags of a search */
enum
{
	/* the directories that are found are searched too */
	_find_files_recursive_bit,
	/* the directories are told as well as the files */
	_find_files_directories_bit,
	/* the files are told even if the directories are */
	_find_files_files_bit
};

/* how a file is opened */
enum
{
	_permission_read_bit,
	_permission_write_bit,
	/* start at the end */
	_permission_append_bit,
	/* others can read the file while it is written */
	_permission_shared_bit,
	/* a file that cannot be opened is no error */
	_permission_silent_bit,
	_file_open_temporary_bit,
	_file_open_delete_on_close_bit,
	_file_open_random_access_bit,
	_file_open_sequential_scan_bit
};

/* why a file could not be opened */
enum
{
	_file_open_error_none,
	_file_open_error_file_not_found,
	_file_open_error_access_denied,
	_file_open_error_path_not_found,
	_file_open_error_invalid_drive,
	_file_open_error_sharing_violation,
	_file_open_error_unknown
};

/* ---------- structures */

struct file_reference
{
	unsigned long signature;
	word flags;
	short location;
	char path[MAXIMUM_FILENAME_LENGTH+1];
	/* of the file when it is open */
	uint32_t handle;
	unsigned long position;
};

struct file_last_modification_date
{
	uint32_t low;
	uint32_t high;
};

/* what the console tells of a file that a search finds: its
WIN32_FIND_DATA */
struct find_files_data
{
	uint32_t attributes;
	struct file_last_modification_date creation_date;
	struct file_last_modification_date last_access_date;
	struct file_last_modification_date last_modification_date;
	uint32_t size_high;
	uint32_t size_low;
	uint32_t reserved[2];
	char name[FIND_FILES_NAME_LENGTH];
	char alternate_name[14];
};

/* what find_files_start sets up to go through the files of a directory */
struct find_files_state
{
	unsigned long flags;
	/* how many directories down the search is */
	short depth;
	short location;
	char path[MAXIMUM_FILENAME_LENGTH+1];
	char pattern[MAXIMUM_FILENAME_LENGTH+1];
	/* of the search in each directory; NONE if there is none */
	uint32_t handles[MAXIMUM_FIND_FILES_DEPTH];
	/* the file that was found last */
	struct find_files_data data;
};

typedef char verify_file_reference_size[
	sizeof(struct file_reference) == 0x110 ? 1 : -1];
typedef char verify_find_files_state_size[
	sizeof(struct find_files_state) == 0x388 ? 1 : -1];
typedef char verify_find_files_data_size[
	sizeof(struct find_files_data) == 0x140 ? 1 : -1];

/* ---------- inline code */

void file_path_add_name(char *path, const char *name);
void path_remove_last_element(char *path);

/* the reference of a path: of a directory, or of a file. The game has this
in the functions that make a reference, and has it as a function too
(file_reference_create_from_path) */
static inline __attribute__((always_inline)) void file_reference_set_path(
	struct file_reference *file,
	const char *path,
	boolean directory)
{
	memset(file, 0, sizeof(*file));
	file->signature= FILE_REFERENCE_SIGNATURE;
	file->location= NONE;
	if (directory)
	{
		file_path_add_name(file->path, path);
	}
	else
	{
		if (file->flags&FLAG(_has_filename_bit))
		{
			path_remove_last_element(file->path);
		}
		file_path_add_name(file->path, path);
		file->flags|= FLAG(_has_filename_bit);
	}
}

/* ---------- prototypes/FILES.C */

void file_path_add_name(char *path, const char *name);
void file_path_add_extension(char *path, const char *extension);
void file_path_split(char *path, char **directory, char **parent_directory, char **filename, char **extension,
	boolean has_filename);
void find_files_start(struct find_files_state *state, unsigned long flags, const struct file_reference *directory,
	const char *pattern);

/* ---------- prototypes/FILES_XBOX.C */

struct file_reference *file_reference_create_from_path(struct file_reference *file, const char *path,
	boolean directory);
long find_files(unsigned long flags, const struct file_reference *directory, long maximum_count,
	struct file_reference *files);
boolean file_create(struct file_reference *file);

void file_location_get_full_path(char *full_path, const char *path);
boolean file_delete(struct file_reference *file);
boolean file_exists(const struct file_reference *file);
boolean file_open(struct file_reference *file, unsigned long flags, long *error);
boolean file_close(struct file_reference *file);
boolean file_set_position(struct file_reference *file, unsigned long position, boolean silent);
boolean file_set_eof(struct file_reference *file, unsigned long position);
boolean file_read(struct file_reference *file, unsigned long count, void *buffer, boolean silent);
boolean file_write(struct file_reference *file, unsigned long count, const void *buffer);
long file_read_from_position(struct file_reference *file, unsigned long position, unsigned long count,
	boolean silent, void *buffer);
long file_write_to_position(struct file_reference *file, unsigned long position, unsigned long count,
	const void *buffer);
boolean file_get_last_modification_date(struct file_last_modification_date *date,
	const struct file_reference *file);
boolean file_get_size(const struct file_reference *file, unsigned long *size);
void find_files_end(struct find_files_state *state);
boolean find_files_next(struct find_files_state *state, struct file_reference *file,
	struct file_last_modification_date *date);

#endif
