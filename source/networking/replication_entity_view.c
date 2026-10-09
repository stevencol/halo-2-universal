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

/* ---------- code */

/* what a view has of the thing of a place */
static inline __attribute__((always_inline)) struct replication_entity_view_entity *replication_entity_view_entity_get(
	byte *view,
	long index)
{
	return (struct replication_entity_view_entity *)(view+REPLICATION_ENTITY_VIEW_ENTITIES_OFFSET)+index;
}

/* 0x00097fe0: a view starts over. Each thing that it has sent and waits to
hear of is taken back, by its ninth function, until there is none. Each
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
		(*(const struct replication_entity_view_functions **)view)->unsent(view,
			**(long **)(view+REPLICATION_ENTITY_VIEW_SENT_OFFSET), 0);
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
			view_entity->unknownA= NONE;
			view_entity->unknown10= 0;
		}
	}
	*(long *)(view+REPLICATION_ENTITY_VIEW_UNKNOWN502C_OFFSET)= 0;
	*(long *)(view+REPLICATION_ENTITY_VIEW_UNKNOWN5028_OFFSET)= 0;
	*(long *)(view+REPLICATION_ENTITY_VIEW_PENDING_COUNT_OFFSET)= 0;
	*(long *)(view+REPLICATION_ENTITY_VIEW_UNKNOWN5030_OFFSET)= 0;
	*(long *)(view+REPLICATION_ENTITY_VIEW_UNKNOWN5024_OFFSET)= NONE;
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
state (unknown_00099690), and the view counts one more of those; that is
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
			view= unknown_00099690(view, entity_index, _replication_entity_view_state_none);
			(*(long *)(view+REPLICATION_ENTITY_VIEW_UNKNOWN5038_OFFSET))++;

			return;
		}
		view= unknown_00099690(view, entity_index, _replication_entity_view_state3);
	}
	entity->unknown2= (short)((word)entity->unknown2|(word)(1u<<(*(const long *)(view+REPLICATION_ENTITY_VIEW_INDEX_OFFSET)&31)));
	if (view_entity->state==_replication_entity_view_state3)
	{
		if (view_entity->pending!=0)
		{
			(*(long *)(view+REPLICATION_ENTITY_VIEW_PENDING_COUNT_OFFSET))--;
		}
		(*(long *)(view+REPLICATION_ENTITY_VIEW_UNKNOWN503C_OFFSET))++;
	}
}
