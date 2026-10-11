/*
MAIN_MAP_PRECACHE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The map that is copied to the hard disk ahead of time. The names are
guesses.
*/

#include "main_map_precache.h"

#include "cache/cache_files.h"

#include <stddef.h>

/* ---------- prototypes */

/* 0x00163680: which map to copy now, if any. 0x00163610: the name of the
map that comes after this one in the campaign, or NULL */
boolean __attribute__((stdcall)) unknown_00163680(struct main_map_precache_request *request);
const char *unknown_00163610(void);

/* ---------- code */

/* 0x00163820: each frame: the map that there is to copy is asked for; one
that is only wanted if nothing is known of it, only then */
void main_map_precache_update(
	void)
{
	struct main_map_precache_request request;

	if (unknown_00163680(&request))
	{
		if (request.only_if_unknown)
		{
			if (cached_map_file_status(request.name)==_cached_map_file_status_none)
			{
				cached_map_file_request(request.name, request.priority);
			}
		}
		else
		{
			cached_map_file_request(request.name, request.priority);
		}
	}
}

/* 0x00163870: whether no map is being copied */
boolean main_map_precache_copy_idle(
	void)
{
	return !cached_map_file_copy_get_progress(NULL);
}

/* 0x00163b60: whether the map that comes after this one is not on the hard
disk yet: nothing is known of it, or it waits, or it is being copied. Not if
there is no such map, or its copy is done or failed */
boolean main_map_precache_next_map_not_ready(
	void)
{
	const char *name= unknown_00163610();

	if (name)
	{
		switch (cached_map_file_status(name))
		{
		case _cached_map_file_status_none:
		case _cached_map_file_status_pending:
		case _cached_map_file_status_copying:
			return TRUE;
		}
	}

	return FALSE;
}
