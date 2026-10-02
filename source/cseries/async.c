/*
ASYNC.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

A thread does the tasks of a list, one at a time: those with more priority
first, and the reads of the disc of the same priority in the order they have
on the disc.
*/

#include "async.h"

#include <string.h>

/* ---------- prototypes of functions of the console's libraries */

/* 0x002d626c */
uint32_t CreateMutexA(void *security_attributes, int initial_owner, const char *name);

/* 0x002d61eb */
uint32_t CreateSemaphoreA(void *security_attributes, long initial_count, long maximum_count, const char *name);

/* 0x002d6246 */
int ReleaseSemaphore(uint32_t semaphore, long release_count, long *previous_count);

/* 0x002d21c6 */
uint32_t CreateThread(void *security_attributes, uint32_t stack_size, void (*start)(void), void *parameter,
	uint32_t flags, uint32_t *thread_id);

/* 0x002d1fb6 */
int SetThreadPriority(uint32_t thread, int priority);

/* 0x002d2488 */
uint32_t SetFilePointer(uint32_t file, long distance, long *distance_high, uint32_t method);

/* 0x002d2241 */
int ReadFile(uint32_t file, void *buffer, uint32_t size, uint32_t *bytes_read, void *overlapped);

/* 0x002d2750 */
uint32_t CreateFileA(const char *path, uint32_t access, uint32_t share_mode, void *security_attributes,
	uint32_t creation_disposition, uint32_t flags, uint32_t template_file);

/* 0x00331a35 */
int CreateDirectoryA(const char *path, void *security_attributes);

/* 0x002d232e */
int WriteFile(uint32_t file, const void *buffer, uint32_t size, uint32_t *bytes_written, void *overlapped);

/* 0x002d26b0 */
int FlushFileBuffers(uint32_t file);

/* 0x002d2404 */
int SetEndOfFile(uint32_t file);

/* 0x002d2715 */
uint32_t GetFileSize(uint32_t file, uint32_t *size_high);

/* 0x002d1f3a */
int CloseHandle(uint32_t handle);

/* ---------- constants */

#define GENERIC_READ 0x80000000
#define GENERIC_WRITE 0x40000000
#define FILE_SHARE_READ 1
#define FILE_FLAG_NO_BUFFERING 0x20000000
#define FILE_FLAG_RANDOM_ACCESS 0x10000000
#define FILE_FLAG_SEQUENTIAL_SCAN 0x08000000
#define INVALID_HANDLE_VALUE 0xFFFFFFFF
#define INFINITE 0xFFFFFFFF
#define FILE_BEGIN 0
#define INVALID_SET_FILE_POINTER 0xFFFFFFFF
#define THREAD_PRIORITY_ABOVE_NORMAL 1

enum
{
	ASYNC_THREAD_STACK_SIZE= 0x4000
};

/* ---------- globals */

/* 0x004e0354 */
struct async_globals async_globals;

/* 0x00510810: the task that the thread works on */
struct async_task *async_current_task;

/* 0x005020c8 */
struct async_buffer_globals async_buffer_globals;

/* 0x004fa0c8 */
byte async_buffer[ASYNC_BUFFER_SIZE];

/* 0x0046e488: the most that a read task reads each time that the thread
calls it */
unsigned long async_read_maximum_size= 0x40000;

/* 0x0046e48c: the same, of a write task */
unsigned long async_write_maximum_size= 0x40000;

/* ---------- code */

/* 0x00120a90: makes the list of the tasks that are not in use and starts the
thread */
void async_initialize(
	void)
{
	long index;

	memset(async_globals.tasks, 0, sizeof(async_globals.tasks));
	for (index= 0; index<MAXIMUM_ASYNC_TASKS; index++)
	{
		async_globals.tasks[index].next= &async_globals.tasks[index+1];
		async_globals.tasks[index].id= NONE;
	}
	async_globals.tasks[MAXIMUM_ASYNC_TASKS-1].next= NULL;
	async_globals.free_list= async_globals.tasks;
	async_globals.task_list= NULL;

	async_globals.free_list_mutex= CreateMutexA(NULL, FALSE, NULL);
	async_globals.task_list_mutex= CreateMutexA(NULL, FALSE, NULL);
	async_globals.semaphore= CreateSemaphoreA(NULL, 0, MAXIMUM_ASYNC_TASKS, NULL);

	async_buffer_globals.unknown0= 0;
	async_buffer_globals.address= async_buffer;
	async_buffer_globals.size= ASYNC_BUFFER_SIZE;

	async_globals.thread= CreateThread(NULL, ASYNC_THREAD_STACK_SIZE, async_thread_procedure, NULL, 0, NULL);
	SetThreadPriority(async_globals.thread, THREAD_PRIORITY_ABOVE_NORMAL);
}

