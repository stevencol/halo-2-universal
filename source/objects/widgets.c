/*
WIDGETS.C

what hangs from the objects and moves by itself: antennas, cloths and liquids.
Worked out from halo2ship.exe; the name of the file is a guess, by the name
that the game gives to its table of data ("widget").
*/

#include "cseries/cseries.h"

#include <stdint.h>

#include "cache/cache_files.h"
#include "memory/data.h"
#include "objects/objects.h"
#include "objects/widgets.h"
#include "saved games/game_state.h"

/* ---------- globals */

/* 0x004e0320: the widgets ("widget": 0x40 of 0xC bytes) */
struct data_array *widget_data;

/* 0x00467498: the kinds, which in the game are the antennas ('ant!'), the
cloths ('clwd') and the liquids ('tdtl'), with the functions of antenna.c,
cloth.c and liquid.c */
struct widget_type_definition widget_type_definitions[NUMBER_OF_WIDGET_TYPES];

/* ---------- code */

/* 0x000d4830: the table is made, in the memory of the state of the game
(data_new, which the game has written here), and each kind starts */
void widgets_initialize(
	void)
{
	long index;

	widget_data= data_new("widget", 0x40, 0xC, 0, game_state_allocation);
	for (index= 0; index<NUMBER_OF_WIDGET_TYPES; index++)
	{
		if (widget_type_definitions[index].initialize)
		{
			widget_type_definitions[index].initialize();
		}
	}
}

/* 0x000d4890: for a new map the table is good to use, with nothing in it,
and each kind is told */
void widgets_initialize_for_new_map(
	void)
{
	long index;

	data_make_valid(widget_data);
	for (index= 0; index<NUMBER_OF_WIDGET_TYPES; index++)
	{
		if (widget_type_definitions[index].initialize_for_new_map)
		{
			widget_type_definitions[index].initialize_for_new_map();
		}
	}
}

/* 0x000d48d0: when the map goes each kind is told, and the table cannot be
used */
void widgets_dispose_from_old_map(
	void)
{
	long index;

	for (index= 0; index<NUMBER_OF_WIDGET_TYPES; index++)
	{
		if (widget_type_definitions[index].dispose_from_old_map)
		{
			widget_type_definitions[index].dispose_from_old_map();
		}
	}
	widget_data->valid= FALSE;
}

/* 0x000d4db0: each tick, each kind that has a function for it is told how
long the tick is */
void __attribute__((stdcall)) widgets_update(
	float seconds)
{
	long index;

	for (index= 0; index<NUMBER_OF_WIDGET_TYPES; index++)
	{
		if (widget_type_definitions[index].update)
		{
			widget_type_definitions[index].update(seconds);
		}
	}
}

/* an object has its first widget at 0xDC, or NONE */
#define OBJECT_FIRST_WIDGET_INDEX(object_index) ((long *)((byte *) \
	((struct object_header_datum *)object_header_data->data)[(object_index)&0xFFFF].datum+0xDC))

/* 0x000d4a60: the widgets of an object are no more: the kind of each is told,
if the widget is something in the table of its kind, and the object has none */
void widgets_delete(
	long object_index)
{
	long *first_widget_index= OBJECT_FIRST_WIDGET_INDEX(object_index);
	long widget_index= *first_widget_index;

	while (widget_index!=NONE)
	{
		struct widget_datum *widget= &((struct widget_datum *)widget_data->data)[widget_index&0xFFFF];
		long next_widget_index= widget->next_widget_index;

		if (widget->index!=NONE)
		{
			widget_type_definitions[widget->type].delete(widget->index);
		}
		datum_delete(widget_data, widget_index);
		widget_index= next_widget_index;
	}
	*first_widget_index= NONE;
}

/* 0x000d4900: the widgets of a new object are made: the definition of the
object (its tag) has at 0x9C so many widgets, each the group of a tag and the
tag. For each whose group is of one of the kinds, and that has a tag, a
widget is made in the table, and the kind makes its own; if the kind cannot,
the widget is no more. The widgets are put first in the list of the object */
void __attribute__((stdcall)) widgets_new(
	long object_index)
{
	byte *object= (byte *)((struct object_header_datum *)object_header_data->data)[object_index&0xFFFF].datum;
	const byte *definition= (const byte *)(uintptr_t)global_tag_instances[*(const long *)object&0xFFFF].address;
	long index;

	*(long *)(object+0xDC)= NONE;
	for (index= 0; (short)index<*(const long *)(definition+0x9C); index++)
	{
		const byte *element= *(const byte *const *)(definition+0xA0)+(short)index*8;
		short type= NONE;
		short other;

		for (other= 0; other<NUMBER_OF_WIDGET_TYPES; other++)
		{
			if (widget_type_definitions[other].group_tag==*(const unsigned long *)element)
			{
				type= other;
				break;
			}
		}
		if (type!=NONE && *(const long *)(element+4)!=NONE)
		{
			long widget_index= datum_new(widget_data);

			if (widget_index!=NONE)
			{
				struct widget_datum *widget= &((struct widget_datum *)widget_data->data)[widget_index&0xFFFF];

				widget->type= type;
				if (widget_type_definitions[type].new)
				{
					widget->index= widget_type_definitions[type].new(*(const long *)(element+4), object_index);
					if (widget->index!=NONE)
					{
						widget->next_widget_index= *(long *)(object+0xDC);
						*(long *)(object+0xDC)= widget_index;
					}
					else
					{
						datum_delete(widget_data, widget_index);
					}
				}
				else
				{
					widget->next_widget_index= *(long *)(object+0xDC);
					*(long *)(object+0xDC)= widget_index;
					widget->index= NONE;
				}
			}
		}
	}
}

/* 0x000d4ae0: the widget of an object that is a given thing in the table of
its kind is no more: it is taken out of the list of the object, its kind is
told, and it is deleted. The kind is not looked at to find it */
void widget_delete(
	long object_index,
	long index)
{
	if (index!=NONE)
	{
		long *first_widget_index= OBJECT_FIRST_WIDGET_INDEX(object_index);
		long widget_index= *first_widget_index;

		if (widget_index!=NONE)
		{
			struct widget_datum *widgets= (struct widget_datum *)widget_data->data;
			struct widget_datum *widget= &widgets[widget_index&0xFFFF];

			if (widget->index==index)
			{
				*first_widget_index= widget->next_widget_index;
			}
			else
			{
				struct widget_datum *previous;

				do
				{
					previous= &widgets[widget_index&0xFFFF];
					widget_index= previous->next_widget_index;
					if (widget_index==NONE)
					{
						return;
					}
					widget= &widgets[widget_index&0xFFFF];
				} while (widget->index!=index);
				previous->next_widget_index= widget->next_widget_index;
			}
			if (widget->index!=NONE)
			{
				widget_type_definitions[widget->type].delete(widget->index);
			}
			datum_delete(widget_data, widget_index);
		}
	}
}
