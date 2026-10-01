/*
ASYNC.H

The work that the game gives to another thread: reads of the disc, mostly.
Worked out from halo2ship.exe; Halo 1 has nothing like it, and the names are
guesses.
*/

#ifndef __ASYNC_H
#define __ASYNC_H

#include "cseries.h"

#include <stdint.h>

/* ---------- constants */

enum
{
	MAXIMUM_ASYNC_TASKS= 150,
	ASYNC_TASK_PARAMETER_COUNT= 10,
	ASYNC_BUFFER_SIZE= 0x8000
};

/* what the function of a task returns */
enum
{
	/* the thread calls the function again */
	_async_task_not_done,
	/* the task is over, and its flag is set */
	_async_task_done,
	/* any other: the task is over, and its flag is not set */
	_async_task_done_without_flag
};

/* ---------- structures */

#define ASYNC_CALLBACK __attribute__((stdcall))

/* does the work of a task, or a part of it, with the task's parameters */
typedef long (ASYNC_CALLBACK *async_task_function)(void *parameters);

struct async_task
{
	/* the tasks with more are done first */
	long priority;
	/* the place in the table of the tasks, and above its low 8 bits the
	count of the tasks added before; NONE in a task that is not in the list */
	long id;
	uint32_t parameters[ASYNC_TASK_PARAMETER_COUNT];
	/* cache_file_block_until_not_busy waits for the tasks of five of them */
	long category;
	async_task_function function;
	/* set when the task is done, if not NULL */
	byte *done;
	struct async_task *next;
};

/* the parameters of a task that reads a file: async_read_work */
struct async_read_parameters
{
	uint32_t file;
	void *buffer;
	uint32_t size;
	/* in the file, of the next byte to read */
	uint32_t offset;
	/* gets the count of bytes read so far, if not NULL */
	long *bytes_read;
	uint32_t bytes_done;
};

/* the parameters of a task that opens a file: async_create_file_work */
struct async_create_file_parameters
{
	const char *path;
	uint32_t access;
	uint32_t share_mode;
	uint32_t creation_disposition;
	uint32_t flags;
	/* gets the handle of the file */
	uint32_t *file;
	/* if the file cannot be opened, make the folders of its path and try
	again */
	boolean create_directories;
};

/* of a task that writes a file: async_write_work */
struct async_write_parameters
{
	uint32_t file;
	const void *buffer;
	uint32_t size;
	/* in the file, of the next byte to write */
	uint32_t offset;
	/* gets the count of bytes written, when the task is done, if not NULL */
	uint32_t *bytes_written;
	uint32_t bytes_done;
	/* bit 0: flush the file when the task is done */
	uint32_t flags;
};

/* of the tasks that take the size of a file, set it and close the file */
struct async_get_file_size_parameters
{
	uint32_t file;
	uint32_t *size;
};

struct async_set_file_size_parameters
{
	uint32_t file;
	long size;
	/* gets whether the size was set, if not NULL */
	boolean *success;
};

struct async_close_file_parameters
{
	uint32_t file;
};

/* where the read before a place of the list ends, to put the reads in the
order they have on the disc */
struct async_read_position_state
{
	uint32_t file;
	uint32_t offset;
	boolean valid;
};

/* 0x004e0354. That these variables are one structure is a guess */
struct async_globals
{
	uint32_t thread;
	uint32_t free_list_mutex;
	uint32_t task_list_mutex;
	/* counts the times that the thread has a function to call */
	uint32_t semaphore;
	/* the tasks put in the list since the game started */
	long generation;
	struct async_task tasks[MAXIMUM_ASYNC_TASKS];
	/* the tasks that are not in use */
	struct async_task *free_list;
	/* the tasks that are not done yet: the first is the one that the thread
	works on */
	struct async_task *task_list;
	/* the tasks made since the game started */
	long tasks_added;
};

/* 0x005020c8: a buffer that the function at 0x002ad7c0 uses, which is not
understood yet */
struct async_buffer_globals
{
	long unknown0;
	byte *address;
	unsigned long size;
};

#if defined(__i386__)
typedef char verify_async_task_size[
	sizeof(struct async_task) == 0x40 ? 1 : -1];
typedef char verify_async_task_next_offset[
	offsetof(struct async_task, next) == 0x3C ? 1 : -1];
typedef char verify_async_globals_size[
	sizeof(struct async_globals) == 0x25A0 ? 1 : -1];
typedef char verify_async_globals_task_list_offset[
	offsetof(struct async_globals, task_list) == 0x2598 ? 1 : -1];
#endif

/* ---------- prototypes/ASYNC.C */

void async_initialize(void);
struct async_task *async_task_allocate(void);
void ASYNC_CALLBACK async_task_list_insert(struct async_task *task);
long async_task_queue(struct async_task *task);
void async_task_list_remove(struct async_task *task);
long async_task_add(long priority, const uint32_t *parameters, long category, async_task_function function,
	byte *done);
long async_task_count(void);
boolean async_category_busy(long category);
boolean async_task_set_priority(long id, long priority);
void async_wait(const volatile byte *done, boolean idle);
void async_thread_procedure(void);
void async_thread_loop(void);

boolean async_read_goes_before(long next_priority, long task_priority, struct async_read_position_state *state,
	async_task_function task_function, async_task_function next_function,
	const struct async_read_parameters *next_parameters, const struct async_read_parameters *task_parameters);
long ASYNC_CALLBACK async_read_work(void *parameters);
long async_read_position(uint32_t file, long size, uint32_t offset, long category, long priority,
	long *bytes_read, void *buffer, uint8_t *done);

long ASYNC_CALLBACK async_create_file_work(void *parameters);
long ASYNC_CALLBACK async_write_work(void *parameters);
long async_write_position(uint32_t file, const void *buffer, uint32_t size, uint32_t offset, uint32_t flags,
	long category, long priority, uint32_t *bytes_written, byte *done);
long ASYNC_CALLBACK async_set_file_size_work(void *parameters);
long async_set_file_size(uint32_t file, long size, long category, long priority, boolean *success, byte *done);
long ASYNC_CALLBACK async_close_file_work(void *parameters);
long async_close_file(uint32_t file, long category, long priority, byte *done);
long ASYNC_CALLBACK async_get_file_size_work(void *parameters);
long async_get_file_size(uint32_t file, long category, long priority, uint32_t *size, byte *done);

/* ---------- prototypes of functions not decompiled yet */

/* 0x00125d60: draws the screen that the game shows while it loads, if it is
time to. The name is a guess */
void main_loading_idle(void);

/* ---------- prototypes of functions of the console's libraries */

/* 0x002d63fa */
uint32_t WaitForSingleObject(uint32_t handle, uint32_t milliseconds);

/* 0x002d62c4 */
int ReleaseMutex(uint32_t mutex);

/* 0x002d2084: lets other threads run */
int SwitchToThread(void);

/* ---------- globals */

extern struct async_globals async_globals;
extern struct async_task *async_current_task;
extern struct async_buffer_globals async_buffer_globals;
extern byte async_buffer[ASYNC_BUFFER_SIZE];
extern unsigned long async_read_maximum_size;
extern unsigned long async_write_maximum_size;

#endif
