/*
RASTERIZER.C

the part of the game that draws: the first of the table of parts, with six
functions. Worked out from halo2ship.exe; the name of the file is a guess:
when a map goes it empties the cache of the text that is drawn.
*/

#include "cseries/cseries.h"
#include "rasterizer/rasterizer_text.h"

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
