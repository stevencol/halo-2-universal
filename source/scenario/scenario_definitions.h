/*
SCENARIO_DEFINITIONS.H

The scenario (scnr). Only the structure bsps and the objects it places are
worked out.
*/

#ifndef __SCENARIO_DEFINITIONS_H
#define __SCENARIO_DEFINITIONS_H

#include "math/real_math.h"
#include "tag_files/tag_groups.h"

#include <stddef.h>

/* ---------- constants */

#define SCENARIO_TAG 'scnr'

/* the offsets in the scenario of its tag blocks. Each kind of object has
its placements (scenario_object_placement at the start of each, with more
after it that depends on the kind) and its palette
(scenario_object_palette_entry): the tags of the objects that the level
uses */
enum
{
	SCENARIO_OBJECT_NAMES_OFFSET= 0x48, /* 0x24 bytes each */
	SCENARIO_SCENERY_OFFSET= 0x50, /* 0x5C bytes each */
	SCENARIO_SCENERY_PALETTE_OFFSET= 0x58,
	SCENARIO_BIPEDS_OFFSET= 0x60, /* 0x54 */
	SCENARIO_BIPED_PALETTE_OFFSET= 0x68,
	SCENARIO_VEHICLES_OFFSET= 0x70, /* 0x54 */
	SCENARIO_VEHICLE_PALETTE_OFFSET= 0x78,
	SCENARIO_EQUIPMENT_OFFSET= 0x80, /* 0x38 */
	SCENARIO_EQUIPMENT_PALETTE_OFFSET= 0x88,
	SCENARIO_WEAPONS_OFFSET= 0x90, /* 0x54 */
	SCENARIO_WEAPON_PALETTE_OFFSET= 0x98,
	SCENARIO_MACHINES_OFFSET= 0xA8, /* 0x48 */
	SCENARIO_MACHINE_PALETTE_OFFSET= 0xB0,
	SCENARIO_CONTROLS_OFFSET= 0xB8, /* 0x44 */
	SCENARIO_CONTROL_PALETTE_OFFSET= 0xC0,
	SCENARIO_STRUCTURE_BSPS_OFFSET= 0x210, /* scenario_structure_bsp_reference */
	SCENARIO_CRATES_OFFSET= 0x328, /* 0x4C */
	SCENARIO_CRATE_PALETTE_OFFSET= 0x330
};

/* ---------- structures */

struct scenario_object_palette_entry
{
	struct tag_reference object;
	uint8_t unknown8[0x20];
};

struct scenario_object_placement
{
	/* in the palette of the kind of object */
	int16_t palette_index;
	/* in the object names; NONE for an object with no name */
	int16_t name_index;
	uint32_t placement_flags;
	real_point3d position;
	/* yaw, pitch and roll, in radians */
	real rotation[3];
	/* 0 for an object of the size of its model */
	real scale;
};

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

typedef char verify_scenario_object_palette_entry_size[
	sizeof(struct scenario_object_palette_entry) == 0x28 ? 1 : -1];
typedef char verify_scenario_object_placement_size[
	sizeof(struct scenario_object_placement) == 0x24 ? 1 : -1];
typedef char verify_scenario_object_placement_scale_offset[
	offsetof(struct scenario_object_placement, scale) == 0x20 ? 1 : -1];
typedef char verify_scenario_structure_bsp_reference_size[
	sizeof(struct scenario_structure_bsp_reference) == 0x44 ? 1 : -1];

#endif
