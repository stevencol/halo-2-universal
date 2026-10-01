/*
STRUCTURE_BSP_DEFINITIONS.H

The structure bsp (sbsp), the geometry of a level. Only the start of the tag
is worked out.
*/

#ifndef __STRUCTURE_BSP_DEFINITIONS_H
#define __STRUCTURE_BSP_DEFINITIONS_H

#include "math/real_math.h"
#include "tag_files/tag_groups.h"

#include <stddef.h>

/* ---------- constants */

#define STRUCTURE_BSP_TAG 'sbsp'
#define STRUCTURE_LIGHTMAP_TAG 'ltmp'
#define CACHE_FILE_STRUCTURE_BSP_HEADER_SIGNATURE 'sbsp'

/* ---------- structures */

/* a structure bsp is a block of the cache file apart from the tag data, with
this header; the game reads the block to the address the scenario gives
(0x00122b40) and sets the addresses of the sbsp and ltmp tags from it */
struct cache_file_structure_bsp_header
{
	uint32_t size;
	uint32_t structure_bsp;
	uint32_t structure_lightmap;
	uint32_t signature;
};

struct structure_collision_material
{
	struct tag_reference old_shader;
	int16_t global_material_index;
	int16_t conveyor_surface_index;
	struct tag_reference shader;
};

struct structure_bsp
{
	uint8_t unknown0[0xC];
	struct tag_block collision_materials; /* structure_collision_material */
	struct tag_block collision_bsp; /* collision_bsp, one */
	real vehicle_floor;
	real vehicle_ceiling;
	uint8_t unknown24[8];
	/* these two as in Halo 1, by their counts and sizes: one leaf per leaf
	of the collision bsp, 8 bytes each */
	struct tag_block leaves;
	real_rectangle3d world_bounds;
	struct tag_block surface_references; /* 8 bytes each */
	/* the rest of the tag (cluster data, portals, clusters, ...) */
};

typedef char verify_cache_file_structure_bsp_header_size[
	sizeof(struct cache_file_structure_bsp_header) == 0x10 ? 1 : -1];
typedef char verify_structure_collision_material_size[
	sizeof(struct structure_collision_material) == 0x14 ? 1 : -1];
typedef char verify_structure_bsp_collision_bsp_offset[
	offsetof(struct structure_bsp, collision_bsp) == 0x14 ? 1 : -1];
typedef char verify_structure_bsp_world_bounds_offset[
	offsetof(struct structure_bsp, world_bounds) == 0x34 ? 1 : -1];
typedef char verify_structure_bsp_surface_references_offset[
	offsetof(struct structure_bsp, surface_references) == 0x4C ? 1 : -1];

#endif