/* 0x001208b0: takes a task that is not in use; waits for one if there is
none */
struct async_task *async_task_allocate(
	void)
{
	struct async_task *task= NULL;

	for (;;)
	{
		WaitForSingleObject(async_globals.free_list_mutex, INFINITE);
		if (async_globals.free_list)
		{
			task= async_globals.free_list;
			async_globals.free_list= task->next;
			task->next= NULL;
		}
		ReleaseMutex(async_globals.free_list_mutex);
		if (task)
		{
			break;
		}
		SwitchToThread();
	}

	return task;
}

/* 0x001a1720: whether a task goes in the list before the one that is next
there. A task goes before those with less priority; among those with the
same, a read goes before what is not a read, and before the reads that are
further on the disc from where the read before them ends. Also keeps, in the
state, where the next task ends up if it is a read */
boolean async_read_goes_before(
	long next_priority,
	long task_priority,
	struct async_read_position_state *state,
	async_task_function task_function,
	async_task_function next_function,
	const struct async_read_parameters *next_parameters,
	const struct async_read_parameters *task_parameters)
{
	boolean result= FALSE;

	if (task_priority>next_priority)
	{
		result= TRUE;
	}
	else if (task_priority<next_priority)
	{
		result= FALSE;
	}
	else if (task_function==async_read_work)
	{
		if (next_function!=async_read_work)
		{
			result= TRUE;
		}
		else if (!state->valid)
		{
			result= FALSE;
		}
		else
		{
			uint32_t task_file= task_parameters->file-state->file;
			uint32_t next_file= next_parameters->file-state->file;

			if (task_file<next_file)
			{
				result= TRUE;
			}
			else if (task_file>next_file)
			{
				result= FALSE;
			}
			else
			{
				result= task_parameters->offset-state->offset < next_parameters->offset-state->offset;
			}
		}
	}
	else if (next_function!=async_read_work)
	{
		return FALSE;
	}

	if (next_function==async_read_work)
	{
		state->file= next_parameters->file;
		state->offset= next_parameters->offset;
		state->valid= TRUE;
	}

	return result;
}

/* 0x00120900: puts a task in the list, in its place. The first task of the
list, which the thread can be working on, only gives up its place to a task
with more priority */
void ASYNC_CALLBACK async_task_list_insert(
	struct async_task *task)
{
	struct async_read_position_state state= {0, 0, FALSE};
	struct async_task *previous= async_globals.task_list;

	if (!previous)
	{
		async_globals.task_list= task;
		task->next= NULL;
		return;
	}
	if (task->priority>previous->priority)
	{
		task->next= previous;
		async_globals.task_list= task;
		return;
	}

	while (previous->next)
	{
		struct async_task *next= previous->next;

		if (async_read_goes_before(next->priority, task->priority, &state, task->function, next->function,
			(const struct async_read_parameters *)next->parameters,
			(const struct async_read_parameters *)task->parameters))
		{
			task->next= previous->next;
			previous->next= task;
			return;
		}
		previous= next;
	}
	previous->next= task;
	task->next= NULL;
}

/* 0x001209c0: puts a task in the list for the thread to do; the task's id */
long async_task_queue(
	struct async_task *task)
{
	long previous_count;
	long id;

	WaitForSingleObject(async_globals.task_list_mutex, INFINITE);
	async_task_list_insert(task);
	task->id= (async_globals.generation<<8)|(long)(task-async_globals.tasks);
	async_globals.generation++;
	id= task->id;
	ReleaseMutex(async_globals.task_list_mutex);

	previous_count= 0;
	ReleaseSemaphore(async_globals.semaphore, 1, &previous_count);

	return id;
}

static inline __attribute__((always_inline)) void async_task_list_unlink(
	struct async_task *task)
{
	if (async_globals.task_list==task)
	{
		async_globals.task_list= task->next;
	}
	else
	{
		struct async_task *previous= async_globals.task_list;

		while (previous->next!=task)
		{
			previous= previous->next;
		}
		previous->next= task->next;
	}
}

