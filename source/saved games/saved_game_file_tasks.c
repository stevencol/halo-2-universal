/*
SAVED_GAME_FILE_TASKS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The files of the players are read and written by the thread of the tasks of
async.c: here is what gives it the tasks. The names of the file and of the
functions are guesses. Not here: the work of a write (0x002acde0), and of
what follows 0x002acf80 all but four functions.
*/

#include "saved_game_files.h"

#include "cache/cache_files.h"
#include "cseries/async.h"
#include "input/input.h"
#include "saved games/game_state.h"
#include "tag_files/files.h"

#include <stdint.h>
#include <string.h>

/* ---------- constants */

enum
{
	SAVED_GAME_FILE_TASK_PRIORITY= 2,
	/* ERROR_TOO_MANY_OPEN_FILES: what a read fails with until it is known
	why */
	SAVED_GAME_FILE_ERROR_DEFAULT= 4,
	SAVED_GAME_FILE_DESCRIPTION_UNKNOWN9C= 6,

	/* what a result says of how far a read and a write got, if they fail */
	_saved_game_file_error_reading= 3,
	_saved_game_file_error_writing= 2,

	/* after the bytes of a file is its signature, which the console makes
	of them */
	SAVED_GAME_FILE_SIGNATURE_SIZE= 20,

	/* a file is copied in pieces of so many bytes */
	SAVED_GAME_FILE_COPY_PIECE_SIZE= 0x20000,
	FILE_BEGIN= 0,
	INVALID_HANDLE_VALUE= -1
};

/* ---------- structures */

/* the data of the task of a read or of a write */
struct saved_game_file_task
{
	void *buffer;
	unsigned long size;
	boolean unknown8;
	byte pad9[3];
	struct saved_game_file_result *result;
};

/* the data of the task that reads the file of the state of a saved game:
what is at its start, and the rest */
#pragma pack(push, 1)
struct saved_game_state_read_task
{
	void *header;
	long header_size;
	void *data;
	long data_size;
	long unknown10;
	long unknown14;
	long unknown18;
	struct saved_game_file_result *result;
	boolean unknown20;
	boolean unknown21;
};

/* that of the task that writes it */
struct saved_game_state_write_task
{
	long unknown0;
	void *header;
	long header_size;
	void *data;
	long data_size;
	long unknown14;
	long unknown18;
	struct saved_game_file_result *result;
	boolean unknown20;
	/* the file has nothing at its start */
	boolean no_header;
};
#pragma pack(pop)

typedef char verify_saved_game_state_read_task_size[
	sizeof(struct saved_game_state_read_task) == 0x22 ? 1 : -1];
typedef char verify_saved_game_state_write_task_size[
	sizeof(struct saved_game_state_write_task) == 0x22 ? 1 : -1];

/* ---------- prototypes of functions of the C library */

/* 0x003207ed: of characters of 16 bits */
wchar *wcsncpy(wchar *destination, const wchar *source, unsigned int count);

/* ---------- prototypes of functions that are not decompiled */

/* ---------- prototypes of functions of the console's libraries */

/* 0x002d2241, 0x002d232e */
int ReadFile(uint32_t file, void *buffer, uint32_t size, uint32_t *bytes_read, void *overlapped);
int WriteFile(uint32_t file, const void *buffer, uint32_t size, uint32_t *bytes_written, void *overlapped);

/* 0x002d2488 */
uint32_t SetFilePointer(uint32_t file, long distance, long *distance_high, uint32_t method);

/* 0x0033039a */
uint32_t XGetLanguage(void);

/* 0x00331055: deletes the saved game that the console shows with a name */
uint32_t XDeleteSaveGame(const char *root, const wchar *name);

/* 0x002d1f3a */
int CloseHandle(uint32_t handle);

/* 0x002d1d3e, 0x002d1d66 */
uint32_t GetLastError(void);
void SetLastError(uint32_t error);

/* 0x002d6765, 0x002d6779, 0x002d67c7: the signature that the console makes
of some bytes, with the key of the game: for it alone, or for any console.
The first gives what the others are told, INVALID_HANDLE_VALUE if it cannot;
the others, 0 if all went well */
uint32_t XCalculateSignatureBegin(uint32_t flags);
uint32_t XCalculateSignatureUpdate(uint32_t signature, const void *data, uint32_t size);
uint32_t XCalculateSignatureEnd(uint32_t signature, void *result);

/* 0x002acde0: the work of the task of a write */
long ASYNC_CALLBACK unknown_002acde0(struct async_custom_task *task, void *data, long size);

