/*
REPLICATION_ENTITY_VIEW.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

A view of the things that the consoles of a game have in common: what one
of the other consoles knows of each of them, and what it is still to be
told. The name of the file is a guess, and so are the names of its
functions.
*/

#include "replication_entity_view.h"

#include "networking/network_heap.h"
#include "networking/network_observer.h"

#include <string.h>

/* ---------- prototypes of functions not decompiled yet */

/* 0x000b5650: the number of a thing is written in a stream */
void __attribute__((stdcall)) unknown_000b5650(long entity_index, struct bitstream *stream);

/* ---------- code */

/* what a view has of the thing of a place */
static inline __attribute__((always_inline)) struct replication_entity_view_entity *replication_entity_view_entity_get(
	byte *view,
	long index)
{
	return (struct replication_entity_view_entity *)(view+REPLICATION_ENTITY_VIEW_ENTITIES_OFFSET)+index;
}

/* 0x00097fe0: a view starts over. Each packet that it wrote in and waits to
hear of is as one that did not come, by its ninth function, until there is
none. Each
thing that is in some state in it is not in it any more; if the thing's count
of what still has it had the bit of the view, it has not, and with no bit
left the thing is done with: what the things are of is told, and its place
is not in use. And the view has no counts */
void replication_entity_view_reset(
	byte *view)
{
	long index;

	while (*(long **)(view+REPLICATION_ENTITY_VIEW_SENT_OFFSET))
	{
		(*(const struct replication_entity_view_functions **)view)->packet_result(view,
			**(long **)(view+REPLICATION_ENTITY_VIEW_SENT_OFFSET), FALSE);
	}
	for (index= 0; index<MAXIMUM_REPLICATION_ENTITIES; index++)
	{
		struct replication_entity_view_entity *view_entity= replication_entity_view_entity_get(view, index);

		if (view_entity->state!=_replication_entity_view_state_none)
		{
			long entity_index= view_entity->entity_index;
			struct replication_entity_manager *manager=
				*(struct replication_entity_manager **)(view+REPLICATION_ENTITY_VIEW_MANAGER_OFFSET);
			struct replication_entity *entity= &manager->entities[entity_index&REPLICATION_ENTITY_INDEX_MASK];
			word bit= (word)(1u<<(*(const long *)(view+REPLICATION_ENTITY_VIEW_INDEX_OFFSET)&31));

			if ((word)entity->unknown2&bit)
			{
				entity->unknown2= (short)((word)entity->unknown2&~bit);
				manager= *(struct replication_entity_manager **)(view+REPLICATION_ENTITY_VIEW_MANAGER_OFFSET);
				entity= &manager->entities[entity_index&REPLICATION_ENTITY_INDEX_MASK];
				if (entity->unknown2==0)
				{
					manager->client->functions->deleted(manager->client, entity_index);
					entity->flags&= ~_replication_entity_used_bit;
				}
			}
			view_entity->state= _replication_entity_view_state_none;
			view_entity->entity_index= NONE;
			view_entity->next= NONE;
			view_entity->unknown10= 0;
		}
	}
	*(long *)(view+REPLICATION_ENTITY_VIEW_STATE1_COUNT_OFFSET)= 0;
	*(long *)(view+REPLICATION_ENTITY_VIEW_UNKNOWN5028_OFFSET)= 0;
	*(long *)(view+REPLICATION_ENTITY_VIEW_PENDING_COUNT_OFFSET)= 0;
	*(long *)(view+REPLICATION_ENTITY_VIEW_UNKNOWN5030_OFFSET)= 0;
	*(long *)(view+REPLICATION_ENTITY_VIEW_FIRST_OFFSET)= NONE;
}

/* 0x000980d0: a view has more to say of a thing: the bits that are given;
what replication_entity_manager_mark_entity does with every view */
void replication_entity_view_mark_entity(
	long entity_index,
	byte *view,
	unsigned long flags)
{
	long index= entity_index&REPLICATION_ENTITY_INDEX_MASK;
	struct replication_entity_view_entity *view_entity= replication_entity_view_entity_get(view, index);

	if (view_entity->state==_replication_entity_view_state3)
	{
		const struct replication_entity_manager *manager=
			*(const struct replication_entity_manager **)(view+REPLICATION_ENTITY_VIEW_MANAGER_OFFSET);

		if (!((1u<<(*(const long *)(view+REPLICATION_ENTITY_VIEW_INDEX_OFFSET)&31))&(word)manager->entities[index].unknown2) &&
			view_entity->pending==0)
		{
			(*(long *)(view+REPLICATION_ENTITY_VIEW_PENDING_COUNT_OFFSET))++;
		}
	}
	view_entity->pending|= flags;
}