/* 0x00120a30: takes a task off the list; it has to be there */
void async_task_list_remove(
	struct async_task *task)
{
	WaitForSingleObject(async_globals.task_list_mutex, INFINITE);
	async_task_list_unlink(task);
	task->id= NONE;
	ReleaseMutex(async_globals.task_list_mutex);
}

/* 0x00120ba0: makes a task and puts it in the list; the task's id. The flag
is cleared, and set when the task is done */
long async_task_add(
	long priority,
	const uint32_t *parameters,
	long category,
	async_task_function function,
	byte *done)
{
	struct async_task *task;

	async_globals.tasks_added++;
	*done= FALSE;
	task= async_task_allocate();
	memcpy(task->parameters, parameters, sizeof(task->parameters));
	task->priority= priority;
	task->done= done;
	task->function= function;
	task->category= category;

	return async_task_queue(task);
}

/* 0x00120bf0: how many tasks are not done yet */
long async_task_count(
	void)
{
	long count= 0;
	struct async_task *task;

	WaitForSingleObject(async_globals.task_list_mutex, INFINITE);
	for (task= async_globals.task_list; task; task= task->next)
	{
		count++;
	}
	ReleaseMutex(async_globals.task_list_mutex);

	return count;
}

/* 0x00120b50: whether a task of a category is not done yet */
boolean async_category_busy(
	long category)
{
	boolean busy= FALSE;
	struct async_task *task;

	WaitForSingleObject(async_globals.task_list_mutex, INFINITE);
	for (task= async_globals.task_list; task; task= task->next)
	{
		if (task->category==category)
		{
			busy= TRUE;
			break;
		}
	}
	ReleaseMutex(async_globals.task_list_mutex);

	return busy;
}

/* 0x00120ce0: gives another priority to a task that is not done yet, and the
place in the list that goes with it; whether the task was there */
boolean async_task_set_priority(
	long id,
	long priority)
{
	boolean changed= FALSE;
	struct async_task *task;

	WaitForSingleObject(async_globals.task_list_mutex, INFINITE);
	task= &async_globals.tasks[id&0xFF];
	if (task->id==id)
	{
		async_task_list_unlink(task);
		task->priority= priority;
		async_task_list_insert(task);
		changed= TRUE;
	}
	ReleaseMutex(async_globals.task_list_mutex);

	return changed;
}

/* 0x00120d50: waits for the flag of a task; can draw the loading screen
meanwhile */
void async_wait(
	const volatile byte *done,
	boolean idle)
{
	while (!*done)
	{
		SwitchToThread();
		if (idle)
		{
			sound_idle();
		}
	}
}

/* 0x00120c30: what the thread runs */
void async_thread_procedure(
	void)
{
	async_thread_loop();
	__builtin_trap();
}

/* 0x00120c40: does the first task of the list, again and again; never
returns */
void async_thread_loop(
	void)
{
	for (;;)
	{
		struct async_task *task;
		long previous_count;
		long result;

		WaitForSingleObject(async_globals.semaphore, INFINITE);

		WaitForSingleObject(async_globals.task_list_mutex, INFINITE);
		task= async_globals.task_list;
		ReleaseMutex(async_globals.task_list_mutex);

		async_current_task= task;
		result= task->function(task->parameters);
		if (result==_async_task_not_done)
		{
			/* the task has more to do */
			previous_count= 0;
			ReleaseSemaphore(async_globals.semaphore, 1, &previous_count);
		}
		else
		{
			if (result==_async_task_done && task->done)
			{
				*task->done= TRUE;
			}
			async_task_list_remove(task);

			WaitForSingleObject(async_globals.free_list_mutex, INFINITE);
			task->next= async_globals.free_list;
			async_globals.free_list= task;
			ReleaseMutex(async_globals.free_list_mutex);
		}
	}
}

/* 0x001a0e70: the function of a task that reads a file. Reads a part, and
is done when all is read or the file gives less than what was asked for */
long ASYNC_CALLBACK async_read_work(
	void *parameters)
{
	struct async_read_parameters *read= parameters;
	uint32_t size= read->size-read->bytes_done;
	uint32_t bytes_read= 0;
	long offset_high= 0;

	if (size>async_read_maximum_size)
	{
		size= async_read_maximum_size;
	}

	if (SetFilePointer(read->file, (long)read->offset, &offset_high, FILE_BEGIN)!=INVALID_SET_FILE_POINTER)
	{
		ReadFile(read->file, (byte *)read->buffer+read->bytes_done, size, &bytes_read, NULL);
		read->bytes_done+= bytes_read;
		read->offset+= bytes_read;
		if (read->bytes_read)
		{
			*read->bytes_read= (long)read->bytes_done;
		}
	}

	if (read->size==read->bytes_done || bytes_read<size)
	{
		return _async_task_done;
	}

	return _async_task_not_done;
}

