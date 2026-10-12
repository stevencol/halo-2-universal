/*
PROGRESS.C

saying that a long piece of work starts and ends, to whoever shows it. Worked
out from halo2ship.exe; the names are guesses.
*/

#include "cseries/cseries.h"

#include <string.h>

#include "cseries/progress.h"

/* ---------- globals */

/* 0x004f55f8 */
struct progress_globals progress_globals;

/* ---------- code */

/* 0x001e9650: the piece of work that goes on, if any, ends: its function is
told, unless nobody is to be */
void progress_done(
	void)
{
	if (progress_globals.active)
	{
		if (!progress_globals.disabled)
		{
			if (progress_globals.callbacks.done_proc)
			{
				progress_globals.callbacks.done_proc(progress_globals.description, progress_globals.callbacks.data);
			}
			else if (progress_globals.default_callbacks.done_proc)
			{
				progress_globals.default_callbacks.done_proc(progress_globals.description,
					progress_globals.default_callbacks.data);
			}
		}
		progress_globals.active= FALSE;
	}
}

/* 0x001e95d0: a piece of work starts, with what it is called; the one that
went on ends first */
void __attribute__((stdcall)) progress_new(
	const char *description)
{
	progress_done();
	progress_globals.active= TRUE;
	strncpy(progress_globals.description, description, PROGRESS_DESCRIPTION_LENGTH+1);
	progress_globals.description[PROGRESS_DESCRIPTION_LENGTH]= 0;
	progress_globals.unknown124= NONE;
	progress_globals.unknown128= 0;
	if (!progress_globals.disabled)
	{
		if (progress_globals.callbacks.new_proc)
		{
			progress_globals.callbacks.new_proc(progress_globals.description, progress_globals.callbacks.data);
		}
		else if (progress_globals.default_callbacks.new_proc)
		{
			progress_globals.default_callbacks.new_proc(progress_globals.description,
				progress_globals.default_callbacks.data);
		}
	}
}
