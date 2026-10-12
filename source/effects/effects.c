/*
EFFECTS.C

the effects. Worked out from halo2ship.exe; the name of the file is a guess, by
the name that the game gives to its table of data ("effect", "effect location").
*/

#include "cseries/cseries.h"
#include "memory/data.h"
#include "saved games/game_state.h"

/* ---------- globals */

/* 0x004ea93c: the effects ("effect": 0x100 of 0x190 bytes). 0x004ea938:
where each is ("effect location": 0x200 of 0x3C bytes). 0x00510c74,
0x0051ec84, 0x0051ec88, 0x0051ec8c: four more tables of data that start and
stop with the effects: what each is of is not known */
struct data_array *effect_data;
struct data_array *effect_location_data;
struct data_array *unknown_00510c74;
struct data_array *unknown_0051ec84;
struct data_array *unknown_0051ec88;
struct data_array *unknown_0051ec8c;

/* ---------- code */

/* 0x00175bd0: the effects have no tables */
void effects_dispose(
	void)
{
	unknown_0051ec8c= NULL;
	unknown_0051ec88= NULL;
	unknown_0051ec84= NULL;
	unknown_00510c74= NULL;
	if (effect_data)
	{
		effect_data= NULL;
	}
	if (effect_location_data)
	{
		effect_location_data= NULL;
	}
}

/* 0x00175c10: for a new map, each of the six tables can be used, and is
empty (data_delete_all) */
void effects_initialize_for_new_map(
	void)
{
	effect_data->valid= TRUE;
	data_delete_all(effect_data);
	effect_location_data->valid= TRUE;
	data_delete_all(effect_location_data);
	unknown_00510c74->valid= TRUE;
	data_delete_all(unknown_00510c74);
	unknown_0051ec84->valid= TRUE;
	data_delete_all(unknown_0051ec84);
	unknown_0051ec88->valid= TRUE;
	data_delete_all(unknown_0051ec88);
	unknown_0051ec8c->valid= TRUE;
	data_delete_all(unknown_0051ec8c);
}

/* 0x00175c70: when a map goes, none of the six can be used */
void effects_dispose_from_old_map(
	void)
{
	unknown_0051ec8c->valid= FALSE;
	unknown_0051ec88->valid= FALSE;
	unknown_0051ec84->valid= FALSE;
	unknown_00510c74->valid= FALSE;
	effect_data->valid= FALSE;
	effect_location_data->valid= FALSE;
}

/* 0x00173de0: the other things that start with the effects */
void unknown_00173de0(void);

/* 0x00175b50: the effects make their two tables, in the memory of the state
of the game (data_new, which the game has written here): 0x100 effects of
0x190 bytes and 0x200 places of 0x3C. Then the other things that start with
them (unknown_00173de0) */
void effects_initialize(
	void)
{
	effect_data= data_new("effect", 0x100, 0x190, 0, game_state_allocation);
	effect_location_data= data_new("effect location", 0x200, 0x3C, 0, game_state_allocation);
	unknown_00173de0();
}
