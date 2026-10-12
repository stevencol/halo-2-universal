/*
PARTICLE_SYSTEMS.C

the particles of the effects. Worked out from halo2ship.exe; the name of the
file is a guess, by the names that the game gives to its four tables of data
("particle_system", "particles", "particle_emitter", "particle_location").
*/

#include "cseries/cseries.h"
#include "memory/data.h"
#include "saved games/game_state.h"

/* ---------- globals */

/* of effects.c, which empties them for a new map: 0x00510c74 (0x80 of 0x54
bytes), 0x0051ec84 (0x400 of 0x40), 0x0051ec88 (0x100 of 0x4C) and 0x0051ec8c
(0x100 of 0x34) */
extern struct data_array *particle_system_data;
extern struct data_array *particle_data;
extern struct data_array *particle_emitter_data;
extern struct data_array *particle_location_data;

/* ---------- code */

/* 0x00173de0: the particles make their four tables, in the memory of the
state of the game (data_new, which the game has written here) */
void particle_systems_initialize(
	void)
{
	particle_system_data= data_new("particle_system", 0x80, 0x54, 0, game_state_allocation);
	particle_data= data_new("particles", 0x400, 0x40, 0, game_state_allocation);
	particle_emitter_data= data_new("particle_emitter", 0x100, 0x4C, 0, game_state_allocation);
	particle_location_data= data_new("particle_location", 0x100, 0x34, 0, game_state_allocation);
}