/* 0x002ad7c0, 0x002adb10: the work of the tasks that read and write the
file of the state of a saved game */
long ASYNC_CALLBACK unknown_002ad7c0(struct async_custom_task *task, void *data, long size);
long ASYNC_CALLBACK unknown_002adb10(struct async_custom_task *task, void *data, long size);

/* ---------- code */

static inline __attribute__((always_inline)) char *saved_game_file_result_get_path(
	struct saved_game_file_result *result)
{
	return (char *)result->unknown8;
}

/* 0x002accd0: the work of the task of a read */
long ASYNC_CALLBACK saved_game_file_read_work(
	struct async_custom_task *task,
	void *data,
	long size)
{
	struct saved_game_file_task *read= data;
	struct file_reference file;

	(void)task;
	(void)size;

	read->result->success= FALSE;
	read->result->error= SAVED_GAME_FILE_ERROR_DEFAULT;
	file_reference_set_path(&file, saved_game_file_result_get_path(read->result), FALSE);
	saved_game_file_read_signed(read->size, &file, read->buffer, read->unknown8, read->result);

	return 1;
}

/* 0x002acd60: gives the read of a file to the thread that does it. Whether
it took it */
boolean saved_game_file_read_task_new(
	void *buffer,
	unsigned long size,
	boolean unknown,
	struct saved_game_file_result *result,
	const char *path)
{
	struct saved_game_file_task task;

	task.buffer= buffer;
	result->unknown1= FALSE;
	task.size= size;
	task.unknown8= unknown;
	task.result= result;
	result->success= FALSE;
	result->progress= -1.0f;
	strncpy(saved_game_file_result_get_path(result), path, SAVED_GAME_FILE_PATH_LENGTH);
	saved_game_file_result_get_path(result)[SAVED_GAME_FILE_PATH_LENGTH-1]= 0;

	return async_task_add_custom(sizeof(task), saved_game_file_read_work, &task, SAVED_GAME_FILE_TASK_PRIORITY,
		(byte *)result)!=NONE;
}

/* 0x002acee0: gives the write of a file to the thread that does it */
boolean saved_game_file_write_task_new(
	void *unknown,
	unsigned long size,
	boolean unknown2,
	struct saved_game_file_result *result)
{
	struct saved_game_file_task task;

	task.buffer= unknown;
	result->unknown1= FALSE;
	result->success= FALSE;
	task.unknown8= unknown2;
	task.size= size;
	task.result= result;
	result->progress= -1.0f;

	return async_task_add_custom(sizeof(task), unknown_002acde0, &task, SAVED_GAME_FILE_TASK_PRIORITY,
		(byte *)result)!=NONE;
}

/* 0x002acf40: what is told of a file that is written as it is */
struct saved_game_file_description *saved_game_file_description_new(
	struct saved_game_file_description *description,
	long file,
	const struct saved_game_file_information *information)
{
	description->file= file;
	description->information.path[0]= 0;
	description->information.name[0]= 0;
	description->new_name[0]= 0;
	description->old_path[0]= 0;
	description->renamed= FALSE;
	description->unknown7C= FALSE;
	description->operation= NULL;
	description->copy_buffer= NULL;
	description->unknown9C= SAVED_GAME_FILE_DESCRIPTION_UNKNOWN9C;
	description->information= *information;

	return description;
}

/* 0x002acf80: what is told of a file that is written with another name:
the name, and where the file is now */
struct saved_game_file_description *saved_game_file_description_new_renamed(
	long file,
	const wchar *name,
	struct saved_game_file_description *description,
	const struct saved_game_file_information *information,
	void *copy_buffer,
	struct saved_game_files_operation *operation)
{
	description->file= file;
	description->information.path[0]= 0;
	description->information.name[0]= 0;
	wcsncpy(description->new_name, name, SAVED_GAME_FILE_NAME_LENGTH);
	description->new_name[SAVED_GAME_FILE_NAME_LENGTH]= 0;
	memcpy(description->old_path, information->path, sizeof(description->old_path));
	description->operation= operation;
	description->unknown7C= FALSE;
	description->source_file= (uint32_t)NONE;
	description->destination_file= (uint32_t)NONE;
	description->size= (unsigned long)NONE;
	description->read_position= 0;
	description->write_position= 0;
	description->unknown9C= 0;
	description->renamed= TRUE;
	description->copy_buffer= copy_buffer;
	description->information= *information;

	return description;
}

