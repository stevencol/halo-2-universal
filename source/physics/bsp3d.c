/*
BSP3D.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.
*/

#include "bsp3d.h"

#include "cseries/cseries.h"

/* ---------- code */

/* 0x0014a300: a plane of the tree, or the same plane facing the other way
if the designator has the bit for it. As in Halo 1, with a designator of 16
bits */
void bsp3d_get_plane_from_designator(
	const struct bsp3d *bsp,
	long plane_designator,
	real_plane3d *result)
{
	const real_plane3d *plane=
		&((const real_plane3d *)bsp->planes.address)[plane_designator&BSP3D_PLANE_DESIGNATOR_INDEX_MASK];

	if (plane_designator&BSP3D_PLANE_DESIGNATOR_NEGATED_FLAG)
	{
		result->n.i= 0.0f-plane->n.i;
		result->n.j= 0.0f-plane->n.j;
		result->n.k= 0.0f-plane->n.k;
		result->d= 0.0f-plane->d;
	}
	else
	{
		*result= *plane;
	}

	return;
}

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
