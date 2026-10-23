/*
USER_INTERFACE_LISTS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

Lists of the interface: things in a list that goes both ways, with the
list that has them; a tree whose first child is followed to its end; and
two small questions of a thing. What the things are (screens or the parts
of one, most likely) is not known, and the names are guesses.
*/

#include "cseries/cseries.h"
#include "math/real_math.h"

#include <string.h>

/* ---------- constants */

enum
{
	/* a list with its first at 0x14, whose things have a number at 0xC
	(NONE when out), something at 0x10, and the next and the one before at
	0x18 and 0x1C */
	USER_INTERFACE_LIST_FIRST_OFFSET= 0x14,
	USER_INTERFACE_LIST_ITEM_INDEX_OFFSET= 0xC,
	USER_INTERFACE_LIST_ITEM_UNKNOWN10_OFFSET= 0x10,
	USER_INTERFACE_LIST_ITEM_NEXT_OFFSET= 0x18,
	USER_INTERFACE_LIST_ITEM_PREVIOUS_OFFSET= 0x1C,

	/* of a thing of the tree: something at 4, and its first child at 0x10 */
	USER_INTERFACE_TREE_UNKNOWN4_OFFSET= 0x4,
	USER_INTERFACE_TREE_FIRST_CHILD_OFFSET= 0x10,

	/* a number of a thing that is one of six, from 0 to 5 */
	USER_INTERFACE_UNKNOWN4_MAXIMUM= 5,

	/* the flags of a thing at 0x42 */
	USER_INTERFACE_FLAGS42_OFFSET= 0x42
};

/* a thing of a list of the other kind: the one before, the next and the
list that has it, whose first is its own first field */
struct user_interface_link
{
	struct user_interface_link *previous;
	struct user_interface_link *next;
	struct user_interface_link **list;
};

/* ---------- code */

#define LINK_AT(item, offset) (*(byte **)((item)+(offset)))

/* 0x0022e37f: whether the number at 4 of something is from 0 to 5 */
boolean user_interface_unknown4_valid(
	const byte *thing)
{
	long value= *(const long *)(thing+4);

	return value>=0 && value<=USER_INTERFACE_UNKNOWN4_MAXIMUM;
}

/* 0x0022eb43: a thing goes out of a list, if it is in it: the ones next to
it are joined, the list starts at its next if it was the first, and its
links, what it has at 0x10 and its number are cleared */
void user_interface_list_remove(
	byte *item,
	byte *list)
{
	byte *other;
	byte *previous;
	byte *next;

	for (other= LINK_AT(list, USER_INTERFACE_LIST_FIRST_OFFSET); other; other= LINK_AT(other, USER_INTERFACE_LIST_ITEM_NEXT_OFFSET))
	{
		if (other==item)
		{
			break;
		}
	}
	if (!other)
	{
		return;
	}
	previous= LINK_AT(item, USER_INTERFACE_LIST_ITEM_PREVIOUS_OFFSET);
	if (previous)
	{
		LINK_AT(previous, USER_INTERFACE_LIST_ITEM_NEXT_OFFSET)= LINK_AT(item, USER_INTERFACE_LIST_ITEM_NEXT_OFFSET);
	}
	next= LINK_AT(item, USER_INTERFACE_LIST_ITEM_NEXT_OFFSET);
	if (next)
	{
		LINK_AT(next, USER_INTERFACE_LIST_ITEM_PREVIOUS_OFFSET)= LINK_AT(item, USER_INTERFACE_LIST_ITEM_PREVIOUS_OFFSET);
	}
	if (item==LINK_AT(list, USER_INTERFACE_LIST_FIRST_OFFSET))
	{
		LINK_AT(list, USER_INTERFACE_LIST_FIRST_OFFSET)= LINK_AT(item, USER_INTERFACE_LIST_ITEM_NEXT_OFFSET);
	}
	LINK_AT(item, USER_INTERFACE_LIST_ITEM_UNKNOWN10_OFFSET)= NULL;
	LINK_AT(item, USER_INTERFACE_LIST_ITEM_NEXT_OFFSET)= NULL;
	LINK_AT(item, USER_INTERFACE_LIST_ITEM_PREVIOUS_OFFSET)= NULL;
	*(long *)(item+USER_INTERFACE_LIST_ITEM_INDEX_OFFSET)= NONE;
}

