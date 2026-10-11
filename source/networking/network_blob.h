/*
NETWORK_BLOB.H

A block of bytes that starts with a checksum of the rest, a version and its
size, and how it is seen to be good; and what has one in memory, which a task
reads from where it is kept and writes there. It is just before what the
network takes when a map is loaded, but it takes its memory from the cache
of textures and is not of the network: what the blocks are is not known yet.
The name of the file is a guess, and so are the names of its functions.
*/

#ifndef __NETWORK_BLOB_H
#define __NETWORK_BLOB_H

#include "cseries/cseries.h"
#include "math/real_math.h"

/* ---------- constants */

enum
{
	/* what seeing to a block says: it is good; its checksum is another; its
	size is not the one of its kind; it is too short, or its size is not the
	one that it says; its version is an older one, or a newer one */
	_network_blob_good= 1,
	_network_blob_bad_checksum= 8,
	_network_blob_wrong_size= 9,
	_network_blob_bad_size= 10,
	_network_blob_too_old= 11,
	_network_blob_too_new= 12,
	NETWORK_BLOB_HEADER_SIZE= 0x10,
	/* the kinds of what has a block: none, and the fourth, whose block can
	be shorter than its size and does not stop being good when it is read
	again */
	_network_blob_kind_none= 0,
	_network_blob_kind4= 4,
	/* what it is doing: nothing, reading its block, or writing it */
	_network_blob_idle= 0,
	_network_blob_reading,
	_network_blob_writing,
	/* its bits: the block is good; something of the block when it could not
	be read; it is one that was made here, not one that was read; and one
	more that writing it clears */
	_network_blob_good_bit= 0x2,
	_network_blob_unknown4_bit= 0x4,
	_network_blob_made_bit= 0x8,
	_network_blob_unknown10_bit= 0x10,
	/* the bits that it is given when it starts: it does not try again, and
	nothing is done when what it did could not be */
	_network_blob_option_once_bit= 0x1,
	_network_blob_option_no_recovery_bit= 0x2,
	/* how a task ends: well; in a way that says something of the block when
	it was being read; and in one after which it is tried again. And what a
	task is told when the memory of its block is taken back */
	_network_blob_task_done= 1,
	_network_blob_task_unknown6= 6,
	_network_blob_task_again= 7,
	_network_blob_task_no_memory= 13,
	/* how many tasks there can be; the two lists of the ones that wait or
	go on, the ones that write and the ones that read; and how many each
	list can have */
	MAXIMUM_NETWORK_BLOB_TASKS= 32,
	_network_blob_task_queue_write= 0,
	_network_blob_task_queue_read,
	NUMBER_OF_NETWORK_BLOB_TASK_QUEUES,
	MAXIMUM_NETWORK_BLOB_TASK_QUEUE_COUNT= 16,
	/* the two kinds whose block gets its checksum when it starts to be
	written */
	_network_blob_kind1= 1,
	_network_blob_kind2= 2,
	/* the kind whose block is read and written in another way, with the
	fourth number of its functions and no memory. More ways that a task
	ends: what does it could not start or go on, in four ways and in any
	other; what was read is more than there is room for; and the block is
	not all there, which is the way after which it is tried again */
	_network_blob_kind3= 3,
	_network_blob_task_failed= 2,
	_network_blob_task_too_big= 9,
	/* of what does the tasks (unknown_004cf78c): what one of its things is,
	where its kind is, and the kind that reads */
	NETWORK_BLOB_OPERATION_SIZE= 0x14,
	NETWORK_BLOB_OPERATION_KIND_OFFSET= 0x4,
	_network_blob_operation_read= 0x2A
};

/* ---------- structures */

struct network_blob_header
{
	unsigned long checksum;
	unsigned long version;
	unsigned long size;
};

/* what has a block has functions, after its version: the fifth makes the
block here when it could not be read, and says whether it did; the sixth and
the seventh are told that it was written, and that it was read, and say
whether it is good */
struct network_blob;

struct network_blob_functions
{
	long unknown0[2];
	unsigned long version;
	long unknownC;
	boolean (__attribute__((stdcall)) *make)(struct network_blob *blob);
	boolean (__attribute__((stdcall)) *written)(struct network_blob *blob);
	boolean (__attribute__((stdcall)) *read)(struct network_blob *blob);
};

/* what it is asked for with, which is kept to ask again */
struct network_blob_request
{
	long unknown0[3];
};

/* what has a block: its functions; its bits and the ones that it was
started with; its kind; what it is doing, what it did last and how that
ended; what it was asked with; its task, or NONE; and the memory of the
block, its size and the most that it can be */
struct network_blob
{
	const struct network_blob_functions *functions;
	word flags;
	word options;
	short kind;
	short state;
	short last_state;
	short result;
	long unknown10;
	struct network_blob_request request;
	long task;
	void *memory;
	unsigned long size;
	long capacity;
};

typedef char verify_network_blob_size[sizeof(struct network_blob) == 0x30 ? 1 : -1];

/* a task: whose block it reads or writes, and nothing if it is free; what
does it once it has started, or NONE; and the size and the memory of the
block */
struct network_blob_task
{
	struct network_blob *blob;
	long handle;
	unsigned long size;
	void *memory;
	long unknown10;
};

typedef char verify_network_blob_task_size[sizeof(struct network_blob_task) == 0x14 ? 1 : -1];

/* ---------- prototypes/NETWORK_BLOB.C */

unsigned long network_blob_checksum(const struct network_blob_header *block, long size);
long network_blob_verify(unsigned long size, const struct network_blob_header *block, const struct network_blob *blob);
boolean network_blob_get_progress(const struct network_blob *blob, real *progress);
boolean __attribute__((stdcall)) network_blob_start_read(struct network_blob *blob, long unknown10,
	struct network_blob_request request, word options);
boolean __attribute__((stdcall)) network_blob_start_write(struct network_blob *blob, long unknown10,
	struct network_blob_request request, word options);
void network_blob_task_finished(struct network_blob *blob, long result);
void __attribute__((stdcall)) network_blob_memory_stolen(long unknown, struct network_blob *blob);

/* ---------- prototypes/NETWORK_BLOB_TASK.C */

void __attribute__((stdcall)) network_blob_task_end(long task, long result);
long network_blob_task_find_free(void);
void network_blob_tasks_reset(void);
long __attribute__((stdcall)) network_blob_task_start_write(struct network_blob *blob, void *memory,
	unsigned long size);
long __attribute__((stdcall)) network_blob_task_start_read(struct network_blob *blob, void *memory,
	unsigned long size);
void network_blob_tasks_update(void);
void __attribute__((stdcall)) network_blob_task_completed(long status, long handle);

/* ---------- globals */

extern struct network_blob_task unknown_004d8c28[MAXIMUM_NETWORK_BLOB_TASKS];
extern long unknown_004d8ba8[NUMBER_OF_NETWORK_BLOB_TASK_QUEUES][MAXIMUM_NETWORK_BLOB_TASK_QUEUE_COUNT];

#endif
