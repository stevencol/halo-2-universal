/*
CLUSTER_PARTITIONS.C

which things of a kind are in each cluster of the structure bsp. Worked out
from halo2ship.exe; the name of the file is a guess, by the names that the
game gives to its tables of data.
*/

#include "cseries/cseries.h"

#include <string.h>

#include "math/real_math.h"
#include "objects/cluster_partitions.h"
#include "saved games/game_state.h"
#include "scenario/scenario.h"

/* ---------- prototypes */

/* 0x00296270: a table of data with a name, for so many things of 0xC bytes
and so many more (hs/object_lists.c makes its own with it too) */
struct data_array *__attribute__((stdcall)) reference_list_new(const char *name, long maximum_count, long unknown);

/* ---------- code */

/* 0x001cabc0: the 0x800 bytes take their memory in the state of the game
(game_state_malloc, which the game has written here), and the two tables are
made with their names; the first with so many bytes more for each thing */
void __attribute__((stdcall)) cluster_partition_initialize(
	struct cluster_partition *partition,
	const char *name,
	long unknown3)
{
	char buffer[0x100];

	partition->unknown0= game_state_malloc(CLUSTER_PARTITION_UNKNOWN0_SIZE);
	partition->unknown4= reference_list_new(csnzprintf(buffer, sizeof(buffer), "cluster %s", name),
		MAXIMUM_CLUSTER_PARTITION_REFERENCES, unknown3);
	partition->unknown8= reference_list_new(csnzprintf(buffer, sizeof(buffer), "%s cluster", name),
		MAXIMUM_CLUSTER_PARTITION_REFERENCES, 0);
}

/* 0x001cae40: a thing is in no cluster any more. It has a list of where it
is (of the second table: each with a cluster, a short at 4, and the next at
8); each of them is deleted, and in the list of that cluster (of the first
table: each with a thing at 4 and the next at 8; the first of each cluster is
in unknown0) the one of the thing is taken out and deleted */
void cluster_partition_remove(
	struct cluster_partition *partition,
	long index,
	long *first_reference_index)
{
	long reference_index= *first_reference_index;

	while (reference_index!=NONE)
	{
		struct data_array *references= partition->unknown8;
		const byte *reference= (const byte *)references->data+(reference_index&0xFFFF)*references->size;
		short cluster_index= *(const short *)(reference+4);
		long next_reference_index= *(const long *)(reference+8);
		struct data_array *cluster_references;
		long *link;

		datum_delete(references, reference_index);
		link= &partition->unknown0[cluster_index];
		cluster_references= partition->unknown4;
		while (*link!=NONE)
		{
			byte *cluster_reference= (byte *)cluster_references->data+(*link&0xFFFF)*cluster_references->size;

			if (*(long *)(cluster_reference+4)==index)
			{
				long next= *(long *)(cluster_reference+8);

				datum_delete(cluster_references, *link);
				*link= next;
				break;
			}
			link= (long *)(cluster_reference+8);
		}
		reference_index= next_reference_index;
	}
	*first_reference_index= NONE;
}

enum
{
	/* a thing is in so many clusters at most */
	MAXIMUM_CLUSTER_PARTITION_CLUSTERS= 0x100
};

/* 0x004e7414: how many times the clusters around a point have been asked
for, and 0x004e7411: set while they are (both of the function that finds
them, 0x0014a5b0, which is not decompiled) */
long unknown_004e7414;
boolean unknown_004e7411;

/* 0x0014a5b0: the clusters that a sphere whose centre is in a cluster
touches: how many there are, with so many of them at most in a list */
short __attribute__((stdcall)) unknown_0014a5b0(short cluster_index, const real_point3d *center, real radius,
	short maximum_count, short *cluster_indices);

/* 0x0014a550: the clusters of the structure bsp (it has how many at 0x9C)
whose bit is set: how many of them are in the list, which is so many at most;
and how many there are in all. In the game it is far from the others of this
file, so it may be of another */
short structure_bsp_clusters_from_bit_vector(
	const unsigned long *cluster_bits,
	short *total_count,
	short maximum_count,
	short *cluster_indices)
{
	short count= 0;
	long cluster_index;

	*total_count= 0;
	for (cluster_index= 0; cluster_index<*(const long *)((const byte *)global_structure_bsp+0x9C); cluster_index++)
	{
		if (BIT_VECTOR_TEST_FLAG(cluster_bits, cluster_index))
		{
			if (count<maximum_count)
			{
				cluster_indices[count++]= (short)cluster_index;
			}
			(*total_count)++;
		}
	}

	return count;
}