/* 0x000995c0: a view is told that a thing is being done with. One in its
first state there that has not the first bit of a number of it is in no
state (replication_entity_view_set_entity_state), and the view counts one more of those; that is
all. One that has the bit goes to the third state. Then the thing's count of
what still has it has the bit of the view; and if the thing is in its third
state there, the view has one less thing to say something of, if it had
something of this one, and counts one more of another kind */
void replication_entity_view_delete_entity(
	byte *view,
	long entity_index)
{
	long index= entity_index&REPLICATION_ENTITY_INDEX_MASK;
	struct replication_entity_manager *manager=
		*(struct replication_entity_manager **)(view+REPLICATION_ENTITY_VIEW_MANAGER_OFFSET);
	struct replication_entity *entity= &manager->entities[index];
	struct replication_entity_view_entity *view_entity= replication_entity_view_entity_get(view, index);

	if (view_entity->state==_replication_entity_view_state1)
	{
		if (!(view_entity->unknown10&1))
		{
			view= replication_entity_view_set_entity_state(view, entity_index, _replication_entity_view_state_none);
			(*(long *)(view+REPLICATION_ENTITY_VIEW_UNKNOWN5038_OFFSET))++;

			return;
		}
		view= replication_entity_view_set_entity_state(view, entity_index, _replication_entity_view_state3);
	}
	entity->unknown2= (short)((word)entity->unknown2|(word)(1u<<(*(const long *)(view+REPLICATION_ENTITY_VIEW_INDEX_OFFSET)&31)));
	if (view_entity->state==_replication_entity_view_state3)
	{
		if (view_entity->pending!=0)
		{
			(*(long *)(view+REPLICATION_ENTITY_VIEW_PENDING_COUNT_OFFSET))--;
		}
		(*(long *)(view+REPLICATION_ENTITY_VIEW_DELETING_COUNT_OFFSET))++;
	}
}

/* 0x00097f60: a view is set up: of what keeps the things, and the one of a
place among the views, with its bit; with nothing sent, no thing in any
state, and no counts */
void replication_entity_view_new(
	struct replication_entity_manager *manager,
	long view_index,
	byte *view)
{
	long index;

	*(struct replication_entity_manager **)(view+REPLICATION_ENTITY_VIEW_MANAGER_OFFSET)= manager;
	*(long *)(view+REPLICATION_ENTITY_VIEW_INDEX_OFFSET)= view_index;
	*(unsigned long *)(view+REPLICATION_ENTITY_VIEW_BIT_OFFSET)= 1u<<(view_index&31);
	*(long *)(view+REPLICATION_ENTITY_VIEW_SENT_OFFSET)= 0;
	*(long *)(view+REPLICATION_ENTITY_VIEW_UNKNOWN1C_OFFSET)= 0;
	for (index= 0; index<MAXIMUM_REPLICATION_ENTITIES; index++)
	{
		struct replication_entity_view_entity *view_entity= replication_entity_view_entity_get(view, index);

		view_entity->entity_index= NONE;
		view_entity->pending= 0;
		view_entity->state= _replication_entity_view_state_none;
		view_entity->next= NONE;
		view_entity->unknownC= 0;
		view_entity->unknown10= 0;
	}
	*(long *)(view+REPLICATION_ENTITY_VIEW_FIRST_OFFSET)= NONE;
	*(long *)(view+REPLICATION_ENTITY_VIEW_STATE1_COUNT_OFFSET)= 0;
	*(long *)(view+REPLICATION_ENTITY_VIEW_UNKNOWN5028_OFFSET)= 0;
	*(long *)(view+REPLICATION_ENTITY_VIEW_DELETING_COUNT_OFFSET)= 0;
	*(long *)(view+REPLICATION_ENTITY_VIEW_UNKNOWN5038_OFFSET)= 0;
	*(long *)(view+REPLICATION_ENTITY_VIEW_PENDING_COUNT_OFFSET)= 0;
	*(long *)(view+REPLICATION_ENTITY_VIEW_UNKNOWN5030_OFFSET)= 0;
	view[REPLICATION_ENTITY_VIEW_UNKNOWN9_OFFSET]= FALSE;
	view[REPLICATION_ENTITY_VIEW_UNKNOWNA_OFFSET]= FALSE;
	view[REPLICATION_ENTITY_VIEW_UNKNOWN8_OFFSET]= TRUE;
}

/* 0x00098480: the number of the next thing, after one, that is in its first
or second state in a view; the one after NONE is the first; NONE if there is
no more */
long replication_entity_view_next_entity(
	long entity_index,
	const byte *view)
{
	long index= entity_index==NONE ? 0 : (entity_index&REPLICATION_ENTITY_INDEX_MASK)+1;

	for (; index<MAXIMUM_REPLICATION_ENTITIES; index++)
	{
		const struct replication_entity_view_entity *view_entity=
			(const struct replication_entity_view_entity *)(view+REPLICATION_ENTITY_VIEW_ENTITIES_OFFSET)+index;

		if (view_entity->state==_replication_entity_view_state1 || view_entity->state==_replication_entity_view_state2)
		{
			return view_entity->entity_index;
		}
	}

	return NONE;
}