/* what a result has before the file of the state of a saved game is read or
written */
static inline __attribute__((always_inline)) void saved_game_state_result_start(
	struct saved_game_file_result *result,
	const char *path,
	float progress)
{
	result->unknown1= FALSE;
	result->success= FALSE;
	result->progress= progress;
	result->error= SAVED_GAME_FILE_ERROR_DEFAULT;
	strncpy(saved_game_file_result_get_path(result), path, SAVED_GAME_FILE_PATH_LENGTH);
	saved_game_file_result_get_path(result)[SAVED_GAME_FILE_PATH_LENGTH-1]= 0;
}

/* 0x002ada70: gives the read of the file of the state of a saved game to
the thread that does it: what is at its start, and the rest. Whether it took
it */
boolean saved_game_state_read_task_new(
	const char *path,
	struct saved_game_file_result *result,
	void *header,
	long header_size,
	void *data,
	long data_size,
	boolean unknown)
{
	struct saved_game_state_read_task task;

	saved_game_state_result_start(result, path, 0.0f);
	task.header= header;
	task.header_size= header_size;
	task.data= data;
	task.data_size= data_size;
	task.unknown10= NONE;
	task.unknown14= 0;
	task.unknown18= NONE;
	task.result= result;
	task.unknown20= unknown;
	task.unknown21= FALSE;

	return async_task_add_custom(sizeof(task), unknown_002ad7c0, &task, SAVED_GAME_FILE_TASK_PRIORITY,
		(byte *)result)!=NONE;
}

/* 0x002adec0: gives the write of it */
boolean saved_game_state_write_task_new(
	const char *path,
	struct saved_game_file_result *result,
	void *header,
	long header_size,
	void *data,
	long data_size,
	boolean unknown)
{
	struct saved_game_state_write_task task;

	saved_game_state_result_start(result, path, -1.0f);
	task.unknown0= 0;
	task.header= header;
	task.header_size= header_size;
	task.data= data;
	task.data_size= data_size;
	task.unknown14= NONE;
	task.unknown18= NONE;
	task.result= result;
	task.unknown20= unknown;
	task.no_header= header==NULL;

	return async_task_add_custom(sizeof(task), unknown_002adb10, &task, SAVED_GAME_FILE_TASK_PRIORITY,
		(byte *)result)!=NONE;
}

/* closes a file. The game has file_close in the two functions that follow */
static inline __attribute__((always_inline)) void saved_game_file_close(
	struct file_reference *file)
{
	if (CloseHandle(file->handle))
	{
		file->handle= 0;
		file->position= 0;
	}
	else
	{
		GetLastError();
		SetLastError(0);
	}
}

/* 0x002adf70: reads a file of so many bytes, and sees that the signature
that follows them is theirs: only then is the read told as one that went
well */
void saved_game_file_read_signed(
	unsigned long size,
	struct file_reference *file,
	void *buffer,
	boolean non_roamable,
	struct saved_game_file_result *result)
{
	/* what the console does not set if it fails is where the file is told */
	uint32_t bytes_read= (uint32_t)(uintptr_t)file;
	long error;
	boolean read= FALSE;

	if (!file_open(file, FLAG(_permission_read_bit)|FLAG(_permission_silent_bit), &error))
	{
		return;
	}
	result->error= _saved_game_file_error_reading;
	if (ReadFile(file->handle, buffer, size, &bytes_read, NULL))
	{
		if (bytes_read==size)
		{
			read= TRUE;
		}
		else
		{
			SetLastError(FILE_ERROR_END_OF_FILE);
		}
	}
	file->position+= bytes_read;
	if (read)
	{
		uint32_t signature_handle= XCalculateSignatureBegin(non_roamable!=0);

		if (signature_handle!=(uint32_t)INVALID_HANDLE_VALUE)
		{
			byte signature[SAVED_GAME_FILE_SIGNATURE_SIZE];
			byte file_signature[SAVED_GAME_FILE_SIGNATURE_SIZE];

			XCalculateSignatureUpdate(signature_handle, buffer, size);
			if (!XCalculateSignatureEnd(signature_handle, signature) &&
				file_read_from_position(file, size, sizeof(file_signature), FALSE, file_signature) &&
				!memcmp(signature, file_signature, sizeof(signature)))
			{
				result->success= TRUE;
				result->error= 0;
			}
		}
	}
	saved_game_file_close(file);
}

/* 0x002ae090: writes a file, and after its bytes their signature */
void saved_game_file_write_signed(
	struct file_reference *file,
	const void *buffer,
	unsigned long size,
	boolean non_roamable,
	struct saved_game_file_result *result)
{
	long error;

