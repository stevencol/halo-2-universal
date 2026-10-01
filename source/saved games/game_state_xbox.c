/*
GAME_STATE_XBOX.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The files of the state of the game, on the hard disk. The names of the
functions are those of Halo 1's game_state_xbox.c where it has them. Unlike
Halo 1, the files are read and written by the tasks of async.c, and a
checkpoint is written while the game goes on: from a copy of the state, if
there is memory for one.
*/

#include "game_state.h"

#include "cseries/async.h"
#include "memory/physical_memory.h"

#include <string.h>

/* ---------- constants */

#define GENERIC_READ 0x80000000
#define GENERIC_WRITE 0x40000000
#define OPEN_ALWAYS 4
#define INVALID_HANDLE_VALUE 0xFFFFFFFF

enum
{
	/* a checkpoint is written from a copy of the state with less priority
	than from the state itself, which the game has to wait for */
	GAME_STATE_WRITE_COPY_PRIORITY= 2,
	GAME_STATE_WRITE_PRIORITY= 3
};

/* ---------- prototypes */

void PHYSICAL_MEMORY_CALLBACK game_state_write_finish(void *buffer, long unknown);
void PHYSICAL_MEMORY_CALLBACK game_state_write_reclaim(void *buffer, long unknown);

/* ---------- globals */

/* 0x005020d8 */
struct xbox_game_state_globals xbox_game_state_globals;

/* 0x00470050 */
const char *game_state_file_name_format= "n:\\savegame%d.bin";

/* ---------- code */

static inline __attribute__((always_inline)) void game_state_wait(
	const volatile boolean *done)
{
	while (!*done)
	{
		SwitchToThread();
	}
}

/* 0x00214f10: takes the memory of the state of the game, of the sum of two
sizes; its address, or NULL */
void *game_state_allocate_buffer(
	uint32_t cpu_size,
	uint32_t gpu_size)
{
	uint32_t size= cpu_size+gpu_size;
	void *buffer= physical_memory_allocate_high(size);

	xbox_game_state_globals.buffer= buffer;
	xbox_game_state_globals.buffer_size= (long)size;
	xbox_game_state_globals.buffer_allocated= TRUE;

	return buffer;
}

/* 0x00214f80: opens the two files, making those that are not there, and
gives them their size */
void game_state_create_or_open_file(
	void)
{
	long file_index;

	for (file_index= 0; file_index<NUMBER_OF_GAME_STATE_FILES; file_index++)
	{
		uint32_t *handle= &xbox_game_state_globals.handles[file_index];
		union
		{
			struct async_create_file_parameters create;
			uint32_t parameters[ASYNC_TASK_PARAMETER_COUNT];
		} task;
		char path[200];
		volatile boolean done;
		boolean opened;

		csnzprintf(path, sizeof(path), game_state_file_name_format, file_index);

		memset(&task, 0, sizeof(task));
		task.create.path= path;
		task.create.access= GENERIC_READ|GENERIC_WRITE;
		task.create.share_mode= 0;
		task.create.creation_disposition= OPEN_ALWAYS;
		task.create.flags= 0;
		task.create.file= handle;
		task.create.create_directories= FALSE;
		async_task_add(GAME_STATE_TASK_PRIORITY, task.parameters, GAME_STATE_TASK_CATEGORY, async_create_file_work,
			(byte *)&done);
		game_state_wait(&done);

		opened= *handle!=INVALID_HANDLE_VALUE;
		if (opened)
		{
			async_set_file_size(*handle, GAME_STATE_FILE_SIZE, GAME_STATE_TASK_CATEGORY, GAME_STATE_TASK_PRIORITY,
				&opened, (byte *)&done);
			game_state_wait(&done);
		}
	}

	xbox_game_state_globals.file_open= TRUE;
}

/* 0x002150f0 */
void game_state_close_file(
	void)
{
	long file_index;

	for (file_index= 0; file_index<NUMBER_OF_GAME_STATE_FILES; file_index++)
	{
		volatile boolean done;

		async_close_file(xbox_game_state_globals.handles[file_index], GAME_STATE_TASK_CATEGORY,
			GAME_STATE_TASK_PRIORITY, (byte *)&done);
		game_state_wait(&done);
	}

	xbox_game_state_globals.file_open= FALSE;
}