/* 0x00099640: whether a thing, which a view has by that number, is not made
yet, or is in its third state in the view and not being done with */
boolean replication_entity_view_unknown_00099640(
	long entity_index,
	const byte *view)
{
	long index= entity_index&REPLICATION_ENTITY_INDEX_MASK;
	const struct replication_entity_view_entity *view_entity=
		(const struct replication_entity_view_entity *)(view+REPLICATION_ENTITY_VIEW_ENTITIES_OFFSET)+index;
	const struct replication_entity_manager *manager;
	byte flags;

	if (view_entity->entity_index!=entity_index)
	{
		return FALSE;
	}
	manager= *(const struct replication_entity_manager **)(view+REPLICATION_ENTITY_VIEW_MANAGER_OFFSET);
	flags= manager->entities[index].flags;

	return !(flags&_replication_entity_created_bit) ||
		(view_entity->state==_replication_entity_view_state3 && !(flags&_replication_entity_deleting_bit));
}

/* 0x00099690: a thing is in a state in a view; the view. The counts of the
view follow: of the things in the first state; and, of the ones in the
third, of those that are being done with and the view has still, or else of
those that it has something to say of. A thing that was in no state is in
the view from now, by its number, with nothing to say of it, and the first
of the things that are; one that goes to no state is not in the view, nor
among the things that are, and has no number */
byte *__attribute__((stdcall)) replication_entity_view_set_entity_state(
	byte *view,
	long entity_index,
	long state)
{
	long index= entity_index&REPLICATION_ENTITY_INDEX_MASK;
	struct replication_entity_view_entity *view_entity= replication_entity_view_entity_get(view, index);
	const struct replication_entity_manager *manager;
	const struct replication_entity *entity;
	boolean has_bit;

	if (state==view_entity->state)
	{
		return view;
	}
	manager= *(const struct replication_entity_manager **)(view+REPLICATION_ENTITY_VIEW_MANAGER_OFFSET);
	entity= &manager->entities[index];
	has_bit= ((word)entity->unknown2&(1u<<(*(const long *)(view+REPLICATION_ENTITY_VIEW_INDEX_OFFSET)&31)))!=0;
	if (view_entity->state==_replication_entity_view_state1)
	{
		(*(long *)(view+REPLICATION_ENTITY_VIEW_STATE1_COUNT_OFFSET))--;
	}
	else if (view_entity->state==_replication_entity_view_state3)
	{
		if (has_bit)
		{
			(*(long *)(view+REPLICATION_ENTITY_VIEW_DELETING_COUNT_OFFSET))--;
		}
		else if (view_entity->pending!=0)
		{
			(*(long *)(view+REPLICATION_ENTITY_VIEW_PENDING_COUNT_OFFSET))--;
		}
	}
	if (state==_replication_entity_view_state1)
	{
		(*(long *)(view+REPLICATION_ENTITY_VIEW_STATE1_COUNT_OFFSET))++;
	}
	else if (state==_replication_entity_view_state3)
	{
		if (has_bit)
		{
			(*(long *)(view+REPLICATION_ENTITY_VIEW_DELETING_COUNT_OFFSET))++;
		}
		else if (view_entity->pending!=0)
		{
			(*(long *)(view+REPLICATION_ENTITY_VIEW_PENDING_COUNT_OFFSET))++;
		}
	}
	else if (state==_replication_entity_view_state_none)
	{
		view_entity->entity_index= NONE;
		if (*(const long *)(view+REPLICATION_ENTITY_VIEW_FIRST_OFFSET)==index)
		{
			*(long *)(view+REPLICATION_ENTITY_VIEW_FIRST_OFFSET)= view_entity->next;
		}
		else
		{
			long other;

			for (other= 0; other<MAXIMUM_REPLICATION_ENTITIES; other++)
			{
				struct replication_entity_view_entity *other_entity= replication_entity_view_entity_get(view, other);

				if (other_entity->next==index)
				{
					other_entity->next= view_entity->next;
					break;
				}
			}
		}
		view_entity->next= NONE;
		view_entity->unknown10= 0;
		view_entity->state= (short)state;

		return view;
	}
	if (view_entity->state==_replication_entity_view_state_none)
	{
		view_entity->entity_index= entity_index;
		view_entity->pending= 0;
		view_entity->unknownC= 0;
		view_entity->next= (short)*(const long *)(view+REPLICATION_ENTITY_VIEW_FIRST_OFFSET);
		*(long *)(view+REPLICATION_ENTITY_VIEW_FIRST_OFFSET)= index;
	}
	view_entity->state= (short)state;

	return view;
}

/* a stream keeps where it is, to go back to */
static inline __attribute__((always_inline)) void replication_entity_view_push_position(
	struct bitstream *stream)
{
	stream->positions[stream->position_count]= stream->position;
	stream->position_count++;
}

