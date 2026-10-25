/*
RASTERIZER_DEBUG.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The lines and triangles that are drawn to see how the game works. They are
sent straight to the console's graphics: the game writes the commands in
Direct3D's push buffer itself, as Direct3D's Begin and End do. The name of
the file is Halo 1's (rasterizer/xbox/rasterizer_xbox_debug.c); that these
functions are of it is a guess, and so are their names.
*/

#include "rasterizer_debug.h"

#include "shell/shell.h"
#include <string.h>

/* ---------- prototypes of functions of the console's libraries */

/* 0x003faae0: makes room in the push buffer and returns where to write;
the game asks for the size of the push buffer, and at least half of it */
uint32_t *__attribute__((stdcall)) D3D_MakeRequestedSpace(uint32_t size, uint32_t minimum);

/* 0x003f5730: how wide the lines are drawn */
void __attribute__((stdcall)) D3DDevice_SetRenderState_LineWidth(real width);

/* 0x003fa680 */
uint32_t __attribute__((stdcall)) D3D_SetFence(uint32_t flags);

/* 0x00400fa0: sends the console the states that Direct3D has been told
and has not sent yet. The name is a guess */
void D3D_LazySetState(void);

/* ---------- constants */

enum
{
	/* a command of the console's graphics (NV2A) with one value: the
	method 0x17FC, which begins (with the kind of primitive) or ends (with
	0) what is drawn */
	NV2A_SET_BEGIN_END= 0x000417FC,

	NV2A_PRIMITIVE_END= 0,
	NV2A_PRIMITIVE_LINES= 2,
	NV2A_PRIMITIVE_LINE_STRIP= 4,
	NV2A_PRIMITIVE_TRIANGLES= 5,
	NV2A_PRIMITIVE_QUADS= 8,

	/* of d3d_device->state_flags: between a begin and an end, and what
	makes the end set a fence */
	_d3d_state_flag_begun= 0x800,
	_d3d_state_flag_fence= 0x1000
};

/* ---------- globals */

/* 0x004b6294: between a rasterizer_debug_immediate_begin and its end */
boolean rasterizer_debug_immediate_active;

/* 0x004b6298: what is being drawn, or none */
long rasterizer_debug_primitive_type;

/* 0x00408648: the size of Direct3D's push buffer. It is Direct3D's; the
name is a guess */
uint32_t d3d_push_buffer_size;

/* ---------- private code */

static inline __attribute__((always_inline)) void push_begin_end(
	struct d3d_device *device,
	uint32_t primitive)
{
	uint32_t *push= device->push;

	if (push>=device->push_limit)
	{
		push= D3D_MakeRequestedSpace(d3d_push_buffer_size, d3d_push_buffer_size>>1);
	}
	push[0]= NV2A_SET_BEGIN_END;
	push[1]= primitive;
	device->push= push+2;
}

/* ---------- public code */

/* 0x0001ee50: what is being drawn is ended. The functions that begin are
0x0001e930 and 0x0001eb00 */
void rasterizer_debug_immediate_end(
	void)
{
	rasterizer_debug_primitive_end();
	rasterizer_debug_immediate_active= FALSE;
}

/* 0x0001ee60: begins drawing primitives of a kind, after ending those of
another; the lines are 1 wide. The type is 1 to 4: the game has nothing for
another */
void rasterizer_debug_primitive_begin(
	long type)
{
	struct d3d_device *device;
	uint32_t primitive;

	if (rasterizer_debug_primitive_type==type)
	{
		return;
	}
	rasterizer_debug_primitive_end();
	D3DDevice_SetRenderState_LineWidth(1.0f);
	switch (type)
	{
	case _rasterizer_debug_primitive_lines:
		primitive= NV2A_PRIMITIVE_LINES;
		break;
	case _rasterizer_debug_primitive_line_strip:
		primitive= NV2A_PRIMITIVE_LINE_STRIP;
		break;
	case _rasterizer_debug_primitive_triangles:
		primitive= NV2A_PRIMITIVE_TRIANGLES;
		break;
	case _rasterizer_debug_primitive_quads:
	default:
		primitive= NV2A_PRIMITIVE_QUADS;
		break;
	}
	device= d3d_device;
	D3D_LazySetState();
	push_begin_end(device, primitive);
	device->state_flags|= _d3d_state_flag_begun;
	rasterizer_debug_primitive_type= type;
}