/* 0x001cac60: a thing is put in its clusters: those whose bits are given;
or without bits, those that a sphere touches (with a radius above zero), or
only the cluster of where the thing is (a short at 4 of it). For each cluster
the thing gets one more of its list of where it is, and the cluster one more
of its list of things, with so many bytes that are given after it. More than
0x100 clusters are not taken, and whether there were more is said */
void __attribute__((stdcall)) cluster_partition_add(
	const unsigned long *cluster_bits,
	struct cluster_partition *partition,
	long index,
	long *first_reference_index,
	const real_point3d *center,
	real radius,
	const byte *location,
	long extra_size,
	const void *extra,
	boolean *overflow)
{
	short cluster_indices[MAXIMUM_CLUSTER_PARTITION_CLUSTERS];
	short total_count;
	short count;
	short at;

	if (cluster_bits)
	{
		count= structure_bsp_clusters_from_bit_vector(cluster_bits, &total_count, MAXIMUM_CLUSTER_PARTITION_CLUSTERS,
			cluster_indices);
	}
	else
	{
		short cluster_index= *(const short *)(location+4);

		count= 0;
		if (cluster_index!=NONE)
		{
			if (radius>0.0f)
			{
				unknown_004e7414++;
				unknown_004e7411= TRUE;
				count= unknown_0014a5b0(cluster_index, center, radius, MAXIMUM_CLUSTER_PARTITION_CLUSTERS,
					cluster_indices);
				unknown_004e7411= FALSE;
			}
			else
			{
				count= 1;
				cluster_indices[0]= cluster_index;
			}
		}
		total_count= count;
		if (count>MAXIMUM_CLUSTER_PARTITION_CLUSTERS)
		{
			count= MAXIMUM_CLUSTER_PARTITION_CLUSTERS;
		}
	}
	*overflow= total_count>count;
	for (at= 0; at<count; at++)
	{
		short cluster_index= cluster_indices[at];
		struct data_array *references= partition->unknown8;
		long reference_index= datum_new(references);
		struct data_array *cluster_references;
		long cluster_reference_index;
		long *first;

		if (reference_index!=NONE)
		{
			byte *reference= (byte *)references->data+(reference_index&0xFFFF)*references->size;

			*(long *)(reference+4)= cluster_index;
			*(long *)(reference+8)= *first_reference_index;
			*first_reference_index= reference_index;
		}
		first= &partition->unknown0[cluster_index];
		cluster_references= partition->unknown4;
		cluster_reference_index= datum_new(cluster_references);
		if (cluster_reference_index!=NONE)
		{
			byte *cluster_reference= (byte *)cluster_references->data+
				(cluster_reference_index&0xFFFF)*cluster_references->size;

			*(long *)(cluster_reference+4)= index;
			*(long *)(cluster_reference+8)= *first;
			*first= cluster_reference_index;
			if (extra)
			{
				memcpy(cluster_reference+0xC, extra, (unsigned long)extra_size);
			}
		}
	}
}

/* 0x002962e0, of reference_lists.c */
void reference_list_set_extra(long *first_reference_index, long index, struct data_array *references, long extra_size,
	const void *extra);

/* 0x001cadf0: what is kept with a thing in the list of each of its clusters
is changed: so many bytes */
void cluster_partition_set_extra(
	long first_reference_index,
	struct cluster_partition *partition,
	const void *extra,
	long index,
	long extra_size)
{
	long reference_index;

	for (reference_index= first_reference_index; reference_index!=NONE; )
	{
		struct data_array *references= partition->unknown8;
		const byte *reference= (const byte *)references->data+(reference_index&0xFFFF)*references->size;

		reference_list_set_extra(&partition->unknown0[*(const short *)(reference+4)], index, partition->unknown4,
			extra_size, extra);
		reference_index= *(const long *)(reference+8);
	}
}
