/*
SCREEN_SPLIT.C

how the screen is split among the local players: the columns and rows, and
where each player goes. Worked out from halo2ship.exe; the names are guesses.
*/

#include "cseries/cseries.h"

/* ---------- code */

/* 0x0002f600: the columns and rows (grid[0] and grid[1]) for so many players:
the columns grow first, and up to one more than the rows unless the mode is 1 */
void screen_split_get_grid(
	long count,
	long mode,
	short *grid)
{
	long columns= 1;
	long rows= 1;
	boolean wide= mode==1;

	if (count>1)
	{
		do
		{
			if (wide ? columns<rows : columns<=rows)
			{
				columns++;
			}
			else
			{
				columns= 1;
				rows++;
			}
		}
		while (rows*columns<count);
	}

	grid[0]= (short)columns;
	grid[1]= (short)rows;
}

/* 0x0002f640: the size (in cells) and the cell of a player in the grid; when
there are fewer players than cells, the first one takes two */
void __attribute__((stdcall)) screen_split_get_position(
	long index,
	const short *grid,
	short *size,
	long count,
	long mode,
	short *position)
{
	long skipped= 0;
	long cell;

	size[1]= 1;
	size[0]= 1;
	if (count<grid[1]*grid[0])
	{
		if (mode==1)
		{
			if (index==0)
			{
				size[0]= 2;
			}
			else
			{
				skipped= 1;
			}
		}
		else if (index==0)
		{
			size[1]= 2;
		}
		else
		{
			skipped= index/grid[0];
		}
	}

	cell= skipped+index;
	position[0]= (short)(cell%grid[0]);
	position[1]= (short)(cell/grid[0]);
}
