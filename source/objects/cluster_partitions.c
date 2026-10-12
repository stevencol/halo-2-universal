/*
CLUSTER_PARTITIONS.C

which things of a kind are in each cluster of the structure bsp. Worked out
from halo2ship.exe; the name of the file is a guess, by the names that the
game gives to its tables of data.
*/

#include "cseries/cseries.h"
#include "objects/cluster_partitions.h"
#include "saved games/game_state.h"

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
