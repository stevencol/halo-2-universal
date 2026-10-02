/*
FILES_XBOX.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The names are those of Halo 1's files_windows.c. Not here: find_files_next
(0x00137000).
*/

#include "files.h"

#include <string.h>

/* ---------- prototypes of functions of the console's libraries */

/* 0x002d1d3e */
uint32_t GetLastError(void);

/* 0x002d1d66 */
void SetLastError(uint32_t error);

/* 0x002d2750 */
uint32_t CreateFileA(const char *path, uint32_t access, uint32_t share_mode, void *security, uint32_t disposition,
	uint32_t flags, uint32_t template_file);

/* 0x002d1f3a */
int CloseHandle(uint32_t handle);

/* 0x002d2488 */
uint32_t SetFilePointer(uint32_t file, long distance, long *distance_high, uint32_t method);

/* 0x002d2404 */
int SetEndOfFile(uint32_t file);

/* 0x002d2241 */
int ReadFile(uint32_t file, void *buffer, uint32_t size, uint32_t *bytes_read, void *overlapped);

/* 0x002d232e */
int WriteFile(uint32_t file, const void *buffer, uint32_t size, uint32_t *bytes_written, void *overlapped);

/* 0x002d6b6b */
int DeleteFileA(const char *path);

/* 0x00331aa1 */
int RemoveDirectoryA(const char *path);

/* 0x002d69f8 */
int SetFileAttributesA(const char *path, uint32_t attributes);

/* 0x002d6a9e */
uint32_t GetFileAttributesA(const char *path);

/* 0x002d6ae9 */
int GetFileAttributesExA(const char *path, long level, void *data);

/* 0x00331a35 */
int CreateDirectoryA(const char *path, void *security);

/* 0x00331b9b */
uint32_t FindFirstFileA(const char *path, struct find_files_data *data);

/* 0x00331ca8 */
int FindNextFileA(uint32_t handle, struct find_files_data *data);

/* ---------- constants */

enum
{
	INVALID_HANDLE_VALUE= -1,
	INVALID_FILE_ATTRIBUTES= -1,
	INVALID_SET_FILE_POINTER= -1,

	GENERIC_READ= (int)0x80000000,
	GENERIC_WRITE= 0x40000000,
	FILE_SHARE_READ= 1,
	CREATE_ALWAYS= 2,
	OPEN_EXISTING= 3,
	FILE_BEGIN= 0,
	FILE_END= 2,

	FILE_ATTRIBUTE_DIRECTORY= 0x10,
	FILE_ATTRIBUTE_NORMAL= 0x80,
	FILE_ATTRIBUTE_TEMPORARY= 0x100,
	FILE_FLAG_DELETE_ON_CLOSE= 0x04000000,
	FILE_FLAG_SEQUENTIAL_SCAN= 0x08000000,
	FILE_FLAG_RANDOM_ACCESS= 0x10000000,

	ERROR_FILE_NOT_FOUND= 2,
	ERROR_PATH_NOT_FOUND= 3,
	ERROR_ACCESS_DENIED= 5,
	ERROR_INVALID_DRIVE= 15,
	ERROR_SHARING_VIOLATION= 32,
	ERROR_INVALID_NAME= 123
};

/* what GetFileAttributesExA tells of a file */
struct file_attribute_data
{
	uint32_t attributes;
	uint32_t creation_time[2];
	uint32_t last_access_time[2];
	uint32_t last_write_time[2];
	uint32_t size_high;
	uint32_t size_low;
};

/* ---------- code */

/* the error of the console's library is looked at and cleared. In the debug
game it was told, most likely; the game has this wherever a file fails */
static inline __attribute__((always_inline)) void file_error(
	void)
{
	GetLastError();
	SetLastError(0);
}

/* 0x001374c0: the path of a file as the console's library takes it: what
does not start with a drive is on the disc of the game */
void file_location_get_full_path(
	char *full_path,
	const char *path)
{
	char first= path[0];
	unsigned long length;

	full_path[0]= 0;
	if (!(((first>='a' && first<='z') || (first>='A' && first<='Z')) && path[1]==':' && path[2]=='\\'))
	{
		strncpy(full_path, "d:\\", MAXIMUM_FILENAME_LENGTH+1);
		full_path[MAXIMUM_FILENAME_LENGTH]= 0;
	}
	for (length= 0; length<MAXIMUM_FILENAME_LENGTH+1 && full_path[length]; length++)
	{
	}
	strncpy(full_path+length, path, MAXIMUM_FILENAME_LENGTH+1-length);
	full_path[MAXIMUM_FILENAME_LENGTH]= 0;
}

