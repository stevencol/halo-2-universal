/*
SAVED_GAME_FILE_TASKS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The files of the players are read and written by the thread of the tasks of
async.c: here is what gives it the tasks. The names of the file and of the
functions are guesses. Not here: 0x002adb10, the work of the task that
writes the file of the state of a saved game.
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
	/* what a rename that does not go on says of where it stopped */
	_saved_game_file_rename_error_step= 2,
	_saved_game_file_rename_error_copy= 4,

	/* what a result says of how far a read and a write got, if they fail */
	_saved_game_file_error_reading= 3,
	_saved_game_file_error_writing= 2,

	/* after the bytes of a file is its signature, which the console makes
	of them */
	SAVED_GAME_FILE_SIGNATURE_SIZE= 20,

	/* a file is copied in pieces of so many bytes */
	SAVED_GAME_FILE_COPY_PIECE_SIZE= 0x20000,
	FILE_BEGIN= 0,
	GENERIC_READ= (int)0x80000000,
	GENERIC_WRITE= 0x40000000,
	CREATE_NEW= 1,
	CREATE_ALWAYS= 2,
	OPEN_EXISTING= 3,
	OPEN_ALWAYS= 4,
	FILE_FLAG_SEQUENTIAL_SCAN= 0x08000000,
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
	/* the file, open; NONE if it is not */
	uint32_t file;
	/* how much of the rest has been read */
	unsigned long position;
	/* the signature of what has been read, which the console is making;
	NONE if it is not */
	uint32_t signature;
	struct saved_game_file_result *result;
	/* the signature is for this console alone */
	boolean non_roamable;
	/* how far the read is */
	char step;
};

enum
{
	_saved_game_state_read_header,
	_saved_game_state_read_data,
	_saved_game_state_read_signature
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

/* 0x002d2750 */
uint32_t CreateFileA(const char *path, uint32_t access, uint32_t share_mode, void *security, uint32_t disposition,
	uint32_t flags, uint32_t template_file);

/* 0x002d2715 */
uint32_t GetFileSize(uint32_t file, uint32_t *size_high);

/* 0x002d2404 */
int SetEndOfFile(uint32_t file);

/* 0x00330df2: makes the directory of a saved game, or finds the one that
there is, by the name that the console shows of it. 0 if it does */
uint32_t XCreateSaveGame(const char *root, const wchar *name, uint32_t disposition, uint32_t flags, char *path,
	uint32_t path_size);

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

/* 0x002adb10: the work of the task that writes the file of the state of a
saved game */
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

