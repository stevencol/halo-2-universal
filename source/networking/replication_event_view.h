/*
REPLICATION_EVENT_VIEW.H

A view of the things that happen in a game: what one of the other consoles
is still to be told of them, and what it was told in packets that are not
heard of yet. The name of the file is a guess, and so are the names of its
functions.
*/

#ifndef __REPLICATION_EVENT_VIEW_H
#define __REPLICATION_EVENT_VIEW_H

#include "cseries/cseries.h"
#include "networking/replication_event_manager.h"

/* ---------- constants */

enum
{
	/* more of a view: the first of the packets that it wrote in and waits to
	hear of; what keeps the things that happen; and three numbers */
	REPLICATION_EVENT_VIEW_SENT_OFFSET= 0x10,
	REPLICATION_EVENT_VIEW_MANAGER_OFFSET= 0x18,
	REPLICATION_EVENT_VIEW_UNKNOWN1C_OFFSET= 0x1C,
	REPLICATION_EVENT_VIEW_UNKNOWN24_OFFSET= 0x24
};

/* ---------- structures */

/* what a view keeps of what it wrote in a packet: of each thing that
happened, which, and the next one; and of the packet, its number, the first
of those, and the next packet */
struct replication_event_view_item
{
	struct replication_event *event;
	struct replication_event_view_item *next;
};

struct replication_event_view_packet
{
	long number;
	struct replication_event_view_item *items;
	struct replication_event_view_packet *next;
};

/* ---------- prototypes/REPLICATION_EVENT_VIEW.C */

void __attribute__((stdcall)) replication_event_view_reset(byte *view);

#endif
