/*
SHADER_DEFINITIONS.H

The shader (shad). Only the start of the tag is worked out.
*/

#ifndef __SHADER_DEFINITIONS_H
#define __SHADER_DEFINITIONS_H

#include "tag_files/tag_groups.h"

#include <stddef.h>

/* ---------- constants */

#define SHADER_TAG 'shad'
#define SHADER_TEMPLATE_TAG 'stem'

/* ---------- structures */

/* the name of this block is a guess. Its diffuse map is the texture that
the tools give a material; a shader for glass, fog and the like has NONE */
struct shader_runtime_properties
{
	struct tag_reference diffuse_map; /* bitm */
	/* the rest of the block */
};

struct shader
{
	struct tag_reference shader_template; /* stem */
	uint32_t material_name; /* a string id */
	struct tag_block runtime_properties; /* shader_runtime_properties, one */
	/* the rest of the tag: among it, the bitmaps of every pass */
};

typedef char verify_shader_runtime_properties_offset[
	offsetof(struct shader, runtime_properties) == 0xC ? 1 : -1];

#endif
