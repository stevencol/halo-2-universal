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
	/* a widget of the kind is made for an object, from a tag: what it is in
	the table of its kind, or NONE */
	long (__attribute__((stdcall)) *new)(long definition_index, long object_index);
	/* a widget of the kind is no more: with what the widget is in the table
	of its kind */
	void (__attribute__((stdcall)) *delete)(long index);
	/* each tick, with how long the tick is */
	void (__attribute__((stdcall)) *update)(float seconds);
	void *unknown24[5];
};

typedef char verify_widget_type_definition_size[
	sizeof(struct widget_type_definition) == 0x38 ? 1 : -1];

/* a widget of an object: its kind, what it is in the table of its kind (or
NONE), and the next widget of the object (or NONE) */
struct widget_datum
{
	short identifier;
	short type;
	long index;
	long next_widget_index;
};

typedef char verify_widget_datum_size[
	sizeof(struct widget_datum) == 0xC ? 1 : -1];

/* ---------- prototypes/WIDGETS.C */

void widgets_initialize(void);
void widgets_initialize_for_new_map(void);
void widgets_dispose_from_old_map(void);
void __attribute__((stdcall)) widgets_update(float seconds);
void widgets_delete(long object_index);
void __attribute__((stdcall)) widgets_new(long object_index);

#endif
