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
struct data_array *__attribute__((stdcall)) unknown_00296270(const char *name, long maximum_count, long unknown);

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
	partition->unknown4= unknown_00296270(csnzprintf(buffer, sizeof(buffer), "cluster %s", name),
		MAXIMUM_CLUSTER_PARTITION_REFERENCES, unknown3);
	partition->unknown8= unknown_00296270(csnzprintf(buffer, sizeof(buffer), "%s cluster", name),
		MAXIMUM_CLUSTER_PARTITION_REFERENCES, 0);
}