/* 0x00098620: a view writes in a packet that the thing that it has in a
place is made: 1 in three bits, its number (unknown_000b5650), and the thing
itself, which what the things are of writes, by its first function, with the
bits of what the view has to say of it. If that went well and so many bits
are left after it, the view keeps that it said so
(replication_entity_view_sent_creation) and, if something of how the thing is
was written, that too (replication_entity_view_sent_update); if not, the
stream goes back to where it was. Whether it was written */
boolean __attribute__((stdcall)) replication_entity_view_write_creation(
	struct bitstream *stream,
	byte *view,
	long index,
	long unknown,
	long bits_after)
{
	struct replication_entity_view_entity *view_entity= replication_entity_view_entity_get(view, index);
	long entity_index= view_entity->entity_index;
	struct replication_entity_manager *manager;
	unsigned long written= 0;

	replication_entity_view_push_position(stream);
	bitstream_write_integer(stream, _replication_entity_view_message_creation, REPLICATION_ENTITY_VIEW_MESSAGE_BITS);
	unknown_000b5650(entity_index, stream);
	manager= *(struct replication_entity_manager **)(view+REPLICATION_ENTITY_VIEW_MANAGER_OFFSET);
	if (manager->client->functions->write_creation(manager->client, entity_index, view_entity->pending, unknown, stream,
		bits_after, &written) &&
		(long)(((unsigned long)stream->size<<3)-(unsigned long)stream->position)>=bits_after)
	{
		stream->position_count--;
		replication_entity_view_sent_creation(entity_index, view);
		if (written!=0)
		{
			replication_entity_view_sent_update(entity_index, view, written);
		}

		return TRUE;
	}
	bitstream_pop_position(stream, TRUE);

	return FALSE;
}

/* 0x000986d0: a view writes in a packet that the thing that it has in a
place is done with: 2 in three bits, and its number. If so many bits are
left after it, the view keeps that it said so
(replication_entity_view_sent_deletion); if not, the stream goes back to
where it was. Whether it was written */
boolean __attribute__((stdcall)) replication_entity_view_write_deletion(
	long index,
	struct bitstream *stream,
	byte *view,
	long bits_after)
{
	long entity_index= replication_entity_view_entity_get(view, index)->entity_index;

	replication_entity_view_push_position(stream);
	bitstream_write_integer(stream, _replication_entity_view_message_deletion, REPLICATION_ENTITY_VIEW_MESSAGE_BITS);
	unknown_000b5650(entity_index, stream);
	if ((long)(((unsigned long)stream->size<<3)-(unsigned long)stream->position)>=bits_after)
	{
		stream->position_count--;
		replication_entity_view_sent_deletion(view, entity_index);

		return TRUE;
	}
	bitstream_pop_position(stream, TRUE);

	return FALSE;
}

/* what a view keeps of having said in a packet that a thing is made or done
with: the first of the ones of the packet that it is writing; with no memory
for it, the view notes that */
static inline __attribute__((always_inline)) void replication_entity_view_keep_creation(
	byte *view,
	long entity_index,
	long type)
{
	struct replication_entity_view_creation *creation= network_heap_allocate(sizeof(struct replication_entity_view_creation));

	if (creation)
	{
		struct replication_entity_view_packet *packet;

		creation->type= 0;
		creation->next= NULL;
		packet= *(struct replication_entity_view_packet **)(view+REPLICATION_ENTITY_VIEW_PACKET_OFFSET);
		creation->next= packet->creations;
		packet= *(struct replication_entity_view_packet **)(view+REPLICATION_ENTITY_VIEW_PACKET_OFFSET);
		packet->creations= creation;
		creation->entity_index= entity_index;
		creation->type= type;
	}
	else
	{
		view[REPLICATION_ENTITY_VIEW_NO_MEMORY_OFFSET]= TRUE;
	}
}

/* 0x00098ac0: a view said in the packet that it is writing that a thing is
made: it keeps that, a number of the thing in the view has its first bit,
and the thing is in its second state there */
void replication_entity_view_sent_creation(
	long entity_index,
	byte *view)
{
	replication_entity_view_keep_creation(view, entity_index, _replication_entity_view_message_creation);
	replication_entity_view_entity_get(view, entity_index&REPLICATION_ENTITY_INDEX_MASK)->unknown10|= 1;
	replication_entity_view_set_entity_state(view, entity_index, _replication_entity_view_state2);
}

/* 0x00098b60: a view said that a thing is done with: it keeps that, and
the thing is in its fourth state there */
void __attribute__((stdcall)) replication_entity_view_sent_deletion(
	byte *view,
	long entity_index)
{
	replication_entity_view_keep_creation(view, entity_index, _replication_entity_view_message_deletion);
	replication_entity_view_set_entity_state(view, entity_index, _replication_entity_view_state4);
}

