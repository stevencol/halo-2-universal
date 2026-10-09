/*
REPLICATION_EVENT_MANAGER.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What keeps the things that happen in a game and that the other consoles are
to be told of: a list of them, each kept until every view that it is for has
told its console. The name of the file is a guess, and so are the names of
its functions.
*/

#include "replication_event_manager.h"

#include "networking/network_observer.h"

/* ---------- prototypes of functions not decompiled yet */

/* 0x00096ed0: something that is done with a view of the things that
happen */
void __attribute__((stdcall)) unknown_00096ed0(byte *view);

/* ---------- globals */

/* 0x004d87f8: the heap of the network */
struct network_heap *unknown_004d87f8;

/* ---------- code */

/* 0x00089dc0: something is done with each view that there is
(unknown_00096ed0) */
void replication_event_manager_unknown_00089dc0(
	struct replication_event_manager *manager)
{
	long index;

	for (index= 0; index<MAXIMUM_REPLICATION_EVENT_VIEWS; index++)
	{
		if (manager->views[index])
		{
			unknown_00096ed0(manager->views[index]);
		}
	}
}

/* 0x00089df0: a new thing that happens, the last of the list; NULL if the
heap of the network has no memory for it, even after making room. It has
nothing yet */
struct replication_event *replication_event_manager_new_event(
	struct replication_event_manager *manager)
{
	struct network_heap *heap= unknown_004d87f8;
	struct replication_event *event= heap->allocator->functions->allocate(heap->allocator, sizeof(struct replication_event),
		0, 0);
	struct replication_event **link;

	if (!event)
	{
		heap->allocator->functions->compact(heap->allocator, 0);
		event= heap->allocator->functions->allocate(heap->allocator, sizeof(struct replication_event), 0, 0);
		if (!event)
		{
			return NULL;
		}
	}
	heap->count++;
	event->unknown4= NONE;
	event->unknownC= NONE;
	event->state= 0;
	event->unknown10= NONE;
	event->unknown14= NONE;
	event->data= NULL;
	event->unknown1C= 0;
	event->view_mask= 0;
	event->unknown24= 0;
	for (link= &manager->events; *link; link= &(*link)->next)
	{
	}
	*link= event;
	event->next= NULL;
	manager->event_count++;

	return event;
}

/* 0x00089e70: a thing that happens is not in the list any more, and is let
go of (replication_event_free); there is one less, whether it was in the
list or not */
void replication_event_manager_remove_event(
	struct replication_event *event,
	struct replication_event_manager *manager)
{
	struct replication_event **link;

	for (link= &manager->events; *link; link= &(*link)->next)
	{
		if (*link==event)
		{
			*link= event->next;
			break;
		}
	}
	manager->event_count--;
	if (event)
	{
		replication_event_free(event, _replication_event_free_event_bit);
	}
}

/* 0x00089eb0: the heap of the network takes back what a thing that happens
has more, and with the first bit the thing itself. The heap is asked how
much each is first, and nothing is done with that. The thing */
struct replication_event *__attribute__((stdcall)) replication_event_free(
	struct replication_event *event,
	unsigned long flags)
{
	void *data= event->data;

	if (data)
	{
		network_heap_free(data);
	}
	if (flags&_replication_event_free_event_bit)
	{
		network_heap_free(event);
	}

	return event;
}

/* 0x00089f20: a thing that happens is to be told of, now, by the views
that a mask says: each of those that there is has it still to tell, by the
bit of which view it is (the processor shifts by its low five bits), and
has one more. With no view for it, what the things are of is told, by its
fourth function, and the thing is not kept. The game has
replication_event_manager_remove_event written here */
void __attribute__((stdcall)) replication_event_manager_send_event(
	struct replication_event *event,
	struct replication_event_manager *manager,
	unsigned long view_mask)
{
	boolean any= FALSE;
	long index;

	event->state= _replication_event_state_pending;
	event->time= network_time_since(0);
	for (index= 0; index<MAXIMUM_REPLICATION_EVENT_VIEWS; index++)
	{
		byte *view= manager->views[index];

		if ((view_mask&(1u<<index)) && view)
		{
			event->view_mask|= 1u<<(*(const long *)(view+REPLICATION_EVENT_VIEW_INDEX_OFFSET)&31);
			(*(long *)(view+REPLICATION_EVENT_VIEW_EVENT_COUNT_OFFSET))++;
			any= TRUE;
		}
	}
	if (!any)
	{
		manager->client->functions->dropped(manager->client, event);
		replication_event_manager_remove_event(event, manager);
	}
}
