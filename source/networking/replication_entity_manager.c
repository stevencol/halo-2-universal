/*
REPLICATION_ENTITY_MANAGER.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What keeps the things that the consoles of a game have in common: 1024 of
them, each with a number, and up to fifteen views of them, one for each of
the other consoles. The name of the file is a guess, and so are the names of
its functions.
*/

#include "replication_entity_manager.h"

#include "networking/replication_entity_view.h"

#include <string.h>

/* ---------- code */

/* 0x000893c0: what keeps the things is set up: with nothing that they are
of, no views, no things, and the first place as the next. The game has
replication_entity_manager_reset written here, which with no views does no
more than that */
void replication_entity_manager_new(
	struct replication_entity_manager *manager)
{
	manager->client= NULL;
	manager->view_mask= 0;
	memset(manager->views, 0, sizeof(manager->views));
	replication_entity_manager_reset(manager);
}

/* 0x00089430: there are no things any more: each view starts over
(replication_entity_view_reset), and the first place is the next */
void __attribute__((stdcall)) replication_entity_manager_reset(
	struct replication_entity_manager *manager)
{
	long index;

	for (index= 0; index<MAXIMUM_REPLICATION_ENTITY_VIEWS; index++)
	{
		if (manager->views[index])
		{
			replication_entity_view_reset(manager->views[index]);
		}
	}
	memset(manager->entities, 0, sizeof(manager->entities));
	manager->next_index= 0;
}

/* 0x00089470: so many things are made at once, four at the most, and their
numbers are given; FALSE, with nothing given, if there is no place for one
of them (and the places that were found are left as in use). The first has
the bit that says so, and the others theirs; each has the number of the one
after it, and the last has NONE. They are made from the last to the first */
boolean __attribute__((stdcall)) replication_entity_manager_create_entities(
	struct replication_entity_manager *manager,
	long count,
	long *entities)
{
	long indices[MAXIMUM_REPLICATION_ENTITY_CHAIN];
	long index;

	for (index= 0; index<count; index++)
	{
		indices[index]= replication_entity_manager_allocate_index(manager);
		if (indices[index]==NONE)
		{
			return FALSE;
		}
	}
	for (index= count-1; index>=0; index--)
	{
		struct replication_entity *entity;

		entities[index]= replication_entity_manager_create_entity(indices[index], manager);
		entity= &manager->entities[entities[index]&REPLICATION_ENTITY_INDEX_MASK];
		entity->flags|= index==0 ? _replication_entity_first_bit : _replication_entity_other_bit;
		entity->next= index+1<count ? entities[index+1] : NONE;
	}

	return TRUE;
}

/* 0x00089500: a thing is to be done with. It has the bit that says so;
what the things are of is told; and so is each view that the mask has
(replication_entity_view_delete_entity). If nothing has it still, it is done with now: what the
things are of is told that too, and its place is not in use */
void __attribute__((stdcall)) replication_entity_manager_delete_entity(
	struct replication_entity_manager *manager,
	long entity_index)
{
	struct replication_entity *entity= &manager->entities[entity_index&REPLICATION_ENTITY_INDEX_MASK];

	entity->flags|= _replication_entity_deleting_bit;
	manager->client->functions->deleting(manager->client, entity_index);
	if (manager->view_mask!=0)
	{
		long index;

		for (index= 0; index<MAXIMUM_REPLICATION_ENTITY_VIEWS; index++)
		{
			if (manager->view_mask&(1u<<index))
			{
				replication_entity_view_delete_entity(manager->views[index], entity_index);
			}
		}
	}
	if (entity->unknown2==0)
	{
		manager->client->functions->deleted(manager->client, entity_index);
		entity->flags&= ~_replication_entity_used_bit;
	}
}

/* 0x00089660: a thing is done with now, whatever has it still, and the
views are not told */
void replication_entity_manager_delete_entity_now(
	struct replication_entity_manager *manager,
	long entity_index)
{
	struct replication_entity *entity= &manager->entities[entity_index&REPLICATION_ENTITY_INDEX_MASK];

	entity->flags|= _replication_entity_deleting_bit;
	entity->unknown2= 0;
	manager->client->functions->deleting(manager->client, entity_index);
	manager->client->functions->deleted(manager->client, entity_index);
	entity->flags&= ~_replication_entity_used_bit;
}

