/*
GAME_STATE.H

The state of the game: one block of memory that the game writes to a file to
make a checkpoint, and reads back to go to it. As in Halo 1, with two files
instead of one.
*/

#ifndef __GAME_STATE_H
#define __GAME_STATE_H

#include "cseries/cseries.h"
#include "game/game.h"
#include "memory/data.h"

#include <stdint.h>

/* ---------- constants */

enum
{
	NUMBER_OF_GAME_STATE_FILES= 2,

	/* the memory of the state: the second part, at the end, is written to
	with the pages set for it (PAGE_WRITECOMBINE), as in Halo 1 */
	GAME_STATE_CPU_SIZE= 0x3BE000,
	GAME_STATE_GPU_SIZE= 0x40000,
	/* more than the two sizes together, which is what is written */
	GAME_STATE_FILE_SIZE= 0x40B000,

	/* of the tasks of the files */
	GAME_STATE_TASK_CATEGORY= 8,
	GAME_STATE_TASK_PRIORITY= 6
};

enum
{
	NUMBER_OF_GAME_STATE_BEFORE_SAVE_PROCS= 4,
	NUMBER_OF_GAME_STATE_AFTER_SAVE_PROCS= 1,
	NUMBER_OF_GAME_STATE_BEFORE_LOAD_PROCS= 5,
	NUMBER_OF_GAME_STATE_AFTER_LOAD_PROCS= 20
};

/* ---------- structures */

/* what the game does before and after it writes or reads its state: each
takes flags, and takes them off the stack */
#define GAME_STATE_PROC __attribute__((stdcall))
typedef void (GAME_STATE_PROC *game_state_proc)(unsigned long flags);

/* at the start of the state of the game: what it is a state of. As in Halo
1, with the address of the memory and the options of the game added */
struct game_state_header
{
	unsigned long allocation_size_checksum;
	void *base_address;
	char map_name[256];
	char build[32];
	unsigned long cache_file_checksum;
	unsigned long unknown12C;
	struct game_options options;
	short structure_bsp_index;
	byte unknown124A[0x3E];
};

/* what the functions of the files of the saved games leave of a read or of
a write, which another thread does */
struct saved_game_file_result
{
	volatile boolean done;
	byte unknown1;
	boolean success;
	byte unknown3[0x109];
};

typedef char verify_saved_game_file_result_size[
	sizeof(struct saved_game_file_result) == 0x10C ? 1 : -1];

/* 0x004e3b60. As in Halo 1, with the two files and their headers added */
struct game_state_globals
{
	boolean initialized;
	boolean locked;
	/* the file that was written last can be gone back to */
	boolean saved_game_valid;
	/* the other file can, too */
	boolean previous_saved_game_valid;
	/* the file that was written last */
	short file_index;
	word pad6;
	/* the header of the state that each file has */
	struct game_state_header headers[NUMBER_OF_GAME_STATE_FILES];
	/* in ticks */
	unsigned long save_time;
	short deaths_after_save;
	word pad251E;
	void *base_address;
	long cpu_allocation_size;
	long gpu_allocation_size;
	unsigned long allocation_size_checksum;
	long revert_time;
	struct game_state_header *header;
	boolean unknown2538;
	byte pad2539[7];
};

#if defined(__i386__)
typedef char verify_game_state_header_size[
	sizeof(struct game_state_header) == 0x1288 ? 1 : -1];
typedef char verify_game_state_header_structure_bsp_index_offset[
	offsetof(struct game_state_header, structure_bsp_index) == 0x1248 ? 1 : -1];
typedef char verify_game_state_globals_size[
	sizeof(struct game_state_globals) == 0x2540 ? 1 : -1];
typedef char verify_game_state_globals_header_offset[
	offsetof(struct game_state_globals, header) == 0x2534 ? 1 : -1];
#endif

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

void *game_state_allocate_buffer(uint32_t gpu_size, uint32_t cpu_size);
boolean game_state_file_valid_for_read(short file_index);
void game_state_create_or_open_file(void);
void game_state_close_file(void);
void game_state_write_to_file(short file_index);
boolean game_state_read_from_file(short file_index);

/* ---------- prototypes/GAME_STATE.C */

void GAME_STATE_PROC game_state_buffer_protect(unsigned long flags);
void game_state_initialize(void);
void game_state_dispose(void);
void game_state_initialize_for_new_map(void);
void game_state_set_structure_bsp_index(void);
void *game_state_malloc(long size);
void *game_state_malloc_aligned(long size, long alignment_bits);
void game_state_save(void);
void game_state_note_death(void);
void game_state_call_before_load_procs(unsigned long flags);
void game_state_call_after_load_procs(unsigned long flags);
void game_state_revert(unsigned long flags);
boolean game_state_header_valid(const struct game_state_header *header);
boolean game_state_headers_match(const struct game_state_header *header, const struct game_state_header *other);
void GAME_STATE_PROC game_state_set_revert_time(unsigned long flags);
void *ALLOCATION_CONVENTION game_state_allocation_allocate(struct allocation *allocation, unsigned long size);
void GAME_STATE_PROC game_state_allocation_restore(unsigned long flags);

/* ---------- globals */

extern struct game_state_globals game_state_globals;
extern struct allocation *game_state_allocation;
extern const struct allocation_functions game_state_allocation_functions;
extern game_state_proc game_state_before_save_procs[NUMBER_OF_GAME_STATE_BEFORE_SAVE_PROCS];
extern game_state_proc game_state_after_save_procs[NUMBER_OF_GAME_STATE_AFTER_SAVE_PROCS];
extern game_state_proc game_state_before_load_procs[NUMBER_OF_GAME_STATE_BEFORE_LOAD_PROCS];
extern game_state_proc game_state_after_load_procs[NUMBER_OF_GAME_STATE_AFTER_LOAD_PROCS];
extern struct xbox_game_state_globals xbox_game_state_globals;
extern const char *game_state_file_name_format;

/* ---------- inline code */

/* the game has no function for it. As Halo 1's, which also frees the
memory */
static inline void game_state_free_buffer(
	void)
{
	xbox_game_state_globals.buffer_allocated= FALSE;
}

/* ---------- prototypes/GAME_STATE.C: the states that are kept in files */

struct file_reference;

void game_state_load_saved_game(long saved_game);
boolean game_state_saved_game_read_header_async(void *header, struct saved_game_file_result *result,
	long saved_game);
void game_state_load_core(const char *name);
void game_state_core_file_reference(const char *name, struct file_reference *file);
boolean game_state_core_read(const char *name, unsigned long size, void *buffer);
boolean game_state_saved_game_create(long saved_game);
boolean game_state_saved_game_read_header(long header_size, long size, long saved_game, void *header);
void game_state_saved_game_read_data(long size, void *data, long saved_game);
boolean game_state_saved_game_write(void *state, long header_size, long size, long saved_game);

/* ---------- prototypes of functions not decompiled yet */

/* 0x002174b0, 0x00217520: start to read and to write the file that a saved
game of the hard disk keeps the state of the game in ("auxilary.bin"): what
is at its start, and the rest. Whether it was started. The names are
guesses */
boolean saved_game_auxiliary_read(void *header, long header_size, void *data, long data_size,
	struct saved_game_file_result *result, long saved_game);
boolean saved_game_auxiliary_write(void *header, long header_size, void *data, long data_size,
	struct saved_game_file_result *result, long saved_game);

#endif
