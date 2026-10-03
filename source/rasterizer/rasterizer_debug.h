/*
RASTERIZER_DEBUG.H

The lines and triangles that are drawn to see how the game works, sent
straight to the console's graphics. The name of the file is Halo 1's; that
these functions are of it is a guess, and so are their names.
*/

#ifndef __RASTERIZER_DEBUG_H
#define __RASTERIZER_DEBUG_H

#include "cseries/cseries.h"

#include <stdint.h>

/* ---------- constants */

/* what rasterizer_debug_primitive_begin draws */
enum
{
	_rasterizer_debug_primitive_none= 0,
	_rasterizer_debug_primitive_lines,
	_rasterizer_debug_primitive_line_strip,
	_rasterizer_debug_primitive_triangles,
	_rasterizer_debug_primitive_quads
};

/* ---------- globals */

extern boolean rasterizer_debug_immediate_active;
extern long rasterizer_debug_primitive_type;
extern uint32_t d3d_push_buffer_size;

/* ---------- prototypes/RASTERIZER_DEBUG.C */

void rasterizer_debug_immediate_end(void);
void rasterizer_debug_primitive_begin(long type);
void rasterizer_debug_primitive_end(void);

#endif