/* 0x00089c40: a place for a thing: the first that is not in use from the
next one on, or else from the start up to it; NONE if they all are. It is in
use, with no other bit, and the next place is the one after it */
long replication_entity_manager_allocate_index(
	struct replication_entity_manager *manager)
{
	long next= manager->next_index;
	long found= NONE;
	long index;

	for (index= next; index<MAXIMUM_REPLICATION_ENTITIES; index++)
	{
		if (!(manager->entities[index].flags&_replication_entity_used_bit))
		{
			found= index;
			break;
		}
	}
	if (found==NONE)
	{
		for (index= 0; index<next; index++)
		{
			if (!(manager->entities[index].flags&_replication_entity_used_bit))
			{
				found= index;
				break;
			}
		}
	}
	if (found==NONE)
	{
		return NONE;
	}
	manager->entities[found].flags= _replication_entity_used_bit;
	manager->next_index= (found+1)%MAXIMUM_REPLICATION_ENTITIES;

	return found;
}

/* 0x00089cc0: the thing of a place is made: it has the bit that says so,
its place has been used once more (of sixteen), and it has no thing after
it. Each view that there is is told (unknown_00099690). Its number */
long replication_entity_manager_create_entity(
	long index,
	struct replication_entity_manager *manager)
{
	struct replication_entity *entity= &manager->entities[index];
	long entity_index;
	long view_index;

	entity->flags|= _replication_entity_created_bit;
	entity->next= NONE;
	entity->salt= (byte)((entity->salt+1)%REPLICATION_ENTITY_SALT_COUNT);
	entity_index= (long)((unsigned long)entity->salt<<REPLICATION_ENTITY_SALT_SHIFT|(unsigned long)index);
	for (view_index= 0; view_index<MAXIMUM_REPLICATION_ENTITY_VIEWS; view_index++)
	{
		if (manager->views[view_index])
		{
			unknown_00099690(manager->views[view_index], entity_index, 1);
		}
	}

	return entity_index;
}

/* 0x00089d20: the numbers of a thing and of the ones after it; how many */
long replication_entity_manager_get_chain(
	long entity_index,
	long *entities,
	const struct replication_entity_manager *manager)
{
	long count= 0;

	while (entity_index!=NONE)
	{
		entities[count++]= entity_index;
		entity_index= manager->entities[entity_index&REPLICATION_ENTITY_INDEX_MASK].next;
	}

	return count;
}

/* 0x00089d50: the things that were made at once with one: their numbers,
from the first, and how many; nothing if none of the first ones that are in
use has it after it */
long __attribute__((stdcall)) replication_entity_manager_find_chain(
	const struct replication_entity_manager *manager,
	long *entities,
	long entity_index)
{
	long index;

	for (index= 0; index<MAXIMUM_REPLICATION_ENTITIES; index++)
	{
		const struct replication_entity *entity= &manager->entities[index];

		if ((entity->flags&_replication_entity_used_bit) && (entity->flags&_replication_entity_first_bit))
		{
			long count= replication_entity_manager_get_chain(
				(long)((unsigned long)entity->salt<<REPLICATION_ENTITY_SALT_SHIFT|(unsigned long)index), entities, manager);
			long place;

			for (place= 0; place<count; place++)
			{
				if (entities[place]==entity_index)
				{
					return count;
				}
			}
		}
	}

	return 0;
}

/* 0x00089570: a thing and the ones after it are to be done with, as with
replication_entity_manager_delete_entity for each, from the last: each has
the bit and what the things are of is told; each view that the mask has is
told of each; and if nothing has the first still, they are all done with */
void replication_entity_manager_delete_chain(
	long entity_index,
	struct replication_entity_manager *manager)
{
	const struct replication_entity *first= &manager->entities[entity_index&REPLICATION_ENTITY_INDEX_MASK];
	long entities[MAXIMUM_REPLICATION_ENTITY_CHAIN];
	long count= replication_entity_manager_get_chain(entity_index, entities, manager);
	long index;

