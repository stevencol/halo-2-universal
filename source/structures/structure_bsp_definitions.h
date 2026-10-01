/*
STRUCTURE_BSP_DEFINITIONS.H

The structure bsp (sbsp), the geometry of a level. Only part of the tag is
worked out.
*/

#ifndef __STRUCTURE_BSP_DEFINITIONS_H
#define __STRUCTURE_BSP_DEFINITIONS_H

#include "geometry/geometry_definitions.h"
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

/* the shader of a part of the geometry that is drawn */
struct structure_material
{
	struct tag_reference old_shader;
	struct tag_reference shader;
	uint8_t unknown10[0x10];
};

/* a piece of the level's geometry, in world space */
struct structure_cluster
{
	struct geometry_section_info section_info;
	struct geometry_block_info geometry_block_info;
	uint8_t unknown4C[0x64];
};

/* geometry that the level has one or more copies of */
struct structure_instanced_geometry_definition
{
	struct geometry_section_info section_info;
	struct geometry_block_info geometry_block_info;
	uint8_t unknown4C[0x7C];
};

/* a copy of an instanced geometry definition; a point of the definition is
at position + scale*(x*forward + y*left + z*up) */
struct structure_instanced_geometry_instance
{
	real scale;
	real_vector3d forward;
	real_vector3d left;
	real_vector3d up;
	real_point3d position;
	int16_t definition_index;
	uint16_t flags;
	uint8_t unknown38[0x20];
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
	uint8_t unknown54[0x48];
	struct tag_block clusters; /* structure_cluster */
	struct tag_block materials; /* structure_material */
	uint8_t unknownAC[0x8C];
	struct tag_block instanced_geometry_definitions; /* structure_instanced_geometry_definition */
	struct tag_block instanced_geometry_instances; /* structure_instanced_geometry_instance */
	/* the rest of the tag */
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
typedef char verify_structure_bsp_clusters_offset[
	offsetof(struct structure_bsp, clusters) == 0x9C ? 1 : -1];
typedef char verify_structure_bsp_instanced_geometry_definitions_offset[
	offsetof(struct structure_bsp, instanced_geometry_definitions) == 0x138 ? 1 : -1];
typedef char verify_structure_material_size[
	sizeof(struct structure_material) == 0x20 ? 1 : -1];
typedef char verify_structure_cluster_size[
	sizeof(struct structure_cluster) == 0xB0 ? 1 : -1];
typedef char verify_structure_instanced_geometry_definition_size[
	sizeof(struct structure_instanced_geometry_definition) == 0xC8 ? 1 : -1];
typedef char verify_structure_instanced_geometry_instance_size[
	sizeof(struct structure_instanced_geometry_instance) == 0x58 ? 1 : -1];
typedef char verify_structure_instanced_geometry_instance_definition_index_offset[
	offsetof(struct structure_instanced_geometry_instance, definition_index) == 0x34 ? 1 : -1];

#endif
