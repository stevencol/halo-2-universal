/*
PROGRESS.H

saying that a long piece of work starts and ends, to whoever shows it. The
names are guesses.
*/

#ifndef __PROGRESS_H
#define __PROGRESS_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	PROGRESS_DESCRIPTION_LENGTH= 0xFF
};

/* ---------- structures */

/* told of a piece of work, with what it is called and with what was given
with the function */
typedef void (__attribute__((stdcall)) *progress_proc)(const char *description, void *data);

struct progress_callbacks
{
	progress_proc new_proc;
	/* never seen called */
	void *unknown4;
	progress_proc done_proc;
	void *data;
};

struct progress_globals
{
	/* those that someone has set are told; if there are none, these are */
	struct progress_callbacks default_callbacks;
	struct progress_callbacks callbacks;
	/* nobody is told */
	boolean disabled;
	/* a piece of work goes on */
	boolean active;
	char description[PROGRESS_DESCRIPTION_LENGTH+1];
	/* NONE and zero when a piece of work starts */
	long unknown124;
	long unknown128;
};

typedef char verify_progress_globals_size[
	sizeof(struct progress_globals) == 0x12C ? 1 : -1];
typedef char verify_progress_globals_description_offset[
	__builtin_offsetof(struct progress_globals, description) == 0x22 ? 1 : -1];

/* ---------- prototypes/PROGRESS.C */

void __attribute__((stdcall)) progress_new(const char *description);
void progress_done(void);

#endif