	if (!file_open(file, FLAG(_permission_write_bit), &error))
	{
		return;
	}
	result->error= _saved_game_file_error_writing;
	if (file_write(file, size, buffer))
	{
		uint32_t signature_handle= XCalculateSignatureBegin(non_roamable!=0);

		if (signature_handle!=(uint32_t)INVALID_HANDLE_VALUE)
		{
			byte signature[SAVED_GAME_FILE_SIGNATURE_SIZE];

			XCalculateSignatureUpdate(signature_handle, buffer, size);
			if (!XCalculateSignatureEnd(signature_handle, signature) &&
				file_write_to_position(file, size, sizeof(signature), signature))
			{
				result->success= TRUE;
				result->error= 0;
			}
		}
	}
	saved_game_file_close(file);
}

/* how much of a file that is copied is its next piece */
static inline __attribute__((always_inline)) unsigned long saved_game_file_copy_piece_size(
	const struct saved_game_file_description *description,
	unsigned long position)
{
	unsigned long left= description->size-position;

	return left<SAVED_GAME_FILE_COPY_PIECE_SIZE ? left : SAVED_GAME_FILE_COPY_PIECE_SIZE;
}

/* 0x002ad380: reads the next piece of a file that is copied */
boolean saved_game_file_copy_read(
	struct saved_game_file_description *description)
{
	unsigned long size= saved_game_file_copy_piece_size(description, description->read_position);
	uint32_t bytes_read;

	if (SetFilePointer(description->source_file, (long)description->read_position, NULL, FILE_BEGIN)!=
			description->read_position ||
		!ReadFile(description->source_file, description->copy_buffer, size, &bytes_read, NULL) ||
		size!=bytes_read)
	{
		return FALSE;
	}
	description->read_position+= bytes_read;

	return TRUE;
}

/* 0x002ad400: writes it */
boolean saved_game_file_copy_write(
	struct saved_game_file_description *description)
{
	unsigned long size= saved_game_file_copy_piece_size(description, description->write_position);
	uint32_t bytes_written;

	if (SetFilePointer(description->destination_file, (long)description->write_position, NULL, FILE_BEGIN)!=
			description->write_position ||
		!WriteFile(description->destination_file, description->copy_buffer, size, &bytes_written, NULL) ||
		size!=bytes_written)
	{
		return FALSE;
	}
	description->write_position+= bytes_written;

	return TRUE;
}

/* 0x002ad480: the end of giving a file another name. If it went well, the
saved game that the console shows with the old name is deleted, and the game
knows the file by the new one, in the language of the console; if not, what
is deleted is the saved game with the new name. Either way the turn is given
back */
void saved_game_file_rename_finish(
	struct saved_game_file_description *description,
	boolean renamed)
{
	long type= description->file&SAVED_GAME_FILE_INDEX_TYPE_MASK;
	char root[SAVED_GAME_FILE_ROOT_LENGTH];
	wchar display_name[SAVED_GAME_FILE_DISPLAY_NAME_LENGTH];
	long language;

	saved_game_files_get_unit_root((description->file>>SAVED_GAME_FILE_INDEX_UNIT_SHIFT)&SAVED_GAME_FILE_INDEX_UNIT_MASK,
		root, sizeof(root));
	display_name[0]= 0;
	if (renamed)
	{
		saved_game_file_get_display_name(type, display_name, description->information.name,
			(char)description->information.language);
		XDeleteSaveGame(root, display_name);
		wcsncpy(description->information.name, description->new_name, SAVED_GAME_FILE_NAME_LENGTH);
		description->information.name[SAVED_GAME_FILE_NAME_LENGTH]= 0;
		if (global_language==NONE)
		{
			global_language= language_from_xbox_language((long)XGetLanguage());
		}
		language= global_language;
		description->information.language= (byte)language;
		saved_game_file_set_information(&description->information, description->file);
	}
	else
	{
		if (global_language==NONE)
		{
			global_language= language_from_xbox_language((long)XGetLanguage());
		}
		language= global_language;
		saved_game_file_get_display_name(type, display_name, description->new_name, language);
		XDeleteSaveGame(root, display_name);
	}
	description->operation->done= TRUE;
}

/* 0x002ad5b0: closes the two files of a copy. The second is taken as open
if it is not 0, and what is then set as closed is the first again: the game
has it so */
void saved_game_file_copy_close(
	struct saved_game_file_description *description)
{
	if (description->source_file!=(uint32_t)NONE)
	{
		CloseHandle(description->source_file);
		description->source_file= (uint32_t)NONE;
	}
	if (description->destination_file)
	{
		CloseHandle(description->destination_file);
		description->source_file= (uint32_t)NONE;
	}
}