	return async_task_add_custom(sizeof(task), saved_game_file_write_work, &task, SAVED_GAME_FILE_TASK_PRIORITY,
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
	description->debug= FALSE;
	description->operation= NULL;
	description->copy_buffer= NULL;
	description->state= _saved_game_file_rename_done;
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
	description->debug= FALSE;
	description->source_file= (uint32_t)NONE;
	description->destination_file= (uint32_t)NONE;
	description->size= (unsigned long)NONE;
	description->read_position= 0;
	description->write_position= 0;
	description->state= _saved_game_file_rename_create;
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

/* a number with no sign, as a real: the game takes it as one with a sign and
adds 4294967296 to it if it is negative. With the precision that the game
runs with, the sum is rounded: it is not the same as taking the number at
once */
static inline __attribute__((always_inline)) long double saved_game_state_unsigned_to_real(
	unsigned long value)
{
	long double real= (long double)(long)value;

	if ((long)value<0)
	{
		real+= 4294967296.0f;
	}

	return real;
}

/* closes what the task that reads the file of the state of a saved game has
open, and ends it */
static inline __attribute__((always_inline)) long saved_game_state_read_work_end(
	struct saved_game_state_read_task *read)
{
	if (read->file!=(uint32_t)INVALID_HANDLE_VALUE)
	{
		CloseHandle(read->file);
	}
	if (read->signature!=(uint32_t)INVALID_HANDLE_VALUE)
	{
		XCalculateSignatureEnd(read->signature, NULL);
	}

	return 1;
}

/* 0x002ad7c0: the work of the task that reads the file of the state of a
saved game, a step each time that the thread gives it the turn: 0 while
there is more to do. First the file is opened and what is at its start is
read; then the rest, a piece at a time if there is nowhere to read it to
(then it goes to the buffer of the tasks, as much as fits); and last the
signature that is after it, which has to be that of what was read. The
result says how far the read is, and whether it went well */
long ASYNC_CALLBACK saved_game_state_read_work(
	struct async_custom_task *task,
	void *data,
	long size)
{
	struct saved_game_state_read_task *read= data;
	float total= (float)saved_game_state_unsigned_to_real(
		(unsigned long)(read->data_size+SAVED_GAME_FILE_SIGNATURE_SIZE+read->header_size));
	uint32_t bytes_read;

	(void)size;

	read->result->error= SAVED_GAME_FILE_ERROR_DEFAULT;
	if (read->result->unknown1)
	{
		return saved_game_state_read_work_end(read);
	}
	if (read->step==_saved_game_state_read_header)
	{
		read->file= CreateFileA(saved_game_file_result_get_path(read->result), (uint32_t)GENERIC_READ, 0, NULL,
			OPEN_ALWAYS, 0, 0);
		if (read->file==(uint32_t)INVALID_HANDLE_VALUE)
		{
			return saved_game_state_read_work_end(read);
		}
		if (!ReadFile(read->file, read->header, (uint32_t)read->header_size, &bytes_read, NULL) ||
			bytes_read!=(uint32_t)read->header_size)
		{
			return saved_game_state_read_work_end(read);
		}
		read->signature= XCalculateSignatureBegin(read->non_roamable!=0);
		if (read->signature==(uint32_t)INVALID_HANDLE_VALUE)
		{
			return saved_game_state_read_work_end(read);
		}
		if (read->header_size && XCalculateSignatureUpdate(read->signature, read->header, (uint32_t)read->header_size))
		{
			return saved_game_state_read_work_end(read);
		}
		read->result->progress=
			(float)(saved_game_state_unsigned_to_real((unsigned long)read->header_size)/(long double)total);
		read->step= _saved_game_state_read_data;

		return 0;
	}
	else if (read->step==_saved_game_state_read_data)
	{
		unsigned long position= read->position;
		unsigned long count;
		byte *buffer;
		uint32_t file_position;

		if (position>=(unsigned long)read->data_size)
		{
			read->step= _saved_game_state_read_signature;

			return 0;
		}
		if (read->data)
		{
			buffer= (byte *)read->data+position;
			count= (unsigned long)read->data_size-position;
		}
		else
		{
			buffer= async_buffer_globals.address;
			count= async_buffer_globals.size;
			async_buffer_globals.unknown0= (long)task;
			async_buffer_globals.other_buffer= buffer!=async_buffer;
			position= read->position;
			if (count>(unsigned long)read->data_size-position)
			{
				count= (unsigned long)read->data_size-position;
			}
		}
		file_position= (uint32_t)read->header_size+position;
		if (SetFilePointer(read->file, (long)file_position, NULL, FILE_BEGIN)!=file_position)
		{
			return saved_game_state_read_work_end(read);
		}
		if (!ReadFile(read->file, buffer, count, &bytes_read, NULL) || bytes_read!=count)
		{
			return saved_game_state_read_work_end(read);
		}
		if (XCalculateSignatureUpdate(read->signature, buffer, bytes_read))
		{
			return saved_game_state_read_work_end(read);
		}
		read->position+= bytes_read;
		if (read->position>=(unsigned long)read->data_size)
		{
			read->step= _saved_game_state_read_signature;
		}
		read->result->progress=
			(float)(saved_game_state_unsigned_to_real((unsigned long)read->header_size+read->position)/(long double)total);

		return 0;
	}
	else
	{
		uint32_t stored[SAVED_GAME_FILE_SIGNATURE_SIZE/sizeof(uint32_t)];
		uint32_t signature[SAVED_GAME_FILE_SIGNATURE_SIZE/sizeof(uint32_t)];
		uint32_t file_position= (uint32_t)(read->data_size+read->header_size);

		if (SetFilePointer(read->file, (long)file_position, NULL, FILE_BEGIN)==file_position &&
			ReadFile(read->file, stored, sizeof(stored), &bytes_read, NULL) && bytes_read==sizeof(stored))
		{
			if (!XCalculateSignatureEnd(read->signature, signature))
			{
				read->result->success= memcmp(signature, stored, sizeof(stored))==0;
			}
			read->signature= (uint32_t)INVALID_HANDLE_VALUE;
		}
		read->result->progress= 1.0f;

		return saved_game_state_read_work_end(read);
	}
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
	task.file= (uint32_t)NONE;
	task.position= 0;
	task.signature= (uint32_t)NONE;
	task.result= result;
	task.non_roamable= unknown;
	task.step= _saved_game_state_read_header;

	return async_task_add_custom(sizeof(task), saved_game_state_read_work, &task, SAVED_GAME_FILE_TASK_PRIORITY,
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

/* closes the two files of a copy. The second is taken as open if it is not
0, and what is then set as closed is the first again: the game has it so, in
the function that follows and where a rename stops */
static inline __attribute__((always_inline)) void saved_game_file_copy_close_inline(
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

/* 0x002ad5b0 */
void saved_game_file_copy_close(
	struct saved_game_file_description *description)
{
	saved_game_file_copy_close_inline(description);
}

/* 0x002ad5f0: one more step of giving a file another name. Whether it is
all done. If a step fails, what was done is undone, and why is told: the
state is left as it was */
boolean saved_game_file_rename_step(
	struct saved_game_file_description *description,
	long *error)
{
	*error= 0;
	switch (description->state)
	{
	case _saved_game_file_rename_create:
		if (saved_game_file_rename_create(description))
		{
			description->state= _saved_game_file_rename_write_image;
		}
		else
		{
			*error= _saved_game_file_rename_error_step;
		}
		break;
	case _saved_game_file_rename_write_image:
		if (saved_game_file_write_image(description->file&SAVED_GAME_FILE_INDEX_TYPE_MASK, description->information.path))
		{
			description->state= _saved_game_file_rename_open_copy;
		}
		else
		{
			*error= _saved_game_file_rename_error_step;
		}
		break;
	case _saved_game_file_rename_open_copy:
	{
		boolean copy= FALSE;

		if (!saved_game_file_rename_open_copy(description, &copy))
		{
			*error= _saved_game_file_rename_error_step;
		}
		else if (copy)
		{
			description->read_position= 0;
			description->write_position= 0;
			description->state= _saved_game_file_rename_copy_read;
		}
		else
		{
			description->state= _saved_game_file_rename_finish;
		}
		break;
	}
	case _saved_game_file_rename_copy_read:
		if (saved_game_file_copy_read(description))
		{
			description->state= _saved_game_file_rename_copy_write;
		}
		else
		{
			*error= _saved_game_file_rename_error_copy;
		}
		break;
	case _saved_game_file_rename_copy_write:
		if (!saved_game_file_copy_write(description))
		{
			*error= _saved_game_file_rename_error_copy;
		}
		else if (description->size>description->write_position)
		{
			description->state= _saved_game_file_rename_copy_read;
		}
		else
		{
			saved_game_file_copy_close(description);
			description->state= _saved_game_file_rename_finish;
		}
		break;
	case _saved_game_file_rename_finish:
		saved_game_file_rename_finish(description, TRUE);
		description->state= _saved_game_file_rename_done;
		break;
	case _saved_game_file_rename_done:
		return TRUE;
	default:
		return FALSE;
	}

	if (*error)
	{
		saved_game_file_copy_close_inline(description);
		saved_game_file_rename_finish(description, FALSE);
	}

	return FALSE;
}

/* 0x002ad760: the path of the file that is written: that of its type, in
its directory */
void saved_game_file_description_get_path(
	const struct saved_game_file_description *description,
	char *path)
{
	if (description->debug)
	{
		strncpy(path, "t:\\blam", SAVED_GAME_FILE_PATH_LENGTH);
		path[SAVED_GAME_FILE_PATH_LENGTH-1]= 0;
	}
	else
	{
		long type= description->information.type;

		strncpy(path, description->information.path, SAVED_GAME_FILE_PATH_LENGTH);
		path[SAVED_GAME_FILE_PATH_LENGTH-1]= 0;
		csstrcat256(path, saved_game_file_type_get_name(type));
	}
}

/* 0x002acde0: the work of the task of a write. First the file is given its
new name, if it has one, a step each time the work is done: the work is not
over until that is, and it is over at once if that fails. Then the file is
written, with its signature */
long ASYNC_CALLBACK saved_game_file_write_work(
	struct async_custom_task *task,
	void *data,
	long size)
{
	struct saved_game_file_task *write= data;
	struct saved_game_file_description *description=
		(struct saved_game_file_description *)write->result->unknown8;
	long error;

	(void)task;
	(void)size;

	if (!saved_game_file_rename_step(description, &error))
	{
		if (error)
		{
			write->result->error= 0;

			return 1;
		}

		return 0;
	}
	if (!error)
	{
		char path[SAVED_GAME_FILE_PATH_LENGTH];
		struct file_reference file;

		write->result->error= SAVED_GAME_FILE_ERROR_DEFAULT;
		path[0]= 0;
		saved_game_file_description_get_path(description, path);
		file_reference_set_path(&file, path, FALSE);
		saved_game_file_write_signed(&file, write->buffer, write->size, write->unknown8, write->result);
	}

	return 1;
}

/* the path of the file of the state of the game of a saved game, by the
directory of the saved game */
static inline __attribute__((always_inline)) void saved_game_file_get_state_path(
	char *path,
	const char *directory)
{
	strncpy(path, directory, SAVED_GAME_FILE_PATH_LENGTH);
	path[SAVED_GAME_FILE_PATH_LENGTH-1]= 0;
	csstrcat256(path, "auxilary.bin");
}

/* 0x002ad020: the first step of giving a file another name: makes the saved
game with the new name, and in it the file, empty but with all its size. The
directory of the file is then that of the new saved game */
boolean saved_game_file_rename_create(
	struct saved_game_file_description *description)
{
	long type= description->file&SAVED_GAME_FILE_INDEX_TYPE_MASK;
	char root[SAVED_GAME_FILE_ROOT_LENGTH];
	char directory[sizeof(description->information.path)];
	char path[SAVED_GAME_FILE_PATH_LENGTH];
	char save_path[SAVED_GAME_FILE_PATH_LENGTH];
	wchar display_name[SAVED_GAME_FILE_DISPLAY_NAME_LENGTH];
	struct file_reference file;
	long language;
	long error;
	boolean created;

	if (global_language==NONE)
	{
		global_language= language_from_xbox_language((long)XGetLanguage());
	}
	language= global_language;
	memset(save_path, 0, sizeof(save_path));
	display_name[0]= 0;
	saved_game_file_get_display_name(type, display_name, description->new_name, language);
	saved_game_files_get_unit_root((description->file>>SAVED_GAME_FILE_INDEX_UNIT_SHIFT)&SAVED_GAME_FILE_INDEX_UNIT_MASK,
		root, sizeof(root));
	if (XCreateSaveGame(root, display_name, CREATE_NEW, 0, save_path, sizeof(save_path)))
	{
		return FALSE;
	}

	path[0]= 0;
	strncpy(directory, save_path, sizeof(directory));
	directory[sizeof(directory)-1]= 0;
	strncpy(path, directory, SAVED_GAME_FILE_PATH_LENGTH);
	path[SAVED_GAME_FILE_PATH_LENGTH-1]= 0;
	csstrcat256(path, saved_game_file_type_get_name(type));
	file_reference_set_path(&file, path, FALSE);
	if (!file_create(&file) || !file_open(&file, FLAG(_permission_write_bit), &error))
	{
		return FALSE;
	}
	created= file_set_eof(&file, (unsigned long)saved_game_file_type_get_size(type));
	if (created)
	{
		memcpy(description->information.path, directory, sizeof(directory));
	}
	file_close(&file);

	return created;
}

/* 0x002ad220: the step of a rename that opens the file of the state of the
game in the old saved game and makes it in the new one, with its size, to
copy it. Whether there is one to copy is left where it is told: that the old
one has none is not to fail */
boolean saved_game_file_rename_open_copy(
	struct saved_game_file_description *description,
	boolean *copy)
{
	char source_path[SAVED_GAME_FILE_PATH_LENGTH];
	char destination_path[SAVED_GAME_FILE_PATH_LENGTH];

	saved_game_file_get_state_path(source_path, description->old_path);
	description->source_file= CreateFileA(source_path, (uint32_t)GENERIC_READ, 0, NULL, OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN, 0);
	if (description->source_file==(uint32_t)INVALID_HANDLE_VALUE)
	{
		*copy= FALSE;

		return TRUE;
	}

	saved_game_file_get_state_path(destination_path, description->information.path);
	description->destination_file= CreateFileA(destination_path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_FLAG_SEQUENTIAL_SCAN, 0);
	if (description->destination_file!=(uint32_t)INVALID_HANDLE_VALUE)
	{
		description->size= GetFileSize(description->source_file, NULL);
		if (description->size!=(unsigned long)NONE)
		{
			SetFilePointer(description->destination_file, (long)description->size, NULL, FILE_BEGIN);
			if (!GetLastError() && SetEndOfFile(description->destination_file))
			{
				*copy= TRUE;

				return TRUE;
			}
		}
	}

	if (description->source_file!=(uint32_t)NONE)
	{
		CloseHandle(description->source_file);
		description->source_file= (uint32_t)NONE;
	}
	if (description->destination_file!=(uint32_t)NONE)
	{
		CloseHandle(description->destination_file);
		description->destination_file= (uint32_t)NONE;
	}

	return FALSE;
}