/* 0x0022eeee: the last of the first children of a thing of a tree, going
down from its first child; with no child, the thing itself if it has
nothing at 4, and NULL if it has */
byte *user_interface_tree_deepest_first_child(
	byte *thing)
{
	byte *child= LINK_AT(thing, USER_INTERFACE_TREE_FIRST_CHILD_OFFSET);

	if (child)
	{
		while (LINK_AT(child, USER_INTERFACE_TREE_FIRST_CHILD_OFFSET))
		{
			child= LINK_AT(child, USER_INTERFACE_TREE_FIRST_CHILD_OFFSET);
		}

		return child;
	}

	return *(const long *)(thing+USER_INTERFACE_TREE_UNKNOWN4_OFFSET) ? NULL : thing;
}

/* 0x0022f0b7: a thing goes at the end of a list, and knows the list */
void user_interface_link_append(
	struct user_interface_link **list,
	struct user_interface_link *link)
{
	struct user_interface_link *last;

	link->list= list;
	if (!*list)
	{
		*list= link;
		return;
	}
	for (last= *list; last->next; last= last->next)
	{
	}
	last->next= link;
	link->previous= last;
}

/* 0x0022f0d2: a thing goes out of its list: the ones next to it are joined,
the list starts at its next if it was the first, and its links and its
list are cleared */
void user_interface_link_remove(
	struct user_interface_link *link,
	struct user_interface_link **list)
{
	struct user_interface_link *next= link->next;

	if (link->previous)
	{
		link->previous->next= link->next;
	}
	if (link->next)
	{
		link->next->previous= link->previous;
	}
	link->next= NULL;
	link->previous= NULL;
	link->list= NULL;
	if (*list==link)
	{
		*list= next;
	}
}

/* 0x0022f0ff: whether a thing has the first or the second of its flags of
0x42 */
boolean user_interface_flags42_unknown_0022f0ff(
	const byte *thing)
{
	byte flags= thing[USER_INTERFACE_FLAGS42_OFFSET];

	return ((flags>>1)&1) || (flags&1);
}

/* ---------- more of the things of the interface */

enum
{
	/* of a thing: two bytes that both have to be set, and a number that
	can be no more than that of 0x0054d5b8 */
	USER_INTERFACE_THING_UNKNOWN6D_OFFSET= 0x6D,
	USER_INTERFACE_THING_UNKNOWN6E_OFFSET= 0x6E,
	USER_INTERFACE_THING_UNKNOWN50_OFFSET= 0x50,
	/* of a thing: its place, as four shorts at 0x20, and three reals at
	0x58 that move it */
	USER_INTERFACE_THING_BOUNDS_OFFSET= 0x20,
	USER_INTERFACE_THING_OFFSET_OFFSET= 0x58
};

/* 0x0054d5b8: a number that the number of 0x50 of a thing can reach */
unsigned long unknown_0054d5b8;

/* 0x0022e3b4: whether a thing has its two bytes of 0x6D and 0x6E, and its
number of 0x50 is no more than that of 0x0054d5b8 */
boolean user_interface_thing_unknown_0022e3b4(
	const byte *thing)
{
	return thing[USER_INTERFACE_THING_UNKNOWN6D_OFFSET] && thing[USER_INTERFACE_THING_UNKNOWN6E_OFFSET] &&
		*(const unsigned long *)(thing+USER_INTERFACE_THING_UNKNOWN50_OFFSET)<=unknown_0054d5b8;
}

/* 0x0022e9aa: the third of the three reals of 0x58 of a thing */
real user_interface_thing_get_offset_z(
	const byte *thing)
{
	real offset[3];

	memcpy(offset, thing+USER_INTERFACE_THING_OFFSET_OFFSET, sizeof(offset));

	return offset[2];
}

/* 0x0022e9c6: the place of a thing, as four shorts, moved by the first two
reals of 0x58 (cut to whole numbers): the second goes to the first and the
third short, and the first to the second and the fourth */
void user_interface_thing_get_bounds(
	const byte *thing,
	short *bounds)
{
	real offset[3];
	short x;
	short y;

	memcpy(bounds, thing+USER_INTERFACE_THING_BOUNDS_OFFSET, 4*sizeof(short));
	memcpy(offset, thing+USER_INTERFACE_THING_OFFSET_OFFSET, sizeof(offset));
	y= (short)(long)offset[1];
	x= (short)(long)offset[0];
	bounds[1]+= x;
	bounds[3]+= x;
	bounds[0]+= y;
	bounds[2]+= y;
}