/* 0x001a0f10: starts to read a part of a file; the task's id. When the read
ends, the flag is set; the number of bytes read is kept as the read goes */
long async_read_position(
	uint32_t file,
	long size,
	uint32_t offset,
	long category,
	long priority,
	long *bytes_read,
	void *buffer,
	uint8_t *done)
{
	union
	{
		struct async_read_parameters read;
		uint32_t parameters[ASYNC_TASK_PARAMETER_COUNT];
	} task;

	memset(&task, 0, sizeof(task));
	task.read.file= file;
	task.read.buffer= buffer;
	task.read.size= (uint32_t)size;
	task.read.offset= offset;
	task.read.bytes_read= bytes_read;

	return async_task_add(priority, task.parameters, category, async_read_work, done);
}

static inline __attribute__((always_inline)) char *path_next_element(
	char *path,
	char **rest)
{
	char *element= NULL;
	char *end= NULL;

	if (path)
	{
		element= path+strspn(path, "\\");
		if (!*element)
		{
			element= NULL;
		}
	}
	if (element)
	{
		end= strpbrk(element, "\\");
		if (end)
		{
			*end++= 0;
		}
	}
	*rest= end;

	return element;
}

/* 0x001a08f0: the function of a task that opens a file. If the file cannot
be opened and the task says so, makes the folders of the path, one after
another, and tries again */
long ASYNC_CALLBACK async_create_file_work(
	void *parameters)
{
	struct async_create_file_parameters *create= parameters;

	*create->file= CreateFileA(create->path, create->access, create->share_mode, NULL, create->creation_disposition,
		create->flags, 0);
	if (*create->file==INVALID_HANDLE_VALUE && create->create_directories)
	{
		char directory[256];
		char path[256];
		char *element;
		char *rest;

		directory[0]= 0;
		strncpy(path, create->path, sizeof(path));
		path[sizeof(path)-1]= 0;
		element= path_next_element(path, &rest);
		strncpy(directory, "", sizeof(directory));
		directory[sizeof(directory)-1]= 0;
		while (element)
		{
			csstrcat256(directory, element);
			element= path_next_element(rest, &rest);
			if (element)
			{
				CreateDirectoryA(directory, NULL);
			}
			csstrcat256(directory, "\\");
		}

		*create->file= CreateFileA(create->path, create->access, create->share_mode, NULL,
			create->creation_disposition, create->flags, 0);
	}

	return _async_task_done;
}

/* 0x001a0fb0: the function of a task that writes a file. Writes a part, and
is done when all is written or the file takes less than what was given */
long ASYNC_CALLBACK async_write_work(
	void *parameters)
{
	struct async_write_parameters *write= parameters;
	uint32_t bytes_written= 0xFFFFFFFF;
	long offset_high= 0;

	if (SetFilePointer(write->file, (long)write->offset, &offset_high, FILE_BEGIN)!=INVALID_SET_FILE_POINTER)
	{
		uint32_t size= write->size-write->bytes_done;

		if (size>async_write_maximum_size)
		{
			size= async_write_maximum_size;
		}
		WriteFile(write->file, (const byte *)write->buffer+write->bytes_done, size, &bytes_written, NULL);
		write->offset+= bytes_written;
		write->bytes_done+= bytes_written;
		if (write->size!=write->bytes_done && bytes_written>=size)
		{
			return _async_task_not_done;
		}
		if (write->bytes_written)
		{
			*write->bytes_written= write->bytes_done;
		}
	}

	if (write->flags&1)
	{
		FlushFileBuffers(write->file);
	}

	return _async_task_done;
}

/* 0x001a1050: starts to write a part of a file; the task's id. When the
write ends, the number of bytes written and the flag are set */
long async_write_position(
	uint32_t file,
	const void *buffer,
	uint32_t size,
	uint32_t offset,
	uint32_t flags,
	long category,
	long priority,
	uint32_t *bytes_written,
	byte *done)
{
	union
	{
		struct async_write_parameters write;
		uint32_t parameters[ASYNC_TASK_PARAMETER_COUNT];
	} task;

	if (bytes_written)
	{
		*bytes_written= 0;
	}
	memset(&task, 0, sizeof(task));
	task.write.file= file;
	task.write.buffer= buffer;
	task.write.size= size;
	task.write.offset= offset;
	task.write.bytes_written= bytes_written;
	task.write.flags= flags;

	return async_task_add(priority, task.parameters, category, async_write_work, done);
}

