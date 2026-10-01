/*
BSP3D.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.
*/

#include "bsp3d.h"

#include "cseries/cseries.h"

/* ---------- code */

/* 0x0014a280: the leaf that a point is in, from a node down; NONE if the
point is in no leaf */
long bsp3d_test_point(
	const struct bsp3d *bsp,
	long node_index,
	const real_point3d *point)
{
	const struct bsp3d_node *nodes= (const struct bsp3d_node *)bsp->nodes.address;
	const real_plane3d *planes= (const real_plane3d *)bsp->planes.address;
	long child;

	do
	{
		const struct bsp3d_node *node= &nodes[node_index&BSP3D_NODE_CHILD_INDEX_MASK];
		const real_plane3d *plane= &planes[node->plane_index];
		real distance= plane->n.k*point->z + plane->n.j*point->y + plane->n.i*point->x - plane->d;
		const unsigned char *bytes= node->children[distance>=0.0f ? 1 : 0];

		/* 24 bits, with their sign */
		child= (long)((unsigned long)bytes[0] | (unsigned long)bytes[1]<<8 |
			(unsigned long)(long)(signed char)bytes[2]<<16);
		node_index= child;
	}
	while (!(child&BSP3D_NODE_CHILD_LEAF_FLAG));

	return child==NONE ? NONE : child&BSP3D_NODE_CHILD_INDEX_MASK;
}