	for (index= count-1; index>=0; index--)
	{
		manager->entities[entities[index]&REPLICATION_ENTITY_INDEX_MASK].flags|= _replication_entity_deleting_bit;
		manager->client->functions->deleting(manager->client, entities[index]);
	}
	if (manager->view_mask!=0)
	{
		long view_index;

		for (view_index= 0; view_index<MAXIMUM_REPLICATION_ENTITY_VIEWS; view_index++)
		{
			if (manager->view_mask&(1u<<view_index))
			{
				for (index= count-1; index>=0; index--)
				{
					replication_entity_view_delete_entity(manager->views[view_index], entities[index]);
				}
			}
		}
	}
	if (first->unknown2==0)
	{
		for (index= count-1; index>=0; index--)
		{
			struct replication_entity *entity= &manager->entities[entities[index]&REPLICATION_ENTITY_INDEX_MASK];

			manager->client->functions->deleted(manager->client, entities[index]);
			entity->flags&= ~_replication_entity_used_bit;
		}
	}
}

/* 0x00089690: so many things are done with now, from the last, as with
replication_entity_manager_delete_entity_now; first none of them is the
first or another of the ones made at once, nor has one after it */
void __attribute__((stdcall)) replication_entity_manager_delete_entities_now(
	struct replication_entity_manager *manager,
	long count,
	const long *entities)
{
	long index;

	for (index= count-1; index>=0; index--)
	{
		struct replication_entity *entity= &manager->entities[entities[index]&REPLICATION_ENTITY_INDEX_MASK];

		entity->flags&= ~(_replication_entity_first_bit|_replication_entity_other_bit);
		entity->next= NONE;
	}
	for (index= count-1; index>=0; index--)
	{
		replication_entity_manager_delete_entity_now(manager, entities[index]);
	}
}

/* 0x000898c0: a view is one of the views, in a place, and the mask has it.
It is told of each thing that is in use and made and not being done with
(unknown_00099690), and given what the things are of has of it, by its
sixteenth function, if it has something (replication_entity_view_mark_entity) */
void __attribute__((stdcall)) replication_entity_manager_add_view(
	byte *view,
	struct replication_entity_manager *manager,
	long view_index)
{
	long index;

	manager->views[view_index]= view;
	manager->view_mask|= 1u<<view_index;
	for (index= 0; index<MAXIMUM_REPLICATION_ENTITIES; index++)
	{
		const struct replication_entity *entity= &manager->entities[index];

		if ((entity->flags&_replication_entity_used_bit) && (entity->flags&_replication_entity_created_bit) &&
			!(entity->flags&_replication_entity_deleting_bit))
		{
			long entity_index= (long)((unsigned long)entity->salt<<REPLICATION_ENTITY_SALT_SHIFT|(unsigned long)index);
			long unknown;

			unknown_00099690(manager->views[view_index], entity_index, 1);
			unknown= manager->client->functions->unknown3C(manager->client, entity_index);
			if (unknown!=0)
			{
				replication_entity_view_mark_entity(entity_index, manager->views[view_index], unknown);
			}
		}
	}
}

/* 0x00089950: so many things that another console made are here too, with
the numbers that they have there: each is in use, with no other bit than the
one of the first or of the others, the count of its place that its number
says, nothing that has it, and the number of the one after it. Then what the
things are of is told of each, by its third function, with the three things
that came with it and 2 */
void __attribute__((stdcall)) replication_entity_manager_create_entities_remote(
	const long *unknown4,
	struct replication_entity_manager *manager,
	long count,
	const long *entities,
	const long *unknown8,
	byte *unknown10)
{
	long index;

	for (index= 0; index<count; index++)
	{
		struct replication_entity *entity= &manager->entities[entities[index]&REPLICATION_ENTITY_INDEX_MASK];

		entity->flags= _replication_entity_used_bit;
		entity->salt= (byte)((unsigned long)entities[index]>>REPLICATION_ENTITY_SALT_SHIFT);
		entity->unknown2= 0;
		entity= &manager->entities[entities[index]&REPLICATION_ENTITY_INDEX_MASK];
		entity->flags|= index==0 ? _replication_entity_first_bit : _replication_entity_other_bit;
		entity->next= index+1<count ? entities[index+1] : NONE;
	}
	for (index= 0; index<count; index++)
	{
		manager->client->functions->created(manager->client, entities[index], unknown4[index], unknown8[index], 2,
			unknown10+index*REPLICATION_ENTITY_CREATION_DATA_SIZE);
	}
}

