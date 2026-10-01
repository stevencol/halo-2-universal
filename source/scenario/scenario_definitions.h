/*
SCENARIO_DEFINITIONS.H

The scenario (scnr). Only the structure bsps are worked out.
*/

#ifndef __SCENARIO_DEFINITIONS_H
#define __SCENARIO_DEFINITIONS_H

#include "tag_files/tag_groups.h"

/* ---------- constants */

#define SCENARIO_TAG 'scnr'

enum
{
	/* the offset in the scenario of the tag block of
	scenario_structure_bsp_reference */
	SCENARIO_STRUCTURE_BSPS_OFFSET= 0x210
};

/* ---------- structures */

struct scenario_structure_bsp_reference
{
	/* the block of the cache file that holds the structure bsp, and the
	address the game reads it to; every structure bsp of a scenario has the
	same address, since one is loaded at a time */
	uint32_t block_offset;
	uint32_t block_size;
	uint32_t block_address;
	uint32_t unknownC;
	struct tag_reference structure_bsp;
	struct tag_reference structure_lightmap;
	uint8_t unknown20[0x24];
};

typedef char verify_scenario_structure_bsp_reference_size[
	sizeof(struct scenario_structure_bsp_reference) == 0x44 ? 1 : -1];

#endif
