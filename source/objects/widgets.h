/*
WIDGETS.H

what hangs from the objects and moves by itself: antennas, cloths and liquids.
The names are guesses.
*/

#ifndef __WIDGETS_H
#define __WIDGETS_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	NUMBER_OF_WIDGET_TYPES= 3
};

/* ---------- structures */

/* a kind of widget: the group of its tags, and its functions; any of them
can be NULL */
struct widget_type_definition
{
	unsigned long group_tag;
	long unknown4;
	void (*initialize)(void);
	void (*initialize_for_new_map)(void);
	void (*dispose_from_old_map)(void);
	void *unknown14;
	void *unknown18;
	void *unknown1C;
	/* each tick, with how long the tick is */
	void (__attribute__((stdcall)) *update)(float seconds);
	void *unknown24[5];
};

typedef char verify_widget_type_definition_size[
	sizeof(struct widget_type_definition) == 0x38 ? 1 : -1];

/* ---------- prototypes/WIDGETS.C */

void widgets_initialize(void);
void widgets_initialize_for_new_map(void);
void widgets_dispose_from_old_map(void);
void __attribute__((stdcall)) widgets_update(float seconds);

#endif
