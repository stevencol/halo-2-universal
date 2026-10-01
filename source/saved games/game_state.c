/*
GAME_STATE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The names are those of Halo 1's game_state.c where it has them. Not here yet:
the functions that keep a state of the game in a core file or in a saved
game (0x001240b0 to 0x00124490, 0x00124640, 0x00124770 on).
*/

#include "game_state.h"

#include "cache/cache_files.h"
#include "game/game.h"
#include "memory/crc.h"
#include "memory/physical_memory.h"
#include "scenario/scenario.h"

#include <string.h>

/* ---------- constants */

enum
{
	/* XPhysicalProtect: PAGE_READWRITE, and with PAGE_WRITECOMBINE */
	GAME_STATE_CPU_PROTECTION= 0x4,
	GAME_STATE_GPU_PROTECTION= 0x404,

	/* a game that is lost this many times, each less than so many seconds
	after the checkpoint was made, goes back to the checkpoint before */
	MAXIMUM_GAME_STATE_DEATHS_AFTER_SAVE= 5,

	/* what game_state_revert tells the procs, from its two flags */
	_game_state_load_flag5_bit= 5,
	_game_state_load_flag6_bit= 6
};

#define GAME_STATE_DEATH_AFTER_SAVE_SECONDS 8.0f

/* ---------- prototypes of functions not decompiled yet */

/* what the game does before and after it writes its state, and before and
after it reads it. Their names are not known */
void GAME_STATE_PROC unknown_001c3540(unsigned long flags);
void GAME_STATE_PROC unknown_00068090(unsigned long flags);
void GAME_STATE_PROC unknown_0016eff0(unsigned long flags);
void GAME_STATE_PROC unknown_001e6af0(unsigned long flags);
void GAME_STATE_PROC unknown_001c3590(unsigned long flags);
void GAME_STATE_PROC unknown_001889d0(unsigned long flags);
void GAME_STATE_PROC unknown_00155f10(unsigned long flags);
void GAME_STATE_PROC unknown_00067fb0(unsigned long flags);
void GAME_STATE_PROC unknown_001264c0(unsigned long flags);
void GAME_STATE_PROC unknown_00188ac0(unsigned long flags);
void GAME_STATE_PROC unknown_0016f090(unsigned long flags);
void GAME_STATE_PROC unknown_0014b660(unsigned long flags);
void GAME_STATE_PROC unknown_00067fc0(unsigned long flags);
void GAME_STATE_PROC unknown_00043970(unsigned long flags);
void GAME_STATE_PROC unknown_0003d380(unsigned long flags);
void GAME_STATE_PROC unknown_00155360(unsigned long flags);
void GAME_STATE_PROC unknown_0024c829(unsigned long flags);
void GAME_STATE_PROC unknown_001c7f80(unsigned long flags);
void GAME_STATE_PROC unknown_0017d190(unsigned long flags);
void GAME_STATE_PROC unknown_0016f0c0(unsigned long flags);
void GAME_STATE_PROC unknown_00226030(unsigned long flags);

/* ---------- globals */

/* 0x004e3b60 */
struct game_state_globals game_state_globals;

/* 0x00510c2c: what gives memory of the state of the game to what takes an
allocation, as the data arrays. It is in the state of the game itself */
struct allocation *game_state_allocation;

/* 0x00453498 */
const struct allocation_functions game_state_allocation_functions=
{
	game_state_allocation_allocate,
	(void (ALLOCATION_CONVENTION *)(struct allocation *, void *))no_operation1
};

/* 0x0046e320 */
game_state_proc game_state_before_save_procs[NUMBER_OF_GAME_STATE_BEFORE_SAVE_PROCS]=
{
	unknown_001c3540,
	unknown_00068090,
	unknown_0016eff0,
	unknown_001e6af0
};

/* 0x0046e330 */
game_state_proc game_state_after_save_procs[NUMBER_OF_GAME_STATE_AFTER_SAVE_PROCS]=
{
	unknown_001c3590
};