/* 0x00098bf0: a view said in the packet that it is writing something of
how a thing is: some bits. It keeps that, the first of the ones of the
packet; and what it said of the same thing before, in the last of the
packets that it waits to hear of that has it, knows that there is this later
one. With no memory for it, the view notes that. The view has those bits of
the thing no more to say, as of now; and if the thing is in its third state
there with nothing left to say, the view has one less to say something of */
void __attribute__((stdcall)) replication_entity_view_sent_update(
	long entity_index,
	byte *view,
	unsigned long flags)
{
	struct replication_entity_view_update *update= network_heap_allocate(sizeof(struct replication_entity_view_update));
	struct replication_entity_view_entity *view_entity=
		replication_entity_view_entity_get(view, entity_index&REPLICATION_ENTITY_INDEX_MASK);

	if (update)
	{
		struct replication_entity_view_packet *packet;
		struct replication_entity_view_update *older= NULL;

		update->newer= NULL;
		update->next= NULL;
		packet= *(struct replication_entity_view_packet **)(view+REPLICATION_ENTITY_VIEW_PACKET_OFFSET);
		update->next= packet->updates;
		packet= *(struct replication_entity_view_packet **)(view+REPLICATION_ENTITY_VIEW_PACKET_OFFSET);
		packet->updates= update;
		for (packet= *(struct replication_entity_view_packet **)(view+REPLICATION_ENTITY_VIEW_SENT_OFFSET); packet;
			packet= packet->next)
		{
			struct replication_entity_view_update *other;

			for (other= packet->updates; other; other= other->next)
			{
				if (other->entity_index==entity_index)
				{
					older= other;
					break;
				}
			}
		}
		if (older)
		{
			older->newer= update;
		}
		update->entity_index= entity_index;
		update->flags= flags;
		update->newer= NULL;
	}
	else
	{
		view[REPLICATION_ENTITY_VIEW_NO_MEMORY_OFFSET]= TRUE;
	}
	view_entity->pending&= ~flags;
	view_entity->unknownC= network_time_since(0);
	if (view_entity->state==_replication_entity_view_state3 && view_entity->pending==0)
	{
		(*(long *)(view+REPLICATION_ENTITY_VIEW_PENDING_COUNT_OFFSET))--;
	}
}

/* 0x00098750: a view writes in a packet that the things that were made at
once with the one that it has in a place are made: 3 in three bits, how many
they are less two in two, and for each its number and the thing itself, as
replication_entity_view_write_creation does. If they all went well and so
many bits are left, the view keeps what it said of each; if not, the stream
goes back to where it was. Whether it was written. The game has
replication_entity_manager_get_chain and bitstream_write_integer_checked
written here */
boolean __attribute__((stdcall)) replication_entity_view_write_chain_creation(
	long index,
	struct bitstream *stream,
	byte *view,
	long unknown,
	long bits_after)
{
	long entities[MAXIMUM_REPLICATION_ENTITY_CHAIN];
	unsigned long written[MAXIMUM_REPLICATION_ENTITY_CHAIN]= {0};
	long count= replication_entity_manager_get_chain(replication_entity_view_entity_get(view, index)->entity_index, entities,
		*(const struct replication_entity_manager **)(view+REPLICATION_ENTITY_VIEW_MANAGER_OFFSET));
	long place;

	replication_entity_view_push_position(stream);
	bitstream_write_integer(stream, _replication_entity_view_message_chain_creation, REPLICATION_ENTITY_VIEW_MESSAGE_BITS);
	bitstream_write_integer_checked((unsigned long)(count-REPLICATION_ENTITY_VIEW_CHAIN_COUNT_LEAST),
		REPLICATION_ENTITY_VIEW_CHAIN_COUNT_BITS, stream);
	for (place= 0; place<count; place++)
	{
		struct replication_entity_manager *manager;
		long entity_index= entities[place];

		unknown_000b5650(entity_index, stream);
		manager= *(struct replication_entity_manager **)(view+REPLICATION_ENTITY_VIEW_MANAGER_OFFSET);
		if (!manager->client->functions->write_creation(manager->client, entity_index,
			replication_entity_view_entity_get(view, entity_index&REPLICATION_ENTITY_INDEX_MASK)->pending, unknown, stream,
			bits_after, &written[place]))
		{
			bitstream_pop_position(stream, TRUE);

			return FALSE;
		}
	}
	if ((long)(((unsigned long)stream->size<<3)-(unsigned long)stream->position)<bits_after)
	{
		bitstream_pop_position(stream, TRUE);

		return FALSE;
	}
	stream->position_count--;
	for (place= 0; place<count; place++)
	{
		replication_entity_view_sent_creation(entities[place], view);
		if (written[place]!=0)
		{
			replication_entity_view_sent_update(entities[place], view, written[place]);
		}
	}

	return TRUE;
}

