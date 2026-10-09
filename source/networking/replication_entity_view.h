/*
REPLICATION_ENTITY_VIEW.H

A view of the things that the consoles of a game have in common: what one
of the other consoles knows of each of them, and what it is still to be
told. The name of the file is a guess, and so are the names of its
functions.
*/

#ifndef __REPLICATION_ENTITY_VIEW_H
#define __REPLICATION_ENTITY_VIEW_H

#include "cseries/cseries.h"
#include "networking/replication_entity_manager.h"

/* ---------- constants */

enum
{
	/* more of a view: the first of the things that it has sent and waits to
	hear of; a number; and six counts, of which the fourth is of the things
	that it has something to say of */
	REPLICATION_ENTITY_VIEW_SENT_OFFSET= 0x18,
	REPLICATION_ENTITY_VIEW_UNKNOWN5024_OFFSET= 0x5024,
	REPLICATION_ENTITY_VIEW_UNKNOWN5028_OFFSET= 0x5028,
	REPLICATION_ENTITY_VIEW_UNKNOWN502C_OFFSET= 0x502C,
	REPLICATION_ENTITY_VIEW_UNKNOWN5030_OFFSET= 0x5030,
	REPLICATION_ENTITY_VIEW_UNKNOWN5038_OFFSET= 0x5038,
	REPLICATION_ENTITY_VIEW_UNKNOWN503C_OFFSET= 0x503C,
	/* the states of a thing in a view: it is not there, and the first one
	that it is in */
	_replication_entity_view_state_none= 0,
	_replication_entity_view_state1= 1
};

/* ---------- structures */

/* the functions of a view, of which the ninth takes back one of the things
that it has sent, by its number */
struct replication_entity_view_functions
{
	void *unknown0[8];
	void (__attribute__((thiscall)) *unsent)(byte *view, long unknown, long unknown2);
};

/* ---------- prototypes/REPLICATION_ENTITY_VIEW.C */

void replication_entity_view_reset(byte *view);
void replication_entity_view_mark_entity(long entity_index, byte *view, unsigned long flags);
void replication_entity_view_delete_entity(byte *view, long entity_index);

/* 0x00099690: a thing is in a state in a view, which is told so; the view */
byte *__attribute__((stdcall)) unknown_00099690(byte *view, long entity, long state);

#endif