/* 0x0046e334 */
game_state_proc game_state_before_load_procs[NUMBER_OF_GAME_STATE_BEFORE_LOAD_PROCS]=
{
	unknown_001c3540,
	game_state_buffer_protect,
	unknown_001889d0,
	unknown_00155f10,
	unknown_00067fb0
};

/* 0x0046e348 */
game_state_proc game_state_after_load_procs[NUMBER_OF_GAME_STATE_AFTER_LOAD_PROCS]=
{
	game_state_allocation_restore,
	(game_state_proc)no_operation1,
	unknown_001264c0,
	unknown_00188ac0,
	unknown_0016f090,
	unknown_0014b660,
	unknown_00067fc0,
	unknown_00043970,
	unknown_0003d380,
	(game_state_proc)no_operation1,
	(game_state_proc)no_operation1,
	(game_state_proc)no_operation1,
	game_state_set_revert_time,
	unknown_00155360,
	unknown_0024c829,
	unknown_001c3590,
	unknown_001c7f80,
	unknown_0017d190,
	unknown_0016f0c0,
	unknown_00226030
};

/* 0x00485ab0 and 0x004e6398: eight bytes that game_state_save copies from
one to the other. What they are is not known, and they are not of this
file */
unsigned long unknown_00485ab0[2];
unsigned long unknown_004e6398[2];

/* ---------- code */

/* 0x00123b00: lets the game write all of the memory of its state. One of the
procs before a state is read. The name is a guess */
void GAME_STATE_PROC game_state_buffer_protect(
	unsigned long flags)
{
	byte *base_address= game_state_globals.base_address;

	(void)flags;

	XPhysicalProtect(base_address, GAME_STATE_CPU_SIZE, GAME_STATE_CPU_PROTECTION);
	XPhysicalProtect(base_address+GAME_STATE_CPU_SIZE, GAME_STATE_GPU_SIZE, GAME_STATE_GPU_PROTECTION);
}

/* 0x00123b30 */
void game_state_initialize(
	void)
{
	if (!game_state_globals.initialized)
	{
		game_state_globals.allocation_size_checksum= CRC_NEW;
		game_state_globals.base_address= game_state_allocate_buffer(GAME_STATE_GPU_SIZE, GAME_STATE_CPU_SIZE);
		memset(game_state_globals.base_address, 0, GAME_STATE_CPU_SIZE+GAME_STATE_GPU_SIZE);
		game_state_create_or_open_file();

		game_state_globals.header= game_state_malloc(sizeof(struct game_state_header));
		game_state_globals.initialized= TRUE;
		game_state_allocation= game_state_malloc(sizeof(struct allocation));
		if (game_state_allocation)
		{
			game_state_allocation->functions= &game_state_allocation_functions;
		}
	}
}

/* 0x00123bf0 */
void game_state_dispose(
	void)
{
	if (game_state_globals.initialized)
	{
		game_state_free_buffer();
		game_state_close_file();
		memset(&game_state_globals, 0, sizeof(game_state_globals));
	}
}

/* 0x00123c20 */
void game_state_initialize_for_new_map(
	void)
{
	struct game_state_header *header;

	game_state_buffer_protect(0);

	game_state_globals.locked= TRUE;
	game_state_globals.saved_game_valid= FALSE;
	game_state_globals.previous_saved_game_valid= FALSE;
	memset(game_state_globals.headers, 0, sizeof(game_state_globals.headers));
	game_state_globals.revert_time= NONE;

	header= game_state_globals.header;
	memset(header, 0, sizeof(*header));
	header->allocation_size_checksum= game_state_globals.allocation_size_checksum;
	header->base_address= game_state_globals.base_address;
	strncpy(header->map_name, cache_file_globals.header.scenario_path, sizeof(header->map_name));
	header->map_name[sizeof(header->map_name)-1]= 0;
	strncpy(header->build, "02.09.27.09809", sizeof(header->build));
	header->build[sizeof(header->build)-1]= 0;
	header->cache_file_checksum= cache_file_globals.header.unknown144;
	memcpy(&header->options, &main_game_globals->options, sizeof(header->options));

	game_state_globals.unknown2538= FALSE;
}