/* 0x001a12c0: the function of a task that sets the size of a file */
long ASYNC_CALLBACK async_set_file_size_work(
	void *parameters)
{
	struct async_set_file_size_parameters *set_size= parameters;
	long size_high= 0;
	boolean success= SetFilePointer(set_size->file, set_size->size, &size_high, FILE_BEGIN)!=INVALID_SET_FILE_POINTER;

	if (success)
	{
		success= SetEndOfFile(set_size->file)!=0;
	}
	if (set_size->success)
	{
		*set_size->success= success;
	}

	return _async_task_done;
}

/* 0x001a1310: starts to set the size of a file; the task's id */
long async_set_file_size(
	uint32_t file,
	long size,
	long category,
	long priority,
	boolean *success,
	byte *done)
{
	union
	{
		struct async_set_file_size_parameters set_size;
		uint32_t parameters[ASYNC_TASK_PARAMETER_COUNT];
	} task;

	if (success)
	{
		*success= FALSE;
	}
	memset(&task, 0, sizeof(task));
	task.set_size.file= file;
	task.set_size.size= size;
	task.set_size.success= success;

	return async_task_add(priority, task.parameters, category, async_set_file_size_work, done);
}

/* 0x001a1530: the function of a task that closes a file */
long ASYNC_CALLBACK async_close_file_work(
	void *parameters)
{
	struct async_close_file_parameters *close= parameters;

	CloseHandle(close->file);

	return _async_task_done;
}

/* 0x001a1550: starts to close a file; the task's id */
long async_close_file(
	uint32_t file,
	long category,
	long priority,
	byte *done)
{
	union
	{
		struct async_close_file_parameters close;
		uint32_t parameters[ASYNC_TASK_PARAMETER_COUNT];
	} task;

	memset(&task, 0, sizeof(task));
	task.close.file= file;

	return async_task_add(priority, task.parameters, category, async_close_file_work, done);
}

/* 0x001a15d0: the function of a task that takes the size of a file */
long ASYNC_CALLBACK async_get_file_size_work(
	void *parameters)
{
	struct async_get_file_size_parameters *get_size= parameters;

	*get_size->size= GetFileSize(get_size->file, NULL);

	return _async_task_done;
}

/* 0x001a15f0: starts to take the size of a file; the task's id */
long async_get_file_size(
	uint32_t file,
	long category,
	long priority,
	uint32_t *size,
	byte *done)
{
	union
	{
		struct async_get_file_size_parameters get_size;
		uint32_t parameters[ASYNC_TASK_PARAMETER_COUNT];
	} task;

	*size= 0;
	memset(&task, 0, sizeof(task));
	task.get_size.file= file;
	task.get_size.size= size;

	return async_task_add(priority, task.parameters, category, async_get_file_size_work, done);
}

/* 0x001a0b40: starts to open a file; the task's id. The access is bit 0 to
read and bit 1 to write; the flags are bit 0 for no buffering, bit 1 for
random access and bit 2 for a sequential scan */
long async_create_file(
	long mode,
	const char *path,
	uint32_t access_flags,
	uint32_t file_flags,
	long category,
	long priority,
	uint32_t *file,
	byte *done)
{
	union
	{
		struct async_create_file_parameters create;
		uint32_t parameters[ASYNC_TASK_PARAMETER_COUNT];
	} task;

	memset(&task, 0, sizeof(task));
	switch (mode)
	{
	case _async_file_open_existing:
		task.create.creation_disposition= 3;
		break;
	case _async_file_create_always:
		task.create.creation_disposition= 2;
		break;
	case _async_file_create_new:
		task.create.creation_disposition= 1;
		break;
	case _async_file_open_always:
		task.create.creation_disposition= 4;
		break;
	case _async_file_open_always_with_directories:
		task.create.creation_disposition= 4;
		task.create.create_directories= TRUE;
		break;
	case _async_file_truncate_existing:
		task.create.creation_disposition= 5;
		break;
	}

	if (access_flags&1)
	{
		task.create.access= GENERIC_READ;
	}
	if (access_flags&2)
	{
		task.create.access|= GENERIC_WRITE;
	}
	else
	{
		task.create.share_mode= FILE_SHARE_READ;
	}

	if (file_flags&1)
	{
		task.create.flags= FILE_FLAG_NO_BUFFERING;
	}
	if (file_flags&2)
	{
		task.create.flags|= FILE_FLAG_RANDOM_ACCESS;
	}
	else if (file_flags&4)
	{
		task.create.flags|= FILE_FLAG_SEQUENTIAL_SCAN;
	}

	task.create.path= path;
	task.create.file= file;

	return async_task_add(priority, task.parameters, category, async_create_file_work, done);
}

