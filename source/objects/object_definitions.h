/*
OBJECT_DEFINITIONS.H

What the tags of the objects have in common (scenery, bipeds, vehicles,
equipment, weapons, machines, controls, crates), and the model (hlmt). Only
the way from an object to its render model is worked out.
*/

#ifndef __OBJECT_DEFINITIONS_H
#define __OBJECT_DEFINITIONS_H

#include "tag_files/tag_groups.h"

#include <stddef.h>

/* ---------- constants */

#define MODEL_TAG 'hlmt'

/* ---------- structures */

struct object_definition
{
	uint8_t unknown0[0x34];
	struct tag_reference model; /* hlmt */
	/* the rest of the tag */
};

struct model_definition
{
	struct tag_reference render_model; /* mode */
	/* the rest of the tag */
};

typedef char verify_object_definition_model_offset[
	offsetof(struct object_definition, model) == 0x34 ? 1 : -1];

#endif