/* 0x000988f0: and that they are done with: 4 in three bits, how many less
two, and the number of each; as replication_entity_view_write_deletion */
boolean __attribute__((stdcall)) replication_entity_view_write_chain_deletion(
	long index,
	struct bitstream *stream,
	byte *view,
	long bits_after)
{
	long entities[MAXIMUM_REPLICATION_ENTITY_CHAIN];
	long count= replication_entity_manager_get_chain(replication_entity_view_entity_get(view, index)->entity_index, entities,
		*(const struct replication_entity_manager **)(view+REPLICATION_ENTITY_VIEW_MANAGER_OFFSET));
	long place;

	replication_entity_view_push_position(stream);
	bitstream_write_integer(stream, _replication_entity_view_message_chain_deletion, REPLICATION_ENTITY_VIEW_MESSAGE_BITS);
	bitstream_write_integer_checked((unsigned long)(count-REPLICATION_ENTITY_VIEW_CHAIN_COUNT_LEAST),
		REPLICATION_ENTITY_VIEW_CHAIN_COUNT_BITS, stream);
	for (place= 0; place<count; place++)
	{
		unknown_000b5650(entities[place], stream);
	}
	if ((long)(((unsigned long)stream->size<<3)-(unsigned long)stream->position)<bits_after)
	{
		bitstream_pop_position(stream, TRUE);

		return FALSE;
	}
	stream->position_count--;
	for (place= 0; place<count; place++)
	{
		replication_entity_view_sent_deletion(view, entities[place]);
	}

	return TRUE;
}

/* 0x00098fb0: a view is given what came in a packet of a thing.

That it is made, or that some are: each thing that the view has in one of
their places makes way (replication_entity_view_make_way), and if one cannot
no more is done. Then they are here: one, in its place, in use, with the
count of its place that its number says, and what the things are of is told,
by its third function; or some at once
(replication_entity_manager_create_entities_remote). And they are in their
third state in the view.

That it is done with, which is only heeded of a thing that the view has by
that number in its third state: one that is not of the ones made at once is
in no state, and done with now
(replication_entity_manager_delete_entity_now). Or that some are: the thing
has to be the first of some, and they the same ones that came, as many and
with the same numbers; then they are all in no state, and done with now.

How it is now: what the things are of is told, by its eighth function */
void __attribute__((stdcall)) replication_entity_view_handle_message(
	byte *view,
	struct replication_entity_view_message *message)
{
	long entity_index= message->entity_index;
	long index= entity_index&REPLICATION_ENTITY_INDEX_MASK;
	struct replication_entity_manager *manager;
	long place;

	switch (message->type)
	{
	case _replication_entity_view_message_creation:
	case _replication_entity_view_message_chain_creation:
	{
		boolean made_way= TRUE;

		for (place= 0; place<message->count; place++)
		{
			if (!replication_entity_view_make_way(message->entities[place], view, message->unknown1C[place],
				message->data[place]))
			{
				made_way= FALSE;
			}
		}
		if (!made_way)
		{
			return;
		}
		manager= *(struct replication_entity_manager **)(view+REPLICATION_ENTITY_VIEW_MANAGER_OFFSET);
		if (message->count==1)
		{
			struct replication_entity *entity= &manager->entities[index];

			entity->flags= _replication_entity_used_bit;
			entity->salt= (byte)((unsigned long)entity_index>>REPLICATION_ENTITY_SALT_SHIFT);
			entity->unknown2= 0;
			manager->client->functions->created(manager->client, entity_index, message->unknown1C[0], message->unknown2C[0],
				message->unknown3C, message->data[0]);
		}
		else
		{
			replication_entity_manager_create_entities_remote(message->unknown1C, manager, message->count, message->entities,
				message->unknown2C, message->data[0]);
		}
		for (place= 0; place<message->count; place++)
		{
			replication_entity_view_set_entity_state(view, message->entities[place], _replication_entity_view_state3);
		}
		break;
	}
	case _replication_entity_view_message_deletion:
	case _replication_entity_view_message_chain_deletion:
	{
		const struct replication_entity_view_entity *view_entity= replication_entity_view_entity_get(view, index);

		if (view_entity->state==_replication_entity_view_state_none || view_entity->entity_index!=entity_index ||
			view_entity->state!=_replication_entity_view_state3)
		{
			return;
		}
		manager= *(struct replication_entity_manager **)(view+REPLICATION_ENTITY_VIEW_MANAGER_OFFSET);
		if (message->type==_replication_entity_view_message_deletion)
		{
			if (manager->entities[index].flags&(_replication_entity_first_bit|_replication_entity_other_bit))
			{
				return;
			}
			replication_entity_view_set_entity_state(view, entity_index, _replication_entity_view_state_none);
			replication_entity_manager_delete_entity_now(
				*(struct replication_entity_manager **)(view+REPLICATION_ENTITY_VIEW_MANAGER_OFFSET), entity_index);
		}
		else
		{
			long entities[MAXIMUM_REPLICATION_ENTITY_CHAIN];
			long count;

			if (!(manager->entities[index].flags&_replication_entity_first_bit))
			{
				return;
			}
			count= replication_entity_manager_get_chain(entity_index, entities, manager);
			if (message->count!=count || memcmp(message->entities, entities, (unsigned long)count*sizeof(long))!=0)
			{
				return;
			}
			for (place= 0; place<message->count; place++)
			{
				replication_entity_view_set_entity_state(view, message->entities[place], _replication_entity_view_state_none);
			}
			replication_entity_manager_delete_entities_now(
				*(struct replication_entity_manager **)(view+REPLICATION_ENTITY_VIEW_MANAGER_OFFSET), message->count,
				message->entities);
		}
		break;
	}
	case _replication_entity_view_message_update:
		manager= *(struct replication_entity_manager **)(view+REPLICATION_ENTITY_VIEW_MANAGER_OFFSET);
		manager->client->functions->updated(manager->client, entity_index, message->unknown2C[0], message->unknown3C,
			message->data[0]);
		break;
	}
}

