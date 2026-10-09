/*
REPLICATION_EVENT_MANAGER.H

What keeps the things that happen in a game and that the other consoles are
to be told of: a list of them, each kept until every view that it is for has
told its console. The name of the file is a guess, and so are the names of
its functions.
*/

#ifndef __REPLICATION_EVENT_MANAGER_H
#define __REPLICATION_EVENT_MANAGER_H

#include "cseries/cseries.h"
#include "networking/network_heap.h"

/* ---------- constants */

enum
{
	MAXIMUM_REPLICATION_EVENT_VIEWS= 15,
	/* of a view: which of the views it is, and how many things it has to
	tell of */
	REPLICATION_EVENT_VIEW_INDEX_OFFSET= 0xC,
	REPLICATION_EVENT_VIEW_EVENT_COUNT_OFFSET= 0x20,
	/* the state of a thing that is to be told of; and the bit that says
	that a thing is let go of itself, and not only what it has */
	_replication_event_state_pending= 1,
	_replication_event_free_event_bit= 0x1
};

/* ---------- structures */

/* one of the things that happen: its state; when it was to be told of;
what it has more, which is taken from the heap of the network as it is; a
bit for each view that has it still to tell; and the next one */
struct replication_event
{
	long state;
	long unknown4;
	long time;
	long unknownC;
	long unknown10;
	long unknown14;
	void *data;
	long unknown1C;
	unsigned long view_mask;
	long unknown24;
	struct replication_event *next;
};

typedef char verify_replication_event_size[sizeof(struct replication_event) == 0x2C || sizeof(void *) != 4 ? 1 : -1];

/* what the things are of: something with functions, of which the fourth is
told that a thing has no view to be told by */
struct replication_event_client;

struct replication_event_client_functions
{
	void *unknown0[3];
	void (__attribute__((thiscall)) *dropped)(struct replication_event_client *client, struct replication_event *event);
};

struct replication_event_client
{
	const struct replication_event_client_functions *functions;
};

struct replication_event_manager
{
	long unknown0;
	struct replication_event_client *client;
	long unknown8;
	byte *views[MAXIMUM_REPLICATION_EVENT_VIEWS];
	long event_count;
	struct replication_event *events;
};

typedef char verify_replication_event_manager_size[sizeof(struct replication_event_manager) == 0x50 ||
	sizeof(void *) != 4 ? 1 : -1];

/* ---------- prototypes/REPLICATION_EVENT_MANAGER.C */

void replication_event_manager_unknown_00089dc0(struct replication_event_manager *manager);
struct replication_event *replication_event_manager_new_event(struct replication_event_manager *manager);
void replication_event_manager_remove_event(struct replication_event *event, struct replication_event_manager *manager);
struct replication_event *__attribute__((stdcall)) replication_event_free(struct replication_event *event,
	unsigned long flags);
void __attribute__((stdcall)) replication_event_manager_send_event(struct replication_event *event,
	struct replication_event_manager *manager, unsigned long view_mask);

#endif
