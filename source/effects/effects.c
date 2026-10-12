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
0x0051ec84, 0x0051ec88, 0x0051ec8c: the four tables of the particles, which
start and stop with the effects (particle_systems.c) */
struct data_array *effect_data;
struct data_array *effect_location_data;
struct data_array *particle_system_data;
struct data_array *particle_data;
struct data_array *particle_emitter_data;
struct data_array *particle_location_data;

/* ---------- code */

/* 0x00175bd0: the effects have no tables */
void effects_dispose(
	void)
{
	particle_location_data= NULL;
	particle_emitter_data= NULL;
	particle_data= NULL;
	particle_system_data= NULL;
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
	particle_system_data->valid= TRUE;
	data_delete_all(particle_system_data);
	particle_data->valid= TRUE;
	data_delete_all(particle_data);
	particle_emitter_data->valid= TRUE;
	data_delete_all(particle_emitter_data);
	particle_location_data->valid= TRUE;
	data_delete_all(particle_location_data);
}

/* 0x00175c70: when a map goes, none of the six can be used */
void effects_dispose_from_old_map(
	void)
{
	particle_location_data->valid= FALSE;
	particle_emitter_data->valid= FALSE;
	particle_data->valid= FALSE;
	particle_system_data->valid= FALSE;
	effect_data->valid= FALSE;
	effect_location_data->valid= FALSE;
}

/* 0x00173de0, of particle_systems.c: the particles start with the effects */
void particle_systems_initialize(void);

/* 0x00175b50: the effects make their two tables, in the memory of the state
of the game (data_new, which the game has written here): 0x100 effects of
0x190 bytes and 0x200 places of 0x3C. Then the other things that start with
them (particle_systems_initialize) */
void effects_initialize(
	void)
{
	effect_data= data_new("effect", 0x100, 0x190, 0, game_state_allocation);
	effect_location_data= data_new("effect location", 0x200, 0x3C, 0, game_state_allocation);
	particle_systems_initialize();
}