/* 0x000991d0: the thing that a view has in the place of a number makes way
for the thing of that number, which came in a packet; whether it could. With
no thing there, it is so. The counts of the place of the two numbers are
taken from each other, in sixteen: the new one has to be the same or up to
three after the old, or four or more before it (which is to be far after it,
all the way round); if not, nothing is done. What the things are of is told,
by its twelfth function; and the old thing is in no state in the view and
done with now: alone, or with the ones that were made at once with it,
which are found from it if it is the first and from the first if not */
boolean __attribute__((stdcall)) replication_entity_view_make_way(
	long entity_index,
	byte *view,
	long unknown,
	void *unknown2)
{
	const struct replication_entity_view_entity *view_entity=
		replication_entity_view_entity_get(view, entity_index&REPLICATION_ENTITY_INDEX_MASK);
	struct replication_entity_manager *manager;
	long entities[MAXIMUM_REPLICATION_ENTITY_CHAIN];
	long old_entity_index;
	long difference;
	long count;
	long place;
	byte flags;

	if (view_entity->state==_replication_entity_view_state_none)
	{
		return TRUE;
	}
	old_entity_index= view_entity->entity_index;
	difference= (long)((unsigned long)entity_index>>REPLICATION_ENTITY_SALT_SHIFT)-
		(long)((unsigned long)old_entity_index>>REPLICATION_ENTITY_SALT_SHIFT)+REPLICATION_ENTITY_SALT_COUNT;
	if (difference>REPLICATION_ENTITY_SALT_COUNT/2)
	{
		difference-= REPLICATION_ENTITY_SALT_COUNT;
	}
	if (!((difference>=0 && difference<4) || difference<=-4))
	{
		return FALSE;
	}
	manager= *(struct replication_entity_manager **)(view+REPLICATION_ENTITY_VIEW_MANAGER_OFFSET);
	manager->client->functions->replaced(manager->client, old_entity_index, entity_index, unknown, unknown2);
	manager= *(struct replication_entity_manager **)(view+REPLICATION_ENTITY_VIEW_MANAGER_OFFSET);
	flags= manager->entities[old_entity_index&REPLICATION_ENTITY_INDEX_MASK].flags;
	if (!(flags&_replication_entity_first_bit) && !(flags&_replication_entity_other_bit))
	{
		replication_entity_view_set_entity_state(view, old_entity_index, _replication_entity_view_state_none);
		replication_entity_manager_delete_entity_now(
			*(struct replication_entity_manager **)(view+REPLICATION_ENTITY_VIEW_MANAGER_OFFSET), old_entity_index);

		return TRUE;
	}
	if (flags&_replication_entity_first_bit)
	{
		count= replication_entity_manager_get_chain(old_entity_index, entities, manager);
	}
	else
	{
		count= replication_entity_manager_find_chain(manager, entities, old_entity_index);
	}
	for (place= 0; place<count; place++)
	{
		replication_entity_view_set_entity_state(view, entities[place], _replication_entity_view_state_none);
	}
	replication_entity_manager_delete_entities_now(
		*(struct replication_entity_manager **)(view+REPLICATION_ENTITY_VIEW_MANAGER_OFFSET), count, entities);

	return TRUE;
}

