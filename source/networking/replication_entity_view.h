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
	/* more of a view: three bytes; the bit of which view it is; the first of
	the things that it has sent and waits to hear of, and a number; the
	place of the first of the things that are in some state in it; and six
	counts: the second is of the things in the first state, the fourth of
	the things that it has something to say of, and the sixth of the things
	that are being done with and that it has still */
	REPLICATION_ENTITY_VIEW_UNKNOWN8_OFFSET= 0x8,
	REPLICATION_ENTITY_VIEW_UNKNOWN9_OFFSET= 0x9,
	REPLICATION_ENTITY_VIEW_UNKNOWNA_OFFSET= 0xA,
	REPLICATION_ENTITY_VIEW_BIT_OFFSET= 0x10,
	REPLICATION_ENTITY_VIEW_UNKNOWN1C_OFFSET= 0x1C,
	REPLICATION_ENTITY_VIEW_SENT_OFFSET= 0x18,
	REPLICATION_ENTITY_VIEW_FIRST_OFFSET= 0x5024,
	REPLICATION_ENTITY_VIEW_UNKNOWN5028_OFFSET= 0x5028,
	REPLICATION_ENTITY_VIEW_STATE1_COUNT_OFFSET= 0x502C,
	REPLICATION_ENTITY_VIEW_UNKNOWN5030_OFFSET= 0x5030,
	REPLICATION_ENTITY_VIEW_UNKNOWN5038_OFFSET= 0x5038,
	REPLICATION_ENTITY_VIEW_DELETING_COUNT_OFFSET= 0x503C,
	/* the states of a thing in a view: it is not there, and the first one
	that it is in */
	_replication_entity_view_state_none= 0,
	_replication_entity_view_state1= 1,
	_replication_entity_view_state2= 2
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
void replication_entity_view_new(struct replication_entity_manager *manager, long index, byte *view);
long replication_entity_view_next_entity(long entity_index, const byte *view);
boolean replication_entity_view_unknown_00099640(long entity_index, const byte *view);
byte *__attribute__((stdcall)) replication_entity_view_set_entity_state(byte *view, long entity_index, long state);

#endif
