/*
CLUSTER_PARTITIONS.H

which things of a kind (the lights, for one) are in each cluster of the
structure bsp. The names are guesses, by the names that the game gives to the
two tables of data of each ("cluster <name>", "<name> cluster").
*/

#ifndef __CLUSTER_PARTITIONS_H
#define __CLUSTER_PARTITIONS_H

#include "cseries/cseries.h"
#include "memory/data.h"

/* ---------- constants */

enum
{
	/* of what unknown0 points to: 0x200 numbers, one for each cluster, it
	seems; and so many things at most in each of the two tables */
	CLUSTER_PARTITION_UNKNOWN0_SIZE= 0x800,
	MAXIMUM_CLUSTER_PARTITION_REFERENCES= 0x800
};

/* ---------- structures */

struct cluster_partition
{
	long *unknown0;
	/* "cluster <name>" */
	struct data_array *unknown4;
	/* "<name> cluster" */
	struct data_array *unknown8;
};

typedef char verify_cluster_partition_size[
	sizeof(struct cluster_partition) == 0xC ? 1 : -1];

/* ---------- prototypes/CLUSTER_PARTITIONS.C */

void __attribute__((stdcall)) cluster_partition_initialize(struct cluster_partition *partition, const char *name,
	long unknown3);

#endif
