/*
UNITS.C

the bipeds and the vehicles: what they carry. Worked out from halo2ship.exe;
the name of the file is a guess, by the one that the first game has.
*/

#include "units.h"

#include "objects/objects.h"

/* ---------- code */

/* 0x000cbd50: one of the weapons that a unit carries, by its place; NONE
for the place NONE */
long unit_get_weapon(
	long unit_index,
	short weapon_index)
{
	const struct object_header_datum *header;

	if (weapon_index==NONE)
	{
		return NONE;
	}
	header= (const struct object_header_datum *)object_header_data->data+(unit_index&0xFFFF);

	return ((const long *)((const byte *)header->datum+UNIT_WEAPONS_OFFSET))[weapon_index];
}

/* 0x000c7100: the last of the chain that the units make by what each has at
0x24C, from the one after a unit; NONE if the unit has none there */
long unit_get_ultimate_unknown24C(
	long unit_index)
{
	const struct object_header_datum *headers= (const struct object_header_datum *)object_header_data->data;
	long next= *(const long *)((const byte *)headers[unit_index&0xFFFF].datum+UNIT_UNKNOWN24C_OFFSET);
	long last= NONE;

	while (next!=NONE)
	{
		last= next;
		next= *(const long *)((const byte *)headers[last&0xFFFF].datum+UNIT_UNKNOWN24C_OFFSET);
	}

	return last;
}
