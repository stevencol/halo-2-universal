/*
RASTERIZER.C

the part of the game that draws: the first of the table of parts, with six
functions. Worked out from halo2ship.exe; the name of the file is a guess:
when a map goes it empties the cache of the text that is drawn.
*/

#include "cseries/cseries.h"
#include "rasterizer/rasterizer_text.h"
#include "scenario/scenario.h"

/* ---------- globals */

/* 0x00485a80: a number that is nothing when a map goes */
long unknown_00485a80;

/* 0x00015b70, 0x0001e1c0, 0x00036ab0: what is done when the game becomes
active */
void unknown_00015b70(void);
void unknown_0001e1c0(void);
void unknown_00036ab0(void);

/* ---------- code */

/* 0x000226c0: when a map goes: the cache of the text is emptied
(rasterizer_text_cache_flush), and the number is nothing */
void rasterizer_dispose_from_old_map(
	void)
{
	rasterizer_text_cache_flush();
	unknown_00485a80= 0;
}

/* 0x000226a0: the fifth of its six functions, for when the game becomes
active: three others, one after the other */
void rasterizer_unknown_000226a0(
	void)
{
	unknown_00015b70();
	unknown_0001e1c0();
	unknown_00036ab0();
}
/* 0x000226d0: whether the lightmap of the map goes with its structure: it
has something (its count at 0x80), there is a structure, the thing that the
lightmap points to at 0x84 is not NONE at 0x1C and has at 4 what the
structure has at 8 */
boolean rasterizer_lightmap_matches_structure(
	void)
{
	byte *lightmap= global_structure_lightmap;
	byte *structure= global_structure_bsp;
	byte *other;

	if (lightmap==NULL || *(long *)(lightmap + 0x80)<=0 || structure==NULL)
	{
		return FALSE;
	}
	other= *(byte **)(lightmap + 0x84);

	return *(long *)(other + 0x1C)!=NONE && *(long *)(other + 4)==*(long *)(structure + 8);
}

/* 0x00485648: a rectangle of shorts: top, left, bottom, right */
short unknown_00485648[4];

/* 0x004ba04c: a count, of players or of views it may be */
long unknown_004ba04c;

/* 0x00022710: whether the rectangle is the whole screen (0, 0, 480, 640)
and the count is not more than 1 */
boolean rasterizer_unknown_00022710(
	void)
{
	return unknown_00485648[1]==0 && unknown_00485648[0]==0 && unknown_00485648[3]==640
		&& unknown_00485648[2]==480 && unknown_004ba04c<=1;
}
