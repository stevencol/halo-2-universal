/*
UNKNOWN_0051F40C.C

a table of entries of 0x20 bytes, with three lists of bits for them; the
second long of each entry keeps another entry's index in its bits 7 to 15.
Worked out from halo2ship.exe; what it is for is not known.
*/

#include "cseries/cseries.h"

#include <string.h>

/* ---------- globals */

/* 0x0051f408: whether the table is in use */
byte unknown_0051f408;

/* 0x0051f40c: the entries, eight longs each */
unsigned long unknown_0051f40c[0x3FE0/4];

/* 0x005233ec: how many entries there are */
long unknown_005233ec;

/* 0x005233f0: three lists of bits of the entries, sixteen longs each */
unsigned long unknown_005233f0[0xD4/4];

/* 0x005234c4: NONE when the table is not ready */
long unknown_005234c4;

/* ---------- code */

/* 0x00020b40: keeps an entry's index (or NONE) in another entry, if the table
is in use and both are entries of it */
void unknown_0051f40c_set_other(
	long index,
	long other)
{
	long last;
	unsigned long *entry;

	if (!unknown_0051f408 || unknown_005234c4==NONE)
	{
		return;
	}
	last= index<0 ? 0 : unknown_005233ec-1;
	if ((index<0 || index>last) && last!=index)
	{
		return;
	}
	last= other<0 ? 0 : unknown_005233ec-1;
	if ((other<0 || other>last) && last!=other && other!=NONE)
	{
		return;
	}

	entry= &unknown_0051f40c[8*index];
	if (other==NONE)
	{
		entry[1]|= 0xFF80;
	}
	else
	{
		entry[1]^= ((unsigned long)other<<7^entry[1])&0xFF80;
	}
}

/* 0x00020e50: an entry is taken out of the three lists of bits and cleared,
with NONE as its other entry */
void unknown_0051f40c_clear(
	long index)
{
	unsigned long *entry= &unknown_0051f40c[8*index];
	long list;

	for (list= 0; list<3; list++)
	{
		unknown_005233f0[16*list+(index>>5)]&= ~(1UL<<(index&31));
	}
	memset(entry, 0, 8*sizeof(unsigned long));
	entry[1]|= 0xFF80;
}

/* 0x0051f409: whether the display is of a kind other than 4 to 7 (of
unknown_00485898). 0x005234b0: the number of the display when the table
was last made ready */
byte unknown_0051f409;
long unknown_005234b0;

/* 0x00485aa0 (rasterizer_xbox.c) and 0x00485898 (shell.c) */
extern long rasterizer_display_initialized;
extern long unknown_00485898;

/* 0x000206a0: the table is made ready again (every entry loses the bit 0x02
of its second long) when the display has been made since it last was, or
with a kind of 4 to 7 and the same display, or for the first one; it is in
use only then, and it is never ready afterwards */
void unknown_0051f40c_update(
	void)
{
	long display= rasterizer_display_initialized;
	long kind= unknown_00485898;

	if (display>=0
		&& ((kind>=4 && kind<=7 && display==unknown_005234b0) || display>unknown_005234b0 || !unknown_005234b0))
	{
		long index;
		boolean special;

		for (index= 0; index<0x3FE0/0x20; index++)
		{
			unknown_0051f40c[index*8+1]&= ~2UL;
		}
		unknown_005234b0= display;
		unknown_0051f408= TRUE;
		special= kind>=4 && kind<=7;
		unknown_0051f409= !special;
		unknown_005234c4= NONE;
		return;
	}
	unknown_0051f408= FALSE;
	unknown_005234c4= NONE;
}

/* 0x0002e1e0: the entries (with the bit 4) of an object (its index in the
bits 6 to 17) are cleared */
void unknown_0051f40c_clear_object(
	long object_index)
{
	long absolute_index;
	long index;

	if (object_index==NONE)
	{
		return;
	}
	absolute_index= object_index&0xFFFF;
	for (index= 0; index<unknown_005233ec; index++)
	{
		unsigned long flags= unknown_0051f40c[8*index];

		if ((flags&0x10) && ((flags>>6)&0xFFF)==(unsigned long)absolute_index)
		{
			unknown_0051f40c_clear(index);
		}
	}
}