/* 0x00136860: deletes a file, or a directory that has nothing in it */
boolean file_delete(
	struct file_reference *file)
{
	char full_path[MAXIMUM_FILENAME_LENGTH+1];

	memset(full_path, 0, sizeof(full_path));
	file_location_get_full_path(full_path, file->path);
	if (TEST_FLAG(file->flags, _has_filename_bit))
	{
		if (SetFileAttributesA(full_path, FILE_ATTRIBUTE_NORMAL) && DeleteFileA(full_path))
		{
			return TRUE;
		}
	}
	else if (RemoveDirectoryA(full_path))
	{
		return TRUE;
	}
	file_error();

	return FALSE;
}

/* 0x001368f0: that there is no such file is no error */
boolean file_exists(
	const struct file_reference *file)
{
	char full_path[MAXIMUM_FILENAME_LENGTH+1];

	memset(full_path, 0, sizeof(full_path));
	file_location_get_full_path(full_path, file->path);
	if (GetFileAttributesA(full_path)!=(uint32_t)INVALID_FILE_ATTRIBUTES)
	{
		return TRUE;
	}
	if (GetLastError()!=ERROR_FILE_NOT_FOUND && GetLastError()!=ERROR_PATH_NOT_FOUND)
	{
		file_error();
	}

	return FALSE;
}

/* 0x00136970: opens a file that exists; whether it could, and why not */
boolean file_open(
	struct file_reference *file,
	unsigned long flags,
	long *error)
{
	char full_path[MAXIMUM_FILENAME_LENGTH+1];
	uint32_t access= 0;
	uint32_t share_mode= 0;
	uint32_t attributes= FILE_ATTRIBUTE_NORMAL;
	uint32_t handle;

	memset(full_path, 0, sizeof(full_path));
	*error= _file_open_error_none;
	file_location_get_full_path(full_path, file->path);
	if (TEST_FLAG(flags, _permission_read_bit))
	{
		access= (uint32_t)GENERIC_READ;
	}
	if (TEST_FLAG(flags, _permission_write_bit))
	{
		access|= GENERIC_WRITE;
	}
	if (!TEST_FLAG(flags, _permission_write_bit) || TEST_FLAG(flags, _permission_shared_bit))
	{
		share_mode= FILE_SHARE_READ;
	}
	if (TEST_FLAG(flags, _file_open_temporary_bit))
	{
		attributes= FILE_ATTRIBUTE_TEMPORARY;
	}
	if (TEST_FLAG(flags, _file_open_delete_on_close_bit))
	{
		attributes= FILE_FLAG_DELETE_ON_CLOSE;
	}
	if (TEST_FLAG(flags, _file_open_random_access_bit))
	{
		attributes= FILE_FLAG_RANDOM_ACCESS;
	}
	if (TEST_FLAG(flags, _file_open_sequential_scan_bit))
	{
		attributes= FILE_FLAG_SEQUENTIAL_SCAN;
	}

	handle= CreateFileA(full_path, access, share_mode, NULL, OPEN_EXISTING, attributes, 0);
	if (handle==(uint32_t)INVALID_HANDLE_VALUE)
	{
		switch (GetLastError())
		{
		case ERROR_FILE_NOT_FOUND:
			*error= _file_open_error_file_not_found;
			break;
		case ERROR_PATH_NOT_FOUND:
			*error= _file_open_error_path_not_found;
			break;
		case ERROR_ACCESS_DENIED:
			*error= _file_open_error_access_denied;
			break;
		case ERROR_INVALID_DRIVE:
			*error= _file_open_error_invalid_drive;
			break;
		case ERROR_SHARING_VIOLATION:
			*error= _file_open_error_sharing_violation;
			break;
		default:
			*error= _file_open_error_unknown;
			break;
		}
	}
	else
	{
		file->handle= handle;
		file->position= 0;
		if (!TEST_FLAG(flags, _permission_append_bit))
		{
			return TRUE;
		}
		file->position= SetFilePointer(handle, 0, NULL, FILE_END);
		if (file->position!=(unsigned long)INVALID_SET_FILE_POINTER)
		{
			return TRUE;
		}
		CloseHandle(file->handle);
		file->handle= 0;
		file->position= 0;
	}
	if (!TEST_FLAG(flags, _permission_silent_bit))
	{
		file_error();
	}

	return FALSE;
}

/* 0x00136bb0 */
boolean file_close(
	struct file_reference *file)
{
	if (CloseHandle(file->handle))
	{
		file->handle= 0;
		file->position= 0;

		return TRUE;
	}
	file_error();

