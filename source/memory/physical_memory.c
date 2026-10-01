/*
PHYSICAL_MEMORY.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.
*/

#include "physical_memory.h"

#include <string.h>

/* ---------- globals */

/* 0x004e6420 */
struct physical_memory_globals physical_memory_globals;

/* ---------- code */

/* 0x0012b400: takes from the console all the memory that the game gives
out afterwards. The name is a guess */
void physical_memory_initialize(
	void)
{
	memset(&physical_memory_globals, 0, sizeof(physical_memory_globals));
	physical_memory_globals.stage= 0;
	physical_memory_globals.base_address= PHYSICAL_MEMORY_BASE_ADDRESS;
	physical_memory_globals.end_address= PHYSICAL_MEMORY_END_ADDRESS;
	physical_memory_globals.low_address[0]= PHYSICAL_MEMORY_BASE_ADDRESS;
	physical_memory_globals.high_address[0]= PHYSICAL_MEMORY_END_ADDRESS;
	XPhysicalAlloc(PHYSICAL_MEMORY_END_ADDRESS-PHYSICAL_MEMORY_BASE_ADDRESS, PHYSICAL_MEMORY_BASE_ADDRESS, 0,
		PHYSICAL_MEMORY_PROTECTION);
}
