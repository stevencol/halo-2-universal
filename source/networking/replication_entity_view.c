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
