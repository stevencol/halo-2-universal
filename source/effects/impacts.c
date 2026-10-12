/*
IMPACTS.C

the impacts of things on one another. Worked out from halo2ship.exe; the name
of the file is a guess, by the names that the game gives to its two tables of
data ("impacts", and "impact arrarys", spelt so).
*/

#include "cseries/cseries.h"
#include "memory/data.h"

/* ---------- globals */

/* 0x0051ebfc: the impacts ("impacts": 0x20 of 0xA0 bytes). 0x0051ec00: their
arrays ("impact arrarys": 0x20 of 0x40 bytes) */
struct data_array *impact_data;
struct data_array *impact_array_data;

/* ---------- code */

/* 0x002263c0: the two tables are made, in memory of the heap (data_new, which
the game has written here) */
void impacts_initialize(
	void)
{
	impact_data= data_new("impacts", 0x20, 0xA0, 0, heap_allocation);
	impact_array_data= data_new("impact arrarys", 0x20, 0x40, 0, heap_allocation);
}

/* 0x00226440: the two tables are given back (data_dispose, which the game
has written here), and there are none */
void impacts_dispose(
	void)
{
	data_dispose(impact_data);
	data_dispose(impact_array_data);
	impact_data= NULL;
	impact_array_data= NULL;
}