	return FALSE;
}

/* 0x00136bf0: nothing is asked of the console if the file is there already */
boolean file_set_position(
	struct file_reference *file,
	unsigned long position,
	boolean silent)
{
	boolean success;

	if (file->position==position)
	{
		return TRUE;
	}
	file->position= SetFilePointer(file->handle, (long)position, NULL, FILE_BEGIN);
	success= file->position!=(unsigned long)INVALID_SET_FILE_POINTER;
	if (!success && !silent)
	{
		file_error();
	}

	return success;
}

/* 0x00136c40: the file ends at a position */
boolean file_set_eof(
	struct file_reference *file,
	unsigned long position)
{
	if (file->position!=position)
	{
		file->position= SetFilePointer(file->handle, (long)position, NULL, FILE_BEGIN);
		if (file->position==(unsigned long)INVALID_SET_FILE_POINTER)
		{
			file_error();
			file_error();

			return FALSE;
		}
	}
	if (SetEndOfFile(file->handle))
	{
		return TRUE;
	}
	file_error();

	return FALSE;
}

/* 0x00136ca0: whether all of it was read */
boolean file_read(
	struct file_reference *file,
	unsigned long count,
	void *buffer,
	boolean silent)
{
	uint32_t bytes_read= (uint32_t)(uintptr_t)buffer;
	boolean success= FALSE;

	if (ReadFile(file->handle, buffer, count, &bytes_read, NULL))
	{
		if (bytes_read==count)
		{
			success= TRUE;
		}
		else
		{
			SetLastError(FILE_ERROR_END_OF_FILE);
		}
	}
	file->position+= bytes_read;
	if (!success && !silent)
	{
		file_error();
	}

	return success;
}

/* 0x00136d00 */
boolean file_write(
	struct file_reference *file,
	unsigned long count,
	const void *buffer)
{
	uint32_t bytes_written= (uint32_t)(uintptr_t)buffer;
	boolean success= FALSE;

	if (WriteFile(file->handle, buffer, count, &bytes_written, NULL) && bytes_written==count)
	{
		success= TRUE;
	}
	file->position+= bytes_written;
	if (!success)
	{
		file_error();
	}

	return success;
}

/* 0x00136d50 */
long file_read_from_position(
	struct file_reference *file,
	unsigned long position,
	unsigned long count,
	boolean silent,
	void *buffer)
{
	return file_set_position(file, position, silent) && file_read(file, count, buffer, silent);
}

/* 0x00136d90 */
long file_write_to_position(
	struct file_reference *file,
	unsigned long position,
	unsigned long count,
	const void *buffer)
{
	if (file->position!=position)
	{
		file->position= SetFilePointer(file->handle, (long)position, NULL, FILE_BEGIN);
		if (file->position==(unsigned long)INVALID_SET_FILE_POINTER)
		{
			file_error();

			return FALSE;
		}
	}

	return file_write(file, count, buffer);
}

/* 0x00136df0: when the file was last written; whether it could be told */
boolean file_get_last_modification_date(
	struct file_last_modification_date *date,
	const struct file_reference *file)
{
	char full_path[MAXIMUM_FILENAME_LENGTH+1];
	struct file_attribute_data data;

	memset(full_path, 0, sizeof(full_path));
	date->low= 0;
	date->high= 0;
	file_location_get_full_path(full_path, file->path);
	if (GetFileAttributesExA(full_path, 0, &data))
	{
		date->low= data.last_write_time[0];
		date->high= data.last_write_time[1];

		return TRUE;
	}
	file_error();

	return FALSE;
}

/* 0x00136e90: the low 32 bits of it */
boolean file_get_size(
	const struct file_reference *file,
	unsigned long *size)
{
	char full_path[MAXIMUM_FILENAME_LENGTH+1];
	struct file_attribute_data data;

	memset(full_path, 0, sizeof(full_path));
	file_location_get_full_path(full_path, file->path);
	if (GetFileAttributesExA(full_path, 0, &data))
	{
		*size= data.size_low;

		return TRUE;
	}
	file_error();

	return FALSE;
}

/* 0x00136fc0: ends a search for files: closes what the console has open
for it */
void find_files_end(
	struct find_files_state *state)
{
	long depth;

	for (depth= state->depth; depth>=0; depth--)
	{
		if (state->handles[depth]!=(uint32_t)NONE)
		{
			CloseHandle(state->handles[depth]);
			state->handles[depth]= (uint32_t)NONE;
		}
	}
}