/* 0x00123d20: keeps in the header which structure bsp is loaded. The name is
a guess */
void game_state_set_structure_bsp_index(
	void)
{
	game_state_globals.header->structure_bsp_index= global_structure_bsp_index;
}

/* 0x00123d40: memory of the state of the game, in whole 32-bit words. The
sizes of what is taken go to a checksum, which tells a state of the game that
was not made with the same ones. In Halo 1 the function also takes a name and
a type, for a list of what was taken */
void *game_state_malloc(
	long size)
{
	byte *pointer;

	size= (size+3)&~3;
	pointer= (byte *)game_state_globals.base_address+game_state_globals.cpu_allocation_size;
	game_state_globals.cpu_allocation_size+= size;
	crc_checksum_buffer(&size, sizeof(size), &game_state_globals.allocation_size_checksum);

	return pointer;
}

/* 0x00123d80: the same, at an address that is a multiple of two to the
alignment. The name is a guess */
void *game_state_malloc_aligned(
	long size,
	long alignment_bits)
{
	unsigned long alignment= 1UL<<(alignment_bits&31);
	byte *pointer;

	size= (long)((alignment+(unsigned long)size+3)&~3UL);
	pointer= (byte *)game_state_globals.base_address+game_state_globals.cpu_allocation_size;
	game_state_globals.cpu_allocation_size+= size;
	crc_checksum_buffer(&size, sizeof(size), &game_state_globals.allocation_size_checksum);

	return (void *)(((uintptr_t)pointer+alignment-1)&~(uintptr_t)(alignment-1));
}

/* 0x00123e20: makes a checkpoint: writes the state of the game to the file
that was not written last */
void game_state_save(
	void)
{
	long index;

	for (index= 0; index<NUMBER_OF_GAME_STATE_BEFORE_SAVE_PROCS; index++)
	{
		game_state_before_save_procs[index](0);
	}

	game_state_globals.save_time= game_time_globals->time;
	game_state_globals.deaths_after_save= 0;
	game_state_globals.previous_saved_game_valid= game_state_globals.saved_game_valid;
	game_state_globals.file_index= (short)((game_state_globals.file_index+1)%NUMBER_OF_GAME_STATE_FILES);
	game_state_write_to_file(game_state_globals.file_index);
	game_state_globals.saved_game_valid= TRUE;
	memcpy(&game_state_globals.headers[game_state_globals.file_index], game_state_globals.header,
		sizeof(struct game_state_header));

	game_state_after_save_procs[0](0);

	unknown_004e6398[0]= unknown_00485ab0[0];
	unknown_004e6398[1]= unknown_00485ab0[1];
}

/* 0x00123ed0: the game was lost. If that happens soon after a checkpoint,
several times in a row, the checkpoint is no good: the game goes back to the
one before it. The name is a guess */
void game_state_note_death(
	void)
{
	if (game_state_globals.saved_game_valid && game_state_globals.previous_saved_game_valid)
	{
		unsigned long ticks= game_time_globals->time-game_state_globals.save_time;

		/* a product of the x87 registers, in the precision that the game
		sets them to */
		if ((long double)GAME_STATE_DEATH_AFTER_SAVE_SECONDS>
			(long double)ticks*(long double)game_time_globals->seconds_per_tick)
		{
			game_state_globals.deaths_after_save++;
			if (game_state_globals.deaths_after_save>=MAXIMUM_GAME_STATE_DEATHS_AFTER_SAVE)
			{
				game_state_globals.file_index=
					(short)((game_state_globals.file_index+1)%NUMBER_OF_GAME_STATE_FILES);
				game_state_globals.saved_game_valid= TRUE;
				game_state_globals.previous_saved_game_valid= FALSE;
			}
			return;
		}
	}

	game_state_globals.deaths_after_save= 0;
}

