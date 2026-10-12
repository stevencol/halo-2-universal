/*
HS.C

the scripts of the levels. Worked out from halo2ship.exe; the name of the
file is a guess, by the name that the game gives to its table of data
("script node") and the one that the first game has for it.
*/

#include "cseries/cseries.h"
#include "memory/data.h"

/* ---------- constants */

enum
{
	/* a bit of a table of data: its elements are not its own, and it has
	none */
	_hs_data_array_disconnected_bit= 0x2
};

/* ---------- globals */

/* 0x004f9394: the nodes of the scripts ("script node"). 0x004f9380,
0x004f55d8, 0x004f55d4: three more tables that stop with the scripts: what
each is of is not known */
struct data_array *script_node_data;
struct data_array *unknown_004f9380;
struct data_array *unknown_004f55d8;
struct data_array *unknown_004f55d4;

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
	unknown_004f55d8->valid= FALSE;
	unknown_004f55d4->valid= FALSE;
}