static inline __attribute__((always_inline)) void game_state_write_end(
	void)
{
	xbox_game_state_globals.file_valid_for_read[xbox_game_state_globals.write_file_index]=
		xbox_game_state_globals.bytes_written==(uint32_t)xbox_game_state_globals.buffer_size;
	physical_memory_buffer_free(xbox_game_state_globals.write_buffer);
	xbox_game_state_globals.write_buffer= NULL;
}

/* 0x00215140: if the copy of the state is written, tells whether the file
can be read and gives the memory of the copy back. The first of the two
functions that the game gets with that memory; the name is a guess */
void PHYSICAL_MEMORY_CALLBACK game_state_write_finish(
	void *buffer,
	long unknown)
{
	(void)buffer;
	(void)unknown;

	if (xbox_game_state_globals.write_done)
	{
		game_state_write_end();
	}
}

/* 0x00215180: the game wants the memory of the copy back: waits for the
copy to be written and does the same. The name is a guess */
void PHYSICAL_MEMORY_CALLBACK game_state_write_reclaim(
	void *buffer,
	long unknown)
{
	(void)buffer;
	(void)unknown;

	if (!xbox_game_state_globals.write_buffer)
	{
		return;
	}
	while (!*(volatile boolean *)&xbox_game_state_globals.write_done)
	{
		SwitchToThread();
		if (!*(void *volatile *)&xbox_game_state_globals.write_buffer)
		{
			return;
		}
	}
	game_state_write_end();
}

/* 0x002151f0: writes the state of the game to one of the files. If there
is memory for a copy of the state, the copy is what is written, and the game
goes on meanwhile; if not, the game waits. In Halo 1 the function tells
whether the file was written; here, file_valid_for_read does */
void game_state_write_to_file(
	short file_index)
{
	struct xbox_game_state_globals *globals= &xbox_game_state_globals;
	void *source;
	long priority;

	/* a copy that is still being written */
	if (globals->write_buffer)
	{
		game_state_write_reclaim(globals->write_buffer, 0);
	}

	globals->file_valid_for_read[file_index]= FALSE;
	globals->write_buffer= physical_memory_buffer_allocate(0, globals->buffer_size, 0, game_state_write_finish,
		game_state_write_reclaim);
	globals->write_file_index= file_index;

	source= globals->buffer;
	priority= GAME_STATE_WRITE_PRIORITY;
	if (globals->write_buffer)
	{
		memcpy(globals->write_buffer, globals->buffer, (size_t)globals->buffer_size);
		source= globals->write_buffer;
		priority= GAME_STATE_WRITE_COPY_PRIORITY;
	}
	async_write_position(globals->handles[file_index], source, (uint32_t)globals->buffer_size, 0, 0,
		GAME_STATE_TASK_CATEGORY, priority, &globals->bytes_written, &globals->write_done);

	if (!globals->write_buffer)
	{
		while (!*(volatile boolean *)&globals->write_done)
		{
			SwitchToThread();
			main_loading_idle();
		}
		globals->file_valid_for_read[file_index]= globals->bytes_written==(uint32_t)globals->buffer_size;
	}
}

/* 0x002152e0: reads the state of the game from one of the files, if it was
written; whether all of it was read */
boolean game_state_read_from_file(
	short file_index)
{
	struct xbox_game_state_globals *globals= &xbox_game_state_globals;
	long bytes_read;
	volatile boolean done;

	if (globals->write_buffer)
	{
		game_state_write_reclaim(globals->write_buffer, 0);
	}
	if (!globals->file_valid_for_read[file_index])
	{
		return FALSE;
	}

	async_read_position(globals->handles[file_index], globals->buffer_size, 0, GAME_STATE_TASK_CATEGORY,
		GAME_STATE_TASK_PRIORITY, &bytes_read, globals->buffer, (byte *)&done);
	game_state_wait(&done);

	return bytes_read==globals->buffer_size;
}
