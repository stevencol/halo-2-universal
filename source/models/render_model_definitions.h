/*
RENDER_MODEL_DEFINITIONS.H

The render model (mode): the geometry of an object that the game draws.
Only part of the tag is worked out.

A model has regions; a region has permutations, the forms it can take; a
permutation has a section for each level of detail. The vertices are in the
space of the model, in the pose it was made in.
*/

#ifndef __RENDER_MODEL_DEFINITIONS_H
#define __RENDER_MODEL_DEFINITIONS_H

#include "geometry/geometry_definitions.h"
#include "math/real_math.h"
#include "tag_files/tag_groups.h"

#include <stddef.h>

/* ---------- constants */

#define RENDER_MODEL_TAG 'mode'

enum
{
	NUMBER_OF_RENDER_MODEL_LEVELS_OF_DETAIL= 6
};

/* ---------- structures */

/* the least and the greatest value of each coordinate of the compressed
vertices: a 16-bit integer goes from the least to the greatest */
struct render_model_compression_info
{
	real position_bounds_x[2];
	real position_bounds_y[2];
	real position_bounds_z[2];
	real texcoord_bounds_u[2];
	real texcoord_bounds_v[2];
	uint8_t unknown28[0x10];
};

struct render_model_permutation
{
	uint32_t name; /* a string id */
	/* the least detailed first; NONE where there is none */
	int16_t section_indices[NUMBER_OF_RENDER_MODEL_LEVELS_OF_DETAIL];
};

struct render_model_region
{
	uint32_t name; /* a string id */
	int16_t unknown4;
	int16_t unknown6;
	struct tag_block permutations; /* render_model_permutation */
};

struct render_model_section
{
	uint16_t unknown0;
	uint16_t unknown2;
	struct geometry_section_info section_info;
	uint8_t unknown2C[0xC];
	struct geometry_block_info geometry_block_info;
};

struct render_model_node
{
	uint32_t name; /* a string id */
	int16_t parent_node_index;
	int16_t first_child_node_index;
	int16_t next_sibling_node_index;
	int16_t unknownA;
	real_point3d default_translation;
	real default_rotation[4];
	uint8_t unknown28[0x38];
};

/* as the structure bsp's material */
struct render_model_material
{
	struct tag_reference old_shader;
	struct tag_reference shader;
	uint8_t unknown10[0x10];
};

struct render_model
{
	uint32_t name; /* a string id */
	uint8_t unknown4[0x10];
	struct tag_block compression_info; /* render_model_compression_info */
	struct tag_block regions; /* render_model_region */
	struct tag_block sections; /* render_model_section */
	uint8_t unknown2C[0x1C];
	struct tag_block nodes; /* render_model_node */
	uint8_t unknown50[0x10];
	struct tag_block materials; /* render_model_material */
	/* the rest of the tag */
};

typedef char verify_render_model_compression_info_size[
	sizeof(struct render_model_compression_info) == 0x38 ? 1 : -1];
typedef char verify_render_model_permutation_size[
	sizeof(struct render_model_permutation) == 0x10 ? 1 : -1];
typedef char verify_render_model_region_size[
	sizeof(struct render_model_region) == 0x10 ? 1 : -1];
typedef char verify_render_model_section_size[
	sizeof(struct render_model_section) == 0x5C ? 1 : -1];
typedef char verify_render_model_section_block_info_offset[
	offsetof(struct render_model_section, geometry_block_info) == 0x38 ? 1 : -1];
typedef char verify_render_model_node_size[
	sizeof(struct render_model_node) == 0x60 ? 1 : -1];
typedef char verify_render_model_material_size[
	sizeof(struct render_model_material) == 0x20 ? 1 : -1];
typedef char verify_render_model_sections_offset[
	offsetof(struct render_model, sections) == 0x24 ? 1 : -1];
typedef char verify_render_model_nodes_offset[
	offsetof(struct render_model, nodes) == 0x48 ? 1 : -1];
typedef char verify_render_model_materials_offset[
	offsetof(struct render_model, materials) == 0x60 ? 1 : -1];

#endif