/* 0x00199520 */
void game_state_call_before_load_procs(
	unsigned long flags)
{
	long index;

	for (index= 0; index<NUMBER_OF_GAME_STATE_BEFORE_LOAD_PROCS; index++)
	{
		game_state_before_load_procs[index](flags);
	}
}

/* 0x00199540 */
void game_state_call_after_load_procs(
	unsigned long flags)
{
	long index;

	for (index= 0; index<NUMBER_OF_GAME_STATE_AFTER_LOAD_PROCS; index++)
	{
		game_state_after_load_procs[index](flags);
	}
}

/* 0x00123f60: goes back to the last checkpoint. If there is none, or it is
not of the game that is being played, asks for the map to be started again.
The state is read until the one read is the one of the checkpoint */
void game_state_revert(
	unsigned long flags)
{
	if (game_state_globals.saved_game_valid && game_state_file_valid_for_read(game_state_globals.file_index))
	{
		struct game_state_header saved_header;

		memcpy(&saved_header, &game_state_globals.headers[game_state_globals.file_index], sizeof(saved_header));
		if (game_state_header_valid(&saved_header))
		{
			saved_header.options.reverted= TRUE;
			game_state_globals.header->options.reverted= TRUE;
			if (main_game_globals &&
				main_game_globals->active &&
				game_options_match(&main_game_globals->options, &saved_header.options) &&
				scenario_switch_structure_bsp(saved_header.structure_bsp_index, TRUE) &&
				game_state_headers_match(&saved_header, game_state_globals.header))
			{
				unsigned long load_flags= (flags&1)<<_game_state_load_flag5_bit;

				if ((flags>>1)&1)
				{
					load_flags|= FLAG(_game_state_load_flag6_bit);
				}

				game_state_call_before_load_procs(load_flags);
				do
				{
					game_state_read_from_file(game_state_globals.file_index);
				}
				while (!game_state_headers_match(&saved_header, game_state_globals.header));
				game_state_call_after_load_procs(load_flags);

				return;
			}
		}
	}

	main_reset_map_requested= TRUE;
}

/* 0x00124520: whether a header is of a state of the game that this game
made, with the memory where it is now */
boolean game_state_header_valid(
	const struct game_state_header *header)
{
	return
		cache_file_build_verify(header->build) &&
		header->allocation_size_checksum==game_state_globals.allocation_size_checksum &&
		header->base_address==game_state_globals.base_address &&
		strlen(header->map_name)!=0 &&
		strchr(header->map_name, '\\')!=NULL &&
		game_options_verify(&header->options);
}

/* 0x00124590: whether a header is good and is of the same game, in the same
map, as another. The name is a guess */
boolean game_state_headers_match(
	const struct game_state_header *header,
	const struct game_state_header *other)
{
	return
		game_state_header_valid(header) &&
		strcmp(other->map_name, header->map_name)==0 &&
		other->cache_file_checksum==header->cache_file_checksum &&
		game_options_match(&other->options, &header->options) &&
		other->structure_bsp_index==header->structure_bsp_index;
}

/* 0x00124620: one of the procs after a state is read. As in Halo 1 */
void GAME_STATE_PROC game_state_set_revert_time(
	unsigned long flags)
{
	(void)flags;

	game_state_globals.revert_time= (long)game_time_globals->time;
	game_time_globals->paused= FALSE;
}

/* 0x00124700: the function of game_state_allocation that takes memory */
void *ALLOCATION_CONVENTION game_state_allocation_allocate(
	struct allocation *allocation,
	unsigned long size)
{
	(void)allocation;

	return game_state_malloc((long)size);
}

/* 0x00124750: one of the procs after a state is read: the state that was
read has game_state_allocation in it, with the address that its functions
had when it was written. The name is a guess */
void GAME_STATE_PROC game_state_allocation_restore(
	unsigned long flags)
{
	(void)flags;

	if (game_state_allocation)
	{
		game_state_allocation->functions= &game_state_allocation_functions;
	}
}