/* 0x000992d0: a view is told what became of a packet that it wrote in, by
its number: whether it came to the other console. With no such packet
nothing is done. It is the ninth function of a view.

Of each thing that the view said something of in it: if the packet did not
come, the view has those bits to say again, but not the ones that it said
again in a packet after this one; and if it had nothing to say of the thing,
which is in its third state and not being done with, it has one more to say
something of. If it came, the view counts one more. What was said of the
same thing before, that knew of this as the later one, knows of the one
after this instead.

Of each thing that the view said was made: if the packet came, the thing is
in its third state, and the view counts one more; if not, it is in its first
again, or in its third if it is being done with. Of each that it said was
done with: if it did not come, it is in its third state again; if it came,
it is in no state, the thing's count of what still has it has not the bit of
the view, and with no bit left the thing is done with: what the things are
of is told, and its place is not in use.

And what the view kept of the packet is taken back by the heap of the
network */
void __attribute__((stdcall)) replication_entity_view_packet_result(
	byte *view,
	long number,
	boolean delivered)
{
	struct replication_entity_view_packet **link= (struct replication_entity_view_packet **)(view+REPLICATION_ENTITY_VIEW_SENT_OFFSET);
	struct replication_entity_view_packet *packet;
	struct replication_entity_view_update *update;
	struct replication_entity_view_update *next_update;
	struct replication_entity_view_creation *creation;
	struct replication_entity_view_creation *next_creation;

	for (packet= *link; packet && packet->number!=number; packet= *link)
	{
		link= &packet->next;
	}
	if (!packet)
	{
		return;
	}
	for (update= packet->updates; update; update= next_update)
	{
		long entity_index= update->entity_index;
		long index= entity_index&REPLICATION_ENTITY_INDEX_MASK;
		const struct replication_entity_manager *manager=
			*(const struct replication_entity_manager **)(view+REPLICATION_ENTITY_VIEW_MANAGER_OFFSET);
		const struct replication_entity *entity= &manager->entities[index];
		struct replication_entity_view_packet *other;

		next_update= update->next;
		if (!delivered)
		{
			struct replication_entity_view_entity *view_entity= replication_entity_view_entity_get(view, index);
			unsigned long flags= update->flags;

			for (other= packet->next; other; other= other->next)
			{
				struct replication_entity_view_update *other_update;

				for (other_update= other->updates; other_update; other_update= other_update->next)
				{
					if (other_update->entity_index==entity_index)
					{
						flags&= ~other_update->flags;
					}
				}
			}
			if (flags!=0 && view_entity->pending==0 && view_entity->state==_replication_entity_view_state3 &&
				!((word)entity->unknown2&(1u<<(*(const long *)(view+REPLICATION_ENTITY_VIEW_INDEX_OFFSET)&31))))
			{
				(*(long *)(view+REPLICATION_ENTITY_VIEW_PENDING_COUNT_OFFSET))++;
			}
			view_entity->pending|= flags;
		}
		else
		{
			(*(long *)(view+REPLICATION_ENTITY_VIEW_UNKNOWN5030_OFFSET))++;
		}
		for (other= *(struct replication_entity_view_packet **)(view+REPLICATION_ENTITY_VIEW_SENT_OFFSET);
			other && other!=packet; other= other->next)
		{
			struct replication_entity_view_update *other_update;

			for (other_update= other->updates; other_update; other_update= other_update->next)
			{
				if (other_update->entity_index==entity_index && other_update->newer==update)
				{
					other_update->newer= update->newer;
					break;
				}
			}
			if (other_update)
			{
				break;
			}
		}
		network_heap_free(update);
	}
	for (creation= packet->creations; creation; creation= next_creation)
	{
		long entity_index= creation->entity_index;
		long index= entity_index&REPLICATION_ENTITY_INDEX_MASK;
		struct replication_entity_manager *manager=
			*(struct replication_entity_manager **)(view+REPLICATION_ENTITY_VIEW_MANAGER_OFFSET);
		struct replication_entity *entity= &manager->entities[index];
		word bit= (word)(1u<<(*(const long *)(view+REPLICATION_ENTITY_VIEW_INDEX_OFFSET)&31));

		next_creation= creation->next;
		if (creation->type==_replication_entity_view_message_creation)
		{
			if (delivered)
			{
				byte *same_view= replication_entity_view_set_entity_state(view, entity_index, _replication_entity_view_state3);

				(*(long *)(same_view+REPLICATION_ENTITY_VIEW_UNKNOWN5028_OFFSET))++;
			}
			else
			{
				replication_entity_view_set_entity_state(view, entity_index,
					((word)entity->unknown2&bit) ? _replication_entity_view_state3 : _replication_entity_view_state1);
			}
		}
		else if (!delivered)
		{
			replication_entity_view_set_entity_state(view, entity_index, _replication_entity_view_state3);
		}
		else
		{
			replication_entity_view_set_entity_state(view, entity_index, _replication_entity_view_state_none);
			entity->unknown2= (short)((word)entity->unknown2&~bit);
			manager= *(struct replication_entity_manager **)(view+REPLICATION_ENTITY_VIEW_MANAGER_OFFSET);
			entity= &manager->entities[index];
			if (entity->unknown2==0)
			{
				manager->client->functions->deleted(manager->client, entity_index);
				entity->flags&= ~_replication_entity_used_bit;
			}
			(*(long *)(view+REPLICATION_ENTITY_VIEW_UNKNOWN5038_OFFSET))++;
		}
		network_heap_free(creation);
	}
	*link= packet->next;
	network_heap_free(packet);
}
