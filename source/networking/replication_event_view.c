/*
REPLICATION_EVENT_VIEW.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

A view of the things that happen in a game: what one of the other consoles
is still to be told of them, and what it was told in packets that are not
heard of yet. The name of the file is a guess, and so are the names of its
functions.
*/

#include "replication_event_view.h"

#include "networking/replication_entity_manager.h"

/* ---------- code */

/* 0x00096ed0: a view starts over. What it kept of the packets that it wrote
in is taken back by the heap of the network, and each thing that happened
that was in one of them is not sent by the view any more. Then each thing
that the view had still to tell has not: and one that no view has still to
tell is not kept: what the things are of is told, and it is taken from the
list and let go of, as replication_event_manager_send_event does with one
that has no view. And the view has no counts */
void __attribute__((stdcall)) replication_event_view_reset(
	byte *view)
{
	struct replication_event_view_packet *packet= *(struct replication_event_view_packet **)(view+REPLICATION_EVENT_VIEW_SENT_OFFSET);
	struct replication_event_manager *manager;
	struct replication_event *event;
	struct replication_event *next_event;

	*(struct replication_event_view_packet **)(view+REPLICATION_EVENT_VIEW_SENT_OFFSET)= NULL;
	while (packet)
	{
		struct replication_event_view_packet *next_packet= packet->next;
		struct replication_event_view_item *item= packet->items;

		while (item)
		{
			struct replication_event_view_item *next_item= item->next;

			item->event->sent_mask&= ~(1u<<(*(const long *)(view+REPLICATION_EVENT_VIEW_INDEX_OFFSET)&31));
			network_heap_free(item);
			item= next_item;
		}
		network_heap_free(packet);
		packet= next_packet;
	}
	manager= *(struct replication_event_manager **)(view+REPLICATION_EVENT_VIEW_MANAGER_OFFSET);
	for (event= manager->events; event; event= next_event)
	{
		unsigned long bit= 1u<<(*(const long *)(view+REPLICATION_EVENT_VIEW_INDEX_OFFSET)&31);

		next_event= event->next;
		if (event->view_mask&bit)
		{
			manager= *(struct replication_event_manager **)(view+REPLICATION_EVENT_VIEW_MANAGER_OFFSET);
			event->view_mask&= ~bit;
			if (event->view_mask==0)
			{
				manager->client->functions->dropped(manager->client, event);
				replication_event_manager_remove_event(event, manager);
			}
		}
	}
	*(long *)(view+REPLICATION_EVENT_VIEW_EVENT_COUNT_OFFSET)= 0;
	*(long *)(view+REPLICATION_EVENT_VIEW_UNKNOWN1C_OFFSET)= 0;
	*(long *)(view+REPLICATION_EVENT_VIEW_UNKNOWN24_OFFSET)= 0;
}

/* 0x000979f0: whether one of the things that the consoles share is being
taken away, for the other console of a view: it is the one that the view of
them has at its place, it was made, and it is being taken away. FALSE if
not, and for a thing that the view has not */
boolean replication_event_view_entity_deleting(
	const byte *owner,
	long entity_index)
{
	const byte *entity_view= *(const byte *const *)(*(const byte *const *)(owner+REPLICATION_EVENT_VIEW_OWNER_VIEW_OFFSET)+
		REPLICATION_EVENT_VIEW_OWNER_VIEW_UNKNOWN8_OFFSET)+REPLICATION_EVENT_VIEW_OWNER_ENTITY_VIEW_OFFSET;
	unsigned long index= (unsigned long)entity_index&(MAXIMUM_REPLICATION_ENTITIES-1);

	if (index<MAXIMUM_REPLICATION_ENTITIES &&
		((const struct replication_entity_view_entity *)(entity_view+REPLICATION_ENTITY_VIEW_ENTITIES_OFFSET))[index].entity_index==entity_index)
	{
		const struct replication_entity_manager *manager=
			*(const struct replication_entity_manager *const *)(entity_view+REPLICATION_ENTITY_VIEW_MANAGER_OFFSET);
		byte flags= manager->entities[index].flags;

		if (flags&_replication_entity_created_bit)
		{
			return (flags&_replication_entity_deleting_bit)!=0;
		}
	}

	return FALSE;
}
