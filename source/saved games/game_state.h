/*
GAME_STATE.H

The state of the game: one block of memory that the game writes to a file to
make a checkpoint, and reads back to go to it. As in Halo 1, with two files
instead of one.
*/

#ifndef __GAME_STATE_H
#define __GAME_STATE_H

#include "cseries/cseries.h"

#include <stdint.h>

/* ---------- constants */

enum
{
	NUMBER_OF_GAME_STATE_FILES= 2,

	GAME_STATE_CPU_SIZE= 0x40000,
	GAME_STATE_GPU_SIZE= 0x3BE000,
	/* more than the two sizes together, which is what is written */
	GAME_STATE_FILE_SIZE= 0x40B000,

	/* of the tasks of the files */
	GAME_STATE_TASK_CATEGORY= 8,
	GAME_STATE_TASK_PRIORITY= 6
};

/* ---------- structures */

/* 0x005020d8. The first fields are those of Halo 1's structure; the names
of the others are guesses */
struct xbox_game_state_globals
{
	boolean buffer_allocated;
	byte pad1[3];
	void *buffer;
	long buffer_size;
	/* a copy of the buffer that is being written to a file, in memory that
	the game can ask back; NULL if there is none */
	void *write_buffer;
	/* set when the task that writes is done */
	boolean write_done;
	byte pad11[3];
	uint32_t bytes_written;
	/* the file that is being written */
	short write_file_index;
	boolean file_valid_for_read[NUMBER_OF_GAME_STATE_FILES];
	boolean file_open;
	byte pad1D[3];
	uint32_t handles[NUMBER_OF_GAME_STATE_FILES];
};

#if defined(__i386__)
typedef char verify_xbox_game_state_globals_size[
	sizeof(struct xbox_game_state_globals) == 0x28 ? 1 : -1];
typedef char verify_xbox_game_state_globals_handles_offset[
	offsetof(struct xbox_game_state_globals, handles) == 0x20 ? 1 : -1];
#endif

/* ---------- prototypes/GAME_STATE_XBOX.C */

void *game_state_allocate_buffer(uint32_t cpu_size, uint32_t gpu_size);
void game_state_create_or_open_file(void);
void game_state_close_file(void);
void game_state_write_to_file(short file_index);
boolean game_state_read_from_file(short file_index);

/* ---------- globals */

extern struct xbox_game_state_globals xbox_game_state_globals;
extern const char *game_state_file_name_format;

#endif
