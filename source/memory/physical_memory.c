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

/* 0x000531b0: what gives memory to something that asks as data arrays do:
whole pages from the high end of what is free at this stage; NULL if there
is not so much. What asks is not looked at. The game has it far from the rest
of this file */
void *__attribute__((thiscall)) physical_memory_allocation_allocate_high(
	void *allocation,
	unsigned long size)
{
	uint32_t *high_address= &physical_memory_globals.high_address[physical_memory_globals.stage];
	uint32_t address;
	void *result;

	(void)allocation;
	size= (size+0xFFF)&~0xFFFUL;
	address= *high_address-size;
	if ((long)address<(long)physical_memory_globals.low_address[physical_memory_globals.stage])
	{
		return NULL;
	}
	*high_address= address;
	if (!address)
	{
		return NULL;
	}
	result= (void *)(uintptr_t)(address|0x80000000);
	XPhysicalProtect(result, size, PHYSICAL_MEMORY_PROTECTION);

	return result;
}
