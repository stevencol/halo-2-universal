/*
BSP3D.H

The tree of planes that divides space into the leaves of a collision bsp.
As in Halo 1, with nodes of 8 bytes where that game's are 12.
*/

#ifndef __BSP3D_H
#define __BSP3D_H

#include "math/real_math.h"
#include "tag_files/tag_groups.h"

/* ---------- constants */

enum
{
	/* in a child of a node: the child is a leaf, not a node */
	BSP3D_NODE_CHILD_LEAF_FLAG= 0x800000,
	BSP3D_NODE_CHILD_INDEX_MASK= 0x7FFFFF,

	/* in a plane designator: the plane faces the other way */
	BSP3D_PLANE_DESIGNATOR_NEGATED_FLAG= 0x8000,
	BSP3D_PLANE_DESIGNATOR_INDEX_MASK= 0x7FFF
};

/* ---------- structures */

struct bsp3d_node
{
	short plane_index;
	/* the child behind the plane, then the child in front of it: 24 bits
	each, the low byte first. All bits set is no child */
	unsigned char children[2][3];
};

struct bsp3d
{
	struct tag_block nodes; /* bsp3d_node */
	struct tag_block planes; /* real_plane3d */
};

typedef char verify_bsp3d_node_size[
	sizeof(struct bsp3d_node) == 0x8 ? 1 : -1];
typedef char verify_bsp3d_size[
	sizeof(struct bsp3d) == 0x10 ? 1 : -1];

/* ---------- prototypes/BSP3D.C */

void bsp3d_get_plane_from_designator(const struct bsp3d *bsp, long plane_designator, real_plane3d *result);
long bsp3d_test_point(const struct bsp3d *bsp, long node_index, const real_point3d *point);

#endif
