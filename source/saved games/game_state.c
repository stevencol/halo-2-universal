/*
GAME_STATE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The names are those of Halo 1's game_state.c where it has them. Not here yet:
the functions that keep a state of the game in a core file or in a saved
game (0x001240b0 to 0x00124490, 0x00124640, 0x00124770 on).
*/

#include "game_state.h"

#include "networking/network_life_cycle.h"
#include "cache/cache_files.h"
#include "game/game.h"
#include "main/main.h"
#include "memory/crc.h"
#include "memory/physical_memory.h"
#include "scenario/scenario.h"
#include "sound/sound_definitions.h"

#include <string.h>
#include "cseries/async.h"
#include "main/main_game.h"
#include "scenario/scenario.h"
#include "tag_files/files.h"

/* ---------- prototypes of functions not decompiled yet: the states in files */

enum
{
	GAME_STATE_HEADER_SIZE= sizeof(struct game_state_header),
	GAME_STATE_SIZE= GAME_STATE_CPU_SIZE+GAME_STATE_GPU_SIZE,
	GAME_STATE_CORE_PATH_SIZE= 1024,
	/* what the functions that are called around a load are told */
	GAME_STATE_LOAD_FLAGS= 0xC
};

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
void GAME_STATE_PROC havok_before_save_or_load(unsigned long flags);
void GAME_STATE_PROC simulation_before_save(unsigned long flags); /* of simulation.c */
void GAME_STATE_PROC unknown_0016eff0(unsigned long flags);
void GAME_STATE_PROC game_unknown_001e6a40_after_load(unsigned long flags);
void GAME_STATE_PROC havok_after_save_or_load(unsigned long flags);
void GAME_STATE_PROC unknown_001889d0(unsigned long flags);
void GAME_STATE_PROC unknown_00155f10(unsigned long flags);
void GAME_STATE_PROC simulation_before_load(unsigned long flags);
void GAME_STATE_PROC unknown_00188ac0(unsigned long flags);
void GAME_STATE_PROC game_unknown_0016efb0_before_load(unsigned long flags);
void GAME_STATE_PROC unknown_0014b660(unsigned long flags);
void GAME_STATE_PROC simulation_after_load(unsigned long flags);
void GAME_STATE_PROC unknown_00043970(unsigned long flags);
void GAME_STATE_PROC objects_unknown_0003d380(unsigned long flags);
void GAME_STATE_PROC game_unknown_001552e0_after_load(unsigned long flags);
void GAME_STATE_PROC unknown_0024c829(unsigned long flags);
void GAME_STATE_PROC unknown_001c7f80(unsigned long flags);
void GAME_STATE_PROC unknown_0017d190(unsigned long flags);
void GAME_STATE_PROC game_unknown_0016efb0_after_load(unsigned long flags);
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
	havok_before_save_or_load,
	simulation_before_save,
	unknown_0016eff0,
	game_unknown_001e6a40_after_load
};

/* 0x0046e330 */
game_state_proc game_state_after_save_procs[NUMBER_OF_GAME_STATE_AFTER_SAVE_PROCS]=
{
	havok_after_save_or_load
};

/* 0x0046e334 */
game_state_proc game_state_before_load_procs[NUMBER_OF_GAME_STATE_BEFORE_LOAD_PROCS]=
{
	havok_before_save_or_load,
	game_state_buffer_protect,
	unknown_001889d0,
	unknown_00155f10,
	simulation_before_load
};

/* 0x0046e348 */
game_state_proc game_state_after_load_procs[NUMBER_OF_GAME_STATE_AFTER_LOAD_PROCS]=
{
	game_state_allocation_restore,
	(game_state_proc)no_operation1,
	sound_reset,
	unknown_00188ac0,
	game_unknown_0016efb0_before_load,
	unknown_0014b660,
	simulation_after_load,
	unknown_00043970,
	objects_unknown_0003d380,
	(game_state_proc)no_operation1,
	(game_state_proc)no_operation1,
	(game_state_proc)no_operation1,
	game_state_set_revert_time,
	game_unknown_001552e0_after_load,
	unknown_0024c829,
	havok_after_save_or_load,
	unknown_001c7f80,
	unknown_0017d190,
	game_unknown_0016efb0_after_load,
	unknown_00226030
};

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

	/* the time that writing took is not a part of the next frame */
	main_time_skip();
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
			saved_header.options.simulation= _game_simulation_local;
			game_state_globals.header->options.simulation= _game_simulation_local;
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

	main_globals.reset_map= TRUE;
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

/* ---------- the states that are kept in files */

/* a state that is loaded from a file is of a game on this console alone,
whatever it was when it was saved; so is the one in memory. The game has
this in the two functions that load one */
static inline __attribute__((always_inline)) void game_state_header_make_local(
	struct game_state_header *header)
{
	header->options.simulation= _game_simulation_local;
	game_state_globals.header->options.simulation= _game_simulation_local;
}

/* the structure bsp of a header, as the game gives it to
scenario_switch_structure_bsp: with the two bytes after it */
static inline __attribute__((always_inline)) long game_state_header_structure_bsp_index(
	const struct game_state_header *header)
{
	long structure_bsp_index;

	memcpy(&structure_bsp_index, &header->structure_bsp_index, sizeof(structure_bsp_index));

	return structure_bsp_index;
}

/* waits for the file of a saved game to be read or written */
static inline __attribute__((always_inline)) void game_state_saved_game_wait(
	const struct saved_game_file_result *result)
{
	while (!result->done)
	{
		SwitchToThread();
		sound_idle();
	}
}

