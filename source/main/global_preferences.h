/*
GLOBAL_PREFERENCES.H

What the game remembers from one run to the next, apart from the players'
profiles: a file on the hard disk, signed so that the game can tell that it
wrote it. Worked out from halo2ship.exe; Halo 1 has nothing like it, and the
names are guesses.
*/

#ifndef __GLOBAL_PREFERENCES_H
#define __GLOBAL_PREFERENCES_H

#include "cseries/cseries.h"
#include "text/unicode.h"

#include <stdint.h>

/* ---------- constants */

enum
{
	GLOBAL_PREFERENCES_VERSION= 8,
	GLOBAL_PREFERENCES_NAME_COUNT= 4,
	GLOBAL_PREFERENCES_NAME_LENGTH= 31,
	GLOBAL_PREFERENCES_SIGNATURE_SIZE= 20,

	/* of the tasks of the file */
	GLOBAL_PREFERENCES_TASK_CATEGORY= 8
};

/* ---------- structures */

struct global_preferences_data
{
	long version;
	/* the language of the copies of the maps on the hard disk; NONE if
	there are none. From 0 to 8, as the next one */
	long language;
	long unknown8;
	/* what the profiles have as unknown5C, as it was set last */
	long unknownC;
	byte unknown10[8];
	wchar names[GLOBAL_PREFERENCES_NAME_COUNT][GLOBAL_PREFERENCES_NAME_LENGTH+1];
	wchar name[GLOBAL_PREFERENCES_NAME_LENGTH+1];
	byte unknown158[4];
	/* from 0 to 15 */
	long unknown15C;
	/* from 0 to 3, as the next one */
	long unknown160;
	long unknown164;
	/* a block that is cleared if its first value or its tenth is not from 0
	to 8 */
	long unknown168[27];
	/* the build of the game that made the copies of the maps on the hard
	disk; 0 makes the game copy them again */
	long cache_build_number;
	byte unknown1D8[0x10];
};

/* what the file has */
struct global_preferences_file
{
	/* of the data, made by the console */
	unsigned long signature[GLOBAL_PREFERENCES_SIGNATURE_SIZE/4];
	struct global_preferences_data data;
};

/* 0x00510818 */
struct global_preferences_globals
{
	boolean initialized;
	/* something changed: the file has to be written */
	boolean dirty;
	byte pad2[2];
	uint32_t file;
	struct global_preferences_file current;
	/* what was written last */
	struct global_preferences_file written;
	/* of the task that writes */
	long task_id;
	/* the task of the file that was started last is done */
	boolean done;
	byte pad405[3];
};

typedef char verify_global_preferences_data_size[
	sizeof(struct global_preferences_data) == 0x1E8 ? 1 : -1];
typedef char verify_global_preferences_data_cache_build_number_offset[
	offsetof(struct global_preferences_data, cache_build_number) == 0x1D4 ? 1 : -1];
typedef char verify_global_preferences_file_size[
	sizeof(struct global_preferences_file) == 0x1FC ? 1 : -1];
typedef char verify_global_preferences_globals_size[
	sizeof(struct global_preferences_globals) == 0x408 ? 1 : -1];
typedef char verify_global_preferences_globals_done_offset[
	offsetof(struct global_preferences_globals, done) == 0x404 ? 1 : -1];

/* ---------- prototypes/GLOBAL_PREFERENCES.C */

struct game_variant;

boolean global_preferences_find_player_profile(long index, long *file);
long __attribute__((stdcall)) global_preferences_get_game_variant(struct game_variant *variant);

void global_preferences_initialize(void);
void global_preferences_reset(void);
void global_preferences_validate(struct global_preferences_file *file);
void global_preferences_flush(void);
void global_preferences_flush_blocking(void);
void global_preferences_set_name(long index, const wchar *name);
void global_preferences_set_other_name(const wchar *name);
void global_preferences_set_unknown15C(long value);
void global_preferences_set_unknown160(long value);
void global_preferences_get_unknown160(long *value);
void global_preferences_set_unknown164(long value);
void global_preferences_get_unknown164(long *value);
boolean xbox_signature_end(unsigned long *signature);

/* ---------- globals */

extern struct global_preferences_globals global_preferences_globals;
extern const char *global_preferences_path;
extern uint32_t xbox_signature_handle;

/* ---------- inline code */

/* a file of a font was found to be bad: the preferences forget which fonts
are on the hard disk, and are written at once. The game has this in each
function that finds one */
static inline __attribute__((always_inline)) void global_preferences_forget_fonts(
	void)
{
	if (global_preferences_globals.current.data.unknown8!=NONE)
	{
		global_preferences_globals.current.data.unknown8= NONE;
		global_preferences_globals.dirty= TRUE;
		global_preferences_flush_blocking();
	}
}

#endif
