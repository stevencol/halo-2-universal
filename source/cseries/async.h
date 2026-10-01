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

/* ---------- structures */

struct async_task
{
	byte unknown0[0x30];
	/* cache_file_block_until_not_busy waits for the tasks of five of them */
	long category;
	byte unknown34[8];
	struct async_task *next;
};

#if defined(__i386__)
typedef char verify_async_task_next_offset[
	offsetof(struct async_task, next) == 0x3C ? 1 : -1];
#endif

/* ---------- prototypes/ASYNC.C */

boolean async_category_busy(long category);

/* ---------- prototypes of functions not decompiled yet */

/* 0x001a0f10: starts to read a part of a file. When the read ends, the
number of bytes read and a flag are set */
void async_read_position(uint32_t file, long size, uint32_t offset, long unknown1, long unknown2,
	long *bytes_read, void *buffer, uint8_t *done);

/* ---------- prototypes of functions of the console's libraries */

/* 0x002d63fa */
uint32_t WaitForSingleObject(uint32_t handle, uint32_t milliseconds);

/* 0x002d62c4 */
int ReleaseMutex(uint32_t mutex);

/* 0x002d2084: lets other threads run */
int SwitchToThread(void);

/* ---------- globals */

extern uint32_t async_mutex;
extern struct async_task *async_task_list;

#endif