/* 0x001240b0: loads the state that a saved game of the hard disk has, if it
is of the game that is on: the map is not changed for it. The state becomes
the one to go back to */
void game_state_load_saved_game(
	long saved_game)
{
	struct game_state_header header;

	if (game_state_saved_game_read_header(GAME_STATE_HEADER_SIZE, GAME_STATE_SIZE, saved_game, &header) &&
		game_state_header_valid(&header))
	{
		network_life_cycle_end();
		game_state_header_make_local(&header);
		if (main_game_globals && main_game_globals->active &&
			game_options_match(&main_game_globals->options, &header.options) &&
			scenario_switch_structure_bsp(game_state_header_structure_bsp_index(&header), TRUE) &&
			game_state_headers_match(&header, game_state_globals.header))
		{
			game_state_call_before_load_procs(GAME_STATE_LOAD_FLAGS);
			game_state_saved_game_read_data(GAME_STATE_SIZE, game_state_globals.base_address, saved_game);
			game_state_call_after_load_procs(GAME_STATE_LOAD_FLAGS);
			game_state_save();
			game_state_globals.unknown2538= TRUE;
		}
	}
}

/* 0x00124360: starts to read the header of the state of a saved game; the
caller waits for it */
boolean game_state_saved_game_read_header_async(
	void *header,
	struct saved_game_file_result *result,
	long saved_game)
{
	return saved_game_auxiliary_read(header, GAME_STATE_HEADER_SIZE, NULL, GAME_STATE_SIZE-GAME_STATE_HEADER_SIZE,
		result, saved_game);
}

/* 0x00124380: loads a state that was written to a file of the disc of the
game: a "core", which is what a developer kept of a game to go back to. If
it is of another game than the one that is on, that game is started first */
void game_state_load_core(
	const char *name)
{
	struct game_state_header header;
	boolean changed= FALSE;

	if (!game_state_core_read(name, GAME_STATE_HEADER_SIZE, &header) || !game_state_header_valid(&header))
	{
		return;
	}
	network_life_cycle_end();
	game_state_header_make_local(&header);
	if (!main_game_globals || !main_game_globals->active ||
		!game_options_match(&main_game_globals->options, &header.options))
	{
		changed= TRUE;
		if (!main_game_change_immediate(&header.options))
		{
			main_game_goto_main_menu(0);

			return;
		}
	}
	if (scenario_switch_structure_bsp(game_state_header_structure_bsp_index(&header), TRUE) &&
		game_state_headers_match(&header, game_state_globals.header))
	{
		game_state_call_before_load_procs(GAME_STATE_LOAD_FLAGS);
		game_state_core_read(name, GAME_STATE_SIZE, game_state_globals.base_address);
		game_state_call_after_load_procs(GAME_STATE_LOAD_FLAGS);
	}
	else if (changed)
	{
		main_game_goto_main_menu(0);
	}
}

/* 0x00124490: the file of a core: in the directory of the cores of the disc
of the game, unless the name is a whole path */
void game_state_core_file_reference(
	const char *name,
	struct file_reference *file)
{
	char path[GAME_STATE_CORE_PATH_SIZE];
	char first= name[0];

	path[0]= 0;
	if (((first>='a' && first<='z') || (first>='A' && first<='Z')) && name[1]==':' && name[2]=='\\')
	{
		csprintf(path, "%s.bin", name);
	}
	else
	{
		csprintf(path, "d:\\core\\%s.bin", name);
	}

	/* in Halo 1, file_reference_create and file_reference_set_name */
	memset(file, 0, sizeof(*file));
	file->signature= FILE_REFERENCE_SIGNATURE;
	file->location= NONE;
	if (TEST_FLAG(file->flags, _has_filename_bit))
	{
		path_remove_last_element(file->path);
	}
	file_path_add_name(file->path, path);
	file->flags|= FLAG(_has_filename_bit);
}

/* 0x00124640: reads the start of the file of a core; whether all that was
asked for was read */
boolean game_state_core_read(
	const char *name,
	unsigned long size,
	void *buffer)
{
	struct file_reference file;
	boolean success= FALSE;
	long error;

	game_state_core_file_reference(name, &file);
	if (file_open(&file, FLAG(_permission_read_bit), &error))
	{
		success= file_read(&file, size, buffer, FALSE);
		file_close(&file);
	}

	return success;
}

/* 0x00124770: makes the file of the state of a saved game, with nothing in
it yet */
boolean game_state_saved_game_create(
	long saved_game)
{
	return game_state_saved_game_write(NULL, GAME_STATE_HEADER_SIZE, GAME_STATE_SIZE, saved_game);
}

/* 0x00124790: reads the header of the state of a saved game, and waits for
it; whether it was read */
boolean game_state_saved_game_read_header(
	long header_size,
	long size,
	long saved_game,
	void *header)
{
	struct saved_game_file_result result;

	if (!saved_game_auxiliary_read(header, header_size, NULL, size-header_size, &result, saved_game))
	{
		return FALSE;
	}
	game_state_saved_game_wait(&result);

	return result.success;
}

/* 0x00124800: reads the rest of the state of a saved game, and waits for it */
void game_state_saved_game_read_data(
	long size,
	void *data,
	long saved_game)
{
	struct saved_game_file_result result;

	if (saved_game_auxiliary_read(NULL, 0, data, size, &result, saved_game))
	{
		game_state_saved_game_wait(&result);
	}
}

/* 0x00124840: writes a state to the file of a saved game, and waits for it;
whether it was written */
boolean game_state_saved_game_write(
	void *state,
	long header_size,
	long size,
	long saved_game)
{
	struct saved_game_file_result result;
	void *data= state ? (byte *)state+header_size : NULL;

	if (!saved_game_auxiliary_write(state, header_size, data, size-header_size, &result, saved_game))
	{
		return FALSE;
	}
	game_state_saved_game_wait(&result);

	return result.success;
}
