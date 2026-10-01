/*
PHYSICAL_MEMORY.H

The block of the console's memory that the game takes when it starts, and
gives out from its two ends. Worked out from halo2ship.exe; the names are
guesses, after Halo 1's physical_memory_map.c.
*/

#ifndef __PHYSICAL_MEMORY_H
#define __PHYSICAL_MEMORY_H

#include <stddef.h>
#include <stdint.h>

/* ---------- constants */

enum
{
	/* the addresses of the console's memory, without the high bit */
	PHYSICAL_MEMORY_BASE_ADDRESS= 0x00061000,
	PHYSICAL_MEMORY_END_ADDRESS= 0x03145000,
	PHYSICAL_MEMORY_PAGE_SIZE= 0x1000,

	/* the console maps all of its memory here */
	PHYSICAL_MEMORY_ADDRESS_FLAG= (int)0x80000000,

	/* PAGE_READWRITE */
	PHYSICAL_MEMORY_PROTECTION= 4,

	MAXIMUM_PHYSICAL_MEMORY_STAGES= 5
};

/* ---------- structures */

struct physical_memory_globals
{
	/* a stage starts with what the stage before it left, and gives it back
	when it ends */
	long stage;
	uint32_t base_address;
	uint32_t end_address;
	/* per stage: the memory that is free, from low_address to high_address */
	uint32_t low_address[MAXIMUM_PHYSICAL_MEMORY_STAGES];
	uint32_t high_address[MAXIMUM_PHYSICAL_MEMORY_STAGES];
};

typedef char verify_physical_memory_globals_size[
	sizeof(struct physical_memory_globals) == 0x34 ? 1 : -1];

/* ---------- prototypes of functions of the console's libraries */

/* 0x002d156a */
void *XPhysicalAlloc(uint32_t size, uint32_t address, uint32_t alignment, uint32_t protection);

/* 0x002d15da */
void XPhysicalProtect(void *address, uint32_t size, uint32_t protection);

/* ---------- prototypes */

void physical_memory_initialize(void);

/* ---------- globals */

extern struct physical_memory_globals physical_memory_globals;

/* ---------- inline code */

/* takes memory from the low end of what is free, in whole pages; NULL if
there is not enough. The game has no function for it: the code is in each
function that takes memory */
static inline void *physical_memory_allocate_low(
	uint32_t size)
{
	uint32_t *low_address= &physical_memory_globals.low_address[physical_memory_globals.stage];
	uint32_t address= *low_address;
	void *result= NULL;

	size= (size+PHYSICAL_MEMORY_PAGE_SIZE-1)&~(uint32_t)(PHYSICAL_MEMORY_PAGE_SIZE-1);
	if ((long)(address+size)<=(long)physical_memory_globals.high_address[physical_memory_globals.stage])
	{
		*low_address= address+size;
		if (address)
		{
			result= (void *)(uintptr_t)(address|(uint32_t)PHYSICAL_MEMORY_ADDRESS_FLAG);
			XPhysicalProtect(result, size, PHYSICAL_MEMORY_PROTECTION);
		}
	}

	return result;
}

/* the same, from the high end */
static inline void *physical_memory_allocate_high(
	uint32_t size)
{
	uint32_t *high_address= &physical_memory_globals.high_address[physical_memory_globals.stage];
	uint32_t address;
	void *result= NULL;

	size= (size+PHYSICAL_MEMORY_PAGE_SIZE-1)&~(uint32_t)(PHYSICAL_MEMORY_PAGE_SIZE-1);
	address= *high_address-size;
	if ((long)address>=(long)physical_memory_globals.low_address[physical_memory_globals.stage])
	{
		*high_address= address;
		if (address)
		{
			result= (void *)(uintptr_t)(address|(uint32_t)PHYSICAL_MEMORY_ADDRESS_FLAG);
			XPhysicalProtect(result, size, PHYSICAL_MEMORY_PROTECTION);
		}
	}

	return result;
}

#endif