/* 0x0001ef70: ends what rasterizer_debug_primitive_begin began, if it
began anything, as Direct3D's End does; the lines are 1 wide again */
void rasterizer_debug_primitive_end(
	void)
{
	if (rasterizer_debug_primitive_type!=_rasterizer_debug_primitive_none)
	{
		real line_width= 1.0f;
		struct d3d_device *device= d3d_device;
		uint32_t flags;

		push_begin_end(device, NV2A_PRIMITIVE_END);
		flags= device->state_flags;
		device->state_flags= flags&~(_d3d_state_flag_begun|_d3d_state_flag_fence);
		if (flags&_d3d_state_flag_fence)
		{
			D3D_SetFence(1);
		}
		D3DDevice_SetRenderState_LineWidth(line_width);
		rasterizer_debug_primitive_type= _rasterizer_debug_primitive_none;
	}
}
/* 0x004b6280, 0x004b6288: how many of the 0x400 entries of 0x70 bytes at
unknown_004b6288 are taken */
long unknown_004b6280;
byte *unknown_004b6288;

/* 0x004b6291: a byte that is cleared when there is no entry left */
byte unknown_004b6291;

/* 0x0001e2d0: the next entry of unknown_004b6288, with its first long and
its byte at 0xC zero; NULL when the 0x400 are taken */
void *unknown_004b6288_new(
	void)
{
	byte *entry;

	if (unknown_004b6280<0x400)
	{
		entry= unknown_004b6288 + unknown_004b6280*0x70;
		unknown_004b6280++;
		*(long *)entry= 0;
		entry[0xC]= 0;
		return entry;
	}
	if (unknown_004b6291!=0)
	{
		unknown_004b6291= 0;
	}

	return NULL;
}

/* 0x0001e8f0: whether two pairs of shorts are the same */
boolean __attribute__((stdcall)) short_pairs_equal(
	const short *first,
	const short *second)
{
	return first[1]==second[1] && first[0]==second[0];
}

/* 0x00485618 and 0x00485624: where the camera is and where it looks */
real unknown_00485618[3];
real unknown_00485624[3];

/* 0x0001e4e0: one more transparent plane (0x70 bytes, 0x400 at most, of
unknown_004b6288): of kind 3, its distance in front of the camera (negated)
at 0x04, a procedure at 0x20, so many bytes of data at 0x24 and its point
at 0x64; FALSE when there is no room (and unknown_004b6291 is cleared) */
boolean __attribute__((stdcall)) rasterizer_debug_add_plane(
	unsigned long size,
	const real *point,
	long procedure,
	const byte *data)
{
	byte *plane;
	real x;
	real y;
	real z;

	if (unknown_004b6280>=0x400)
	{
		if (unknown_004b6291)
		{
			unknown_004b6291= FALSE;
		}
		return FALSE;
	}
	plane= unknown_004b6288+unknown_004b6280*0x70;
	*(long *)plane= 0;
	plane[0x0C]= 0;
	x= point[0]-unknown_00485618[0];
	y= point[1]-unknown_00485618[1];
	z= point[2]-unknown_00485618[2];
	unknown_004b6280++;
	if (size)
	{
		memcpy(plane+0x24, data, size);
	}
	*(long *)plane= 3;
	*(long *)(plane+0x08)= 0;
	*(long *)(plane+0x20)= procedure;
	*(real *)(plane+0x04)= 0.0f-(unknown_00485624[2]*z + unknown_00485624[1]*y + unknown_00485624[0]*x);
	((unsigned long *)(plane+0x64))[0]= ((const unsigned long *)point)[0];
	((unsigned long *)(plane+0x64))[1]= ((const unsigned long *)point)[1];
	((unsigned long *)(plane+0x64))[2]= ((const unsigned long *)point)[2];
	return TRUE;
}