/* the reference of something that a search found: the directory where the
search is */
static inline __attribute__((always_inline)) void find_files_reference(
	const struct find_files_state *state,
	struct file_reference *file)
{
	memset(file, 0, sizeof(*file));
	file->signature= FILE_REFERENCE_SIGNATURE;
	file->location= state->location;
	file_path_add_name(file->path, state->path);
}

/* 0x00137000: the next file of a search, with the date it was last written
if it is asked for. FALSE when there are no more. The search goes down the
directories it finds, if it is told to, and back up when one is done: the
path of the state is that of the directory where it is */
boolean find_files_next(
	struct find_files_state *state,
	struct file_reference *file,
	struct file_last_modification_date *date)
{
	char full_path[MAXIMUM_FILENAME_LENGTH+1];
	long depth= (word)state->depth;

	memset(full_path, 0, sizeof(full_path));
	while ((short)depth>=0)
	{
		uint32_t *handle= &state->handles[(short)depth];
		boolean found;

		if (*handle==(uint32_t)NONE)
		{
			file_location_get_full_path(full_path, state->path);
			file_path_add_name(full_path, state->pattern);
			*handle= FindFirstFileA(full_path, &state->data);
			found= *handle!=(uint32_t)NONE;
		}
		else
		{
			found= FindNextFileA(*handle, &state->data)!=0;
			if (!found)
			{
				CloseHandle(*handle);
				*handle= (uint32_t)NONE;
			}
		}

		if (!found)
		{
			/* this directory is done */
			path_remove_last_element(state->path);
			depth--;
		}
		else if (state->data.attributes&FILE_ATTRIBUTE_DIRECTORY)
		{
			if (memcmp(state->data.name, ".", 2)!=0 && memcmp(state->data.name, "..", 3)!=0)
			{
				boolean entered= FALSE;

				if (state->flags&FLAG(_find_files_recursive_bit))
				{
					file_path_add_name(state->path, state->data.name);
					depth++;
					entered= TRUE;
				}
				if (state->flags&(FLAG(_find_files_directories_bit)|FLAG(_find_files_files_bit)))
				{
					find_files_reference(state, file);
					if (!entered)
					{
						file_path_add_name(file->path, state->data.name);
					}
					if (date)
					{
						*date= state->data.last_modification_date;
					}
					state->depth= (short)depth;

					return TRUE;
				}
			}
		}
		else if (state->flags&FLAG(_find_files_files_bit) || !(state->flags&FLAG(_find_files_directories_bit)))
		{
			find_files_reference(state, file);
			if (file->flags&FLAG(_has_filename_bit))
			{
				path_remove_last_element(file->path);
			}
			file_path_add_name(file->path, state->data.name);
			file->flags|= FLAG(_has_filename_bit);
			if (date)
			{
				*date= state->data.last_modification_date;
			}
			state->depth= (short)depth;

			return TRUE;
		}
	}
	state->depth= (short)depth;

	return FALSE;
}

/* 0x00136710: the reference of a path: of a directory, or of a file. The
name is Halo 1's */
struct file_reference *file_reference_create_from_path(
	struct file_reference *file,
	const char *path,
	boolean directory)
{
	file_reference_set_path(file, path, directory);

	return file;
}

/* 0x00136770: the files of a directory, up to so many: how many there are.
The name is Halo 1's */
long find_files(
	unsigned long flags,
	const struct file_reference *directory,
	long maximum_count,
	struct file_reference *files)
{
	struct find_files_state state;
	long count= 0;

	find_files_start(&state, flags, directory, "*.*");
	if (maximum_count>0)
	{
		while (find_files_next(&state, &files[count], NULL))
		{
			count++;
			if (count>=maximum_count)
			{
				break;
			}
		}
	}
	find_files_end(&state);

	return count;
}

/* 0x001367d0: makes a file, empty, or a directory. A file that is there is
emptied */
boolean file_create(
	struct file_reference *file)
{
	char full_path[MAXIMUM_FILENAME_LENGTH+1];

	memset(full_path, 0, sizeof(full_path));
	file_location_get_full_path(full_path, file->path);
	if (TEST_FLAG(file->flags, _has_filename_bit))
	{
		uint32_t handle= CreateFileA(full_path, (uint32_t)GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

		if (handle!=(uint32_t)INVALID_HANDLE_VALUE)
		{
			CloseHandle(handle);

			return TRUE;
		}
	}
	else if (CreateDirectoryA(full_path, NULL))
	{
		return TRUE;
	}
	file_error();

	return FALSE;
}
