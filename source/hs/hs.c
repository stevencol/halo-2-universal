/*
HS.C

the scripts of the levels. Worked out from halo2ship.exe; the name of the
file is a guess, by the name that the game gives to its table of data
("script node") and the one that the first game has for it.
*/

#include "cseries/cseries.h"
#include "memory/data.h"
#include "scenario/scenario.h"

/* ---------- constants */

enum
{
	/* a bit of a table of data: its elements are not its own, and it has
	none */
	_hs_data_array_disconnected_bit= 0x2,
	/* how many nodes the table has room for, and how big a node is; in the
	scenario: the block of the nodes of its scripts, and two numbers that
	say whether what the scripts need of the map is to be made */
	MAXIMUM_HS_SCRIPT_NODES= 0x9000,
	HS_SCRIPT_NODE_SIZE= 0x14,
	SCENARIO_SCRIPT_NODES_OFFSET= 0x238,
	SCENARIO_UNKNOWN1B8_OFFSET= 0x1B8,
	SCENARIO_UNKNOWN1D0_OFFSET= 0x1D0
};

/* ---------- globals */

/* 0x004f9394: the nodes of the scripts ("script node"). 0x004f9380,
three more tables that stop with the scripts: one whose use is not known,
and the two of the lists of objects (object_lists.c) */
struct data_array *script_node_data;
struct data_array *unknown_004f9380;
struct data_array *object_list_header_data;
struct data_array *list_object_data;

/* 0x0020a850, 0x00209290: what else stops with the scripts when a map goes */
void unknown_0020a850(void);
void unknown_00209290(void);

/* ---------- code */

/* 0x0020a810: the scripts close: the table of the nodes is given back
(data_dispose, which the game has written here), and another cannot be used */
void hs_dispose(
	void)
{
	data_dispose(script_node_data);
	unknown_004f9380->valid= FALSE;
}

/* 0x0020a7d0: when a map goes: unknown_0020a850; the table of the nodes,
unless it had none already, has no elements, as they were those of the map,
and cannot be used; unknown_00209290; and two more tables cannot be used */
void hs_dispose_from_old_map(
	void)
{
	struct data_array *data;

	unknown_0020a850();
	data= script_node_data;
	if (!(data->flags&_hs_data_array_disconnected_bit))
	{
		data->flags|= _hs_data_array_disconnected_bit;
		data->data= NULL;
		data->valid= FALSE;
	}
	unknown_00209290();
	object_list_header_data->valid= FALSE;
	list_object_data->valid= FALSE;
}

/* 0x001ded00, of object_lists.c, and 0x00208ff0: what else starts with the
scripts. 0x002a09c0:
what the scripts need of the map is made. 0x002090a0: the rest of what is
done for a map */
void object_lists_initialize(void);
void unknown_00208ff0(void);
void unknown_002a09c0(void);
void unknown_002090a0(void);

void hs_initialize_for_new_map(void);

/* 0x0020a700: the scripts start: the table of the nodes takes memory of the
heap for itself and its bits, with no elements of its own
(data_initialize_disconnected): those are the ones of each map. Then what
else starts with them, and what is done for a map, which the game goes on
into */
void hs_initialize(
	void)
{
	struct allocation *allocation= heap_allocation;
	struct data_array *data= allocation->functions->allocate(allocation,
		sizeof(struct data_array)+BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_HS_SCRIPT_NODES)*sizeof(unsigned long));

	if (data)
	{
		data_initialize_disconnected(data, "script node", MAXIMUM_HS_SCRIPT_NODES, HS_SCRIPT_NODE_SIZE, 0, allocation,
			(unsigned long *)(data+1));
	}
	script_node_data= data;
	object_lists_initialize();
	unknown_00208ff0();
	hs_initialize_for_new_map();
}

/* 0x0020a750: the scripts for a map. With a scenario: the table of the nodes
gets those of the scenario for its elements, if it has none (data_connect);
and unless the scenario has nothing at 0x1B8 and something at 0x1D0, what
the scripts need of the map is made (unknown_002a09c0). Then two tables can
be used and are empty (data_delete_all), and the rest is done
(unknown_002090a0) */
void hs_initialize_for_new_map(
	void)
{
	const byte *scenario= global_scenario_index!=NONE ? (const byte *)global_scenario : NULL;

	if (scenario)
	{
		struct data_array *data= script_node_data;

		if (data->flags&_hs_data_array_disconnected_bit)
		{
			data_connect(data, *(const long *)(scenario+SCENARIO_SCRIPT_NODES_OFFSET),
				*(void *const *)(scenario+SCENARIO_SCRIPT_NODES_OFFSET+4));
		}
		if (*(const long *)(scenario+SCENARIO_UNKNOWN1B8_OFFSET)!=0 ||
			*(const long *)(scenario+SCENARIO_UNKNOWN1D0_OFFSET)<=0)
		{
			unknown_002a09c0();
		}
	}
	object_list_header_data->valid= TRUE;
	data_delete_all(object_list_header_data);
	list_object_data->valid= TRUE;
	data_delete_all(list_object_data);
	unknown_002090a0();
}
