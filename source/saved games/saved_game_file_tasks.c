/*
SAVED_GAME_FILE_TASKS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The files of the players are read and written by the thread of the tasks of
async.c: here is what gives it the tasks. The names of the file and of the
functions are guesses. Not here: the work of a write (0x002acde0) and what
follows 0x002acf80.
*/

#include "saved_game_files.h"

#include "cseries/async.h"
#include "saved games/game_state.h"
#include "tag_files/files.h"

#include <string.h>

/* ---------- constants */

enum
{
	SAVED_GAME_FILE_TASK_PRIORITY= 2,
	/* ERROR_TOO_MANY_OPEN_FILES: what a read fails with until it is known
	why */
	SAVED_GAME_FILE_ERROR_DEFAULT= 4,
	SAVED_GAME_FILE_DESCRIPTION_UNKNOWN9C= 6
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

/* ---------- prototypes of functions of the C library */

/* 0x003207ed: of characters of 16 bits */
wchar *wcsncpy(wchar *destination, const wchar *source, unsigned int count);

/* ---------- prototypes of functions that are not decompiled */

/* 0x002adf70: reads a file into a buffer */
void unknown_002adf70(unsigned long size, struct file_reference *file, void *buffer, boolean unknown,
	struct saved_game_file_result *result);

/* 0x002acde0: the work of the task of a write */
long ASYNC_CALLBACK unknown_002acde0(struct async_custom_task *task, void *data, long size);

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
	unknown_002adf70(read->size, &file, read->buffer, read->unknown8, read->result);

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
	description->unknown8C= NULL;
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
	void *unknown,
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
	description->unknown84= NONE;
	description->unknown88= NONE;
	description->unknown90= NONE;
	description->unknown94= 0;
	description->unknown98= 0;
	description->unknown9C= 0;
	description->renamed= TRUE;
	description->unknown8C= unknown;
	description->information= *information;

	return description;
}