/* 0x001a1100: the function of a task that copies a part of a file to
another, through a buffer. Each time, either reads a part or writes the part
that was read */
long ASYNC_CALLBACK async_copy_work(
	void *parameters)
{
	struct async_copy_parameters *copy= parameters;
	uint32_t maximum_size= async_read_maximum_size;
	uint32_t size;
	uint32_t bytes= 0;
	long offset_high= 0;
	long result= _async_task_not_done;

	if (async_write_maximum_size<=maximum_size)
	{
		maximum_size= async_write_maximum_size;
	}
	size= copy->size-copy->bytes_done;
	if (size>maximum_size)
	{
		size= maximum_size;
	}

	if (!copy->writing)
	{
		if (SetFilePointer(copy->source_file, (long)copy->source_offset, &offset_high, FILE_BEGIN)!=
			INVALID_SET_FILE_POINTER)
		{
			if (!ReadFile(copy->source_file, (byte *)copy->buffer+copy->bytes_done, size, &bytes, NULL))
			{
				bytes= 0;
			}
			copy->source_offset+= bytes;
		}
		if (bytes<size)
		{
			result= _async_task_done;
		}
		copy->writing= TRUE;
	}
	else
	{
		if (SetFilePointer(copy->destination_file, (long)copy->destination_offset, &offset_high, FILE_BEGIN)!=
			INVALID_SET_FILE_POINTER)
		{
			if (!WriteFile(copy->destination_file, (byte *)copy->buffer+copy->bytes_done, size, &bytes, NULL))
			{
				bytes= 0;
			}
			copy->bytes_done+= bytes;
			copy->destination_offset+= bytes;
		}
		if (copy->size==copy->bytes_done || bytes<size)
		{
			result= _async_task_done;
		}
		copy->writing= FALSE;
	}

	if (copy->bytes_copied)
	{
		*copy->bytes_copied= copy->bytes_done;
	}

	return result;
}

/* 0x001a1210: starts to copy a part of a file to another; the task's id */
long async_copy_position(
	uint32_t source_file,
	uint32_t destination_file,
	void *buffer,
	uint32_t size,
	uint32_t source_offset,
	uint32_t destination_offset,
	long category,
	long priority,
	uint32_t *bytes_copied,
	byte *done)
{
	union
	{
		struct async_copy_parameters copy;
		uint32_t parameters[ASYNC_TASK_PARAMETER_COUNT];
	} task;

	if (bytes_copied)
	{
		*bytes_copied= 0;
	}
	memset(&task, 0, sizeof(task));
	task.copy.source_file= source_file;
	task.copy.destination_file= destination_file;
	task.copy.buffer= buffer;
	task.copy.size= size;
	task.copy.bytes_copied= bytes_copied;
	task.copy.source_offset= source_offset;
	task.copy.destination_offset= destination_offset;

	return async_task_add(priority, task.parameters, category, async_copy_work, done);
}

/* 0x001a1680: the function of a task that has what was written to a file
put on the disk */
long ASYNC_CALLBACK async_flush_file_work(
	void *parameters)
{
	struct async_flush_file_parameters *flush= parameters;

	FlushFileBuffers(flush->file);

	return _async_task_done;
}

/* 0x001a16a0: starts that; the task's id */
long async_flush_file(
	uint32_t file,
	long category,
	long priority,
	byte *done)
{
	union
	{
		struct async_flush_file_parameters flush;
		uint32_t parameters[ASYNC_TASK_PARAMETER_COUNT];
	} task;

	memset(&task, 0, sizeof(task));
	task.flush.file= file;

	return async_task_add(priority, task.parameters, category, async_flush_file_work, done);
}

/* 0x001a0af0: has what was written to a file put on the disk, and waits for
it. The name is a guess */
void async_flush_file_blocking(
	long category,
	uint32_t file)
{
	volatile boolean done;

	async_flush_file(file, category, 6, (byte *)&done);
	while (!done)
	{
		SwitchToThread();
	}
}
