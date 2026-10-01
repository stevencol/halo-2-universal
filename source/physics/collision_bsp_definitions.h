/*
COLLISION_BSP_DEFINITIONS.H

The collision bsp has the organization it has in Halo 1; the indices are 16
bits here, where they are 32 bits there.
*/

#ifndef __COLLISION_BSP_DEFINITIONS_H
#define __COLLISION_BSP_DEFINITIONS_H

#include "math/real_math.h"
#include "tag_files/tag_groups.h"

#include <stddef.h>

/* ---------- constants */

enum
{
	MAXIMUM_EDGES_PER_COLLISION_SURFACE= 8,

	/* in a plane designator: the plane faces the other way */
	PLANE_DESIGNATOR_NEGATED_FLAG= 0x8000
};

/* ---------- structures */

struct collision_surface
{
	uint16_t plane_designator;
	uint16_t first_edge_index;
	uint8_t flags;
	uint8_t breakable_surface_index;
	int16_t material_index;
};

/* the surface on the left of an edge goes from the first vertex to the
second and continues with the first edge index; the surface on the right
goes the other way and continues with the second */
struct collision_edge
{
	uint16_t vertex_indices[2];
	uint16_t edge_indices[2];
	uint16_t surface_indices[2];
};

struct collision_vertex
{
	real_point3d point;
	uint16_t first_edge_index;
	uint16_t pad;
};

struct collision_bsp
{
	struct tag_block bsp3d_nodes; /* 8 bytes each */
	struct tag_block planes; /* real_plane3d */
	struct tag_block leaves; /* 4 bytes each */
	struct tag_block bsp2d_references; /* 4 bytes each */
	struct tag_block bsp2d_nodes; /* 16 bytes each */
	struct tag_block surfaces; /* collision_surface */
	struct tag_block edges; /* collision_edge */
	struct tag_block vertices; /* collision_vertex */
};

typedef char verify_collision_surface_size[
	sizeof(struct collision_surface) == 0x8 ? 1 : -1];
typedef char verify_collision_edge_size[
	sizeof(struct collision_edge) == 0xC ? 1 : -1];
typedef char verify_collision_vertex_size[
	sizeof(struct collision_vertex) == 0x10 ? 1 : -1];
typedef char verify_collision_bsp_size[
	sizeof(struct collision_bsp) == 0x40 ? 1 : -1];

#endif