/* 0x00089710: every view has more to say of a thing: the bits that are
given. A view in which the thing is in its third state, and that had nothing
to say of it, has one more thing to say something of; but not if the thing's
count of what still has it has the bit of the view (the processor shifts by
the low five bits of which view it is) */
void __attribute__((stdcall)) replication_entity_manager_mark_entity(
	struct replication_entity_manager *manager,
	long entity_index,
	unsigned long flags)
{
	long view_index;

	for (view_index= 0; view_index<MAXIMUM_REPLICATION_ENTITY_VIEWS; view_index++)
	{
		byte *view= manager->views[view_index];

		if (view)
		{
			long index= entity_index&REPLICATION_ENTITY_INDEX_MASK;
			struct replication_entity_view_entity *entity=
				(struct replication_entity_view_entity *)(view+REPLICATION_ENTITY_VIEW_ENTITIES_OFFSET)+index;

			if (entity->state==_replication_entity_view_state3)
			{
				const struct replication_entity_manager *view_manager=
					*(const struct replication_entity_manager **)(view+REPLICATION_ENTITY_VIEW_MANAGER_OFFSET);

				if (!((1u<<(*(const long *)(view+REPLICATION_ENTITY_VIEW_INDEX_OFFSET)&31))&
					(word)view_manager->entities[index].unknown2) && entity->pending==0)
				{
					(*(long *)(view+REPLICATION_ENTITY_VIEW_PENDING_COUNT_OFFSET))++;
				}
			}
			entity->pending|= flags;
		}
	}
}

/* 0x00089a20: the things that another console made are this one's now.
Each thing in use that is not made is: it has the bit, and what the things
are of is told, by its fourteenth function; if that says no, the thing is to
be done with. The ones that were made at once go together, from the first,
and with the first no they are all to be done with; one that is another of
them and not the first is left for its first. Then every place in use has
been used once more, and so says the number of the thing after each. And
what the things are of is told that it is done, by its fifteenth */
void replication_entity_manager_adopt_entities(
	struct replication_entity_manager *manager)
{
	long index;

	for (index= 0; index<MAXIMUM_REPLICATION_ENTITIES; index++)
	{
		struct replication_entity *entity= &manager->entities[index];

		if ((entity->flags&_replication_entity_used_bit) && !(entity->flags&_replication_entity_created_bit))
		{
			long entity_index= (long)((unsigned long)entity->salt<<REPLICATION_ENTITY_SALT_SHIFT|(unsigned long)index);

			if (entity->flags&_replication_entity_first_bit)
			{
				long entities[MAXIMUM_REPLICATION_ENTITY_CHAIN];
				long count= replication_entity_manager_get_chain(entity_index, entities, manager);
				long place;

				for (place= 0; place<count; place++)
				{
					manager->entities[entities[place]&REPLICATION_ENTITY_INDEX_MASK].flags|= _replication_entity_created_bit;
					if (!manager->client->functions->adopt(manager->client, entities[place]))
					{
						replication_entity_manager_delete_chain(entity_index, manager);
						break;
					}
				}
			}
			else if (!(entity->flags&_replication_entity_other_bit))
			{
				entity->flags|= _replication_entity_created_bit;
				if (!manager->client->functions->adopt(manager->client, entity_index))
				{
					replication_entity_manager_delete_entity(manager, entity_index);
				}
			}
		}
	}
	for (index= 0; index<MAXIMUM_REPLICATION_ENTITIES; index++)
	{
		struct replication_entity *entity= &manager->entities[index];

		if (entity->flags&_replication_entity_used_bit)
		{
			entity->salt= (byte)((entity->salt+1)%REPLICATION_ENTITY_SALT_COUNT);
			if (entity->next!=NONE)
			{
				unsigned long next= (unsigned long)entity->next;

				entity->next= (long)((((next>>REPLICATION_ENTITY_SALT_SHIFT)+1)%REPLICATION_ENTITY_SALT_COUNT)<<
					REPLICATION_ENTITY_SALT_SHIFT|(next&REPLICATION_ENTITY_INDEX_MASK));
			}
		}
	}
	manager->client->functions->adopted(manager->client);
}
