/*
GLOBAL_PREFERENCES.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

Not here yet: 0x00120e20 and 0x00120e70, which read two of the names.
*/

#include "global_preferences.h"

#include "cseries/async.h"
#include "cache/cache_files.h"
#include "game/game.h"
#include "saved games/player_profile.h"
#include "scenario/scenario.h"
#include "tag_files/tag_groups.h"
#include "text/unicode_string_list.h"

#include <string.h>

/* ---------- prototypes of functions of the console's libraries */

/* 0x002d6765, 0x002d6779 and 0x002d67c7: the console's signature of data
that a game keeps: only this console gives the same one for the same data */
uint32_t XCalculateSignatureBegin(uint32_t flags);
uint32_t XCalculateSignatureUpdate(uint32_t signature, const void *data, uint32_t size);
uint32_t XCalculateSignatureEnd(uint32_t signature, void *result);

/* 0x003207ed: of the game's C library, with characters of 16 bits */
wchar *wcsncpy(wchar *destination, const wchar *source, size_t count);

/* ---------- prototypes of functions that are not decompiled */

/* 0x00212c20: finds the game variant of a name, and its file; whether there
is one */
boolean unknown_00212c20(const wchar *name, struct game_variant *variant, long *file);

/* ---------- constants */

/* the default game variant: its name is the text of this id, in the list
that is at this place of the first element of the tag of the game globals */
#define DEFAULT_VARIANT_NAME_STRING_ID 0x0E000780
#define DEFAULT_VARIANT_STRING_LIST_INDEX 7
#define DEFAULT_VARIANT_NAME_SIZE 258

#define GENERIC_READ 0x80000000
#define GENERIC_WRITE 0x40000000
#define OPEN_ALWAYS 4
#define FILE_FLAG_SEQUENTIAL_SCAN 0x08000000
#define INVALID_HANDLE_VALUE 0xFFFFFFFF

enum
{
	GLOBAL_PREFERENCES_TASK_PRIORITY= 6,
	GLOBAL_PREFERENCES_WRITE_PRIORITY= 2,
	/* of async_write_position: flush the file when it is written */
	GLOBAL_PREFERENCES_WRITE_FLAGS= 1,

	MAXIMUM_GLOBAL_PREFERENCES_LANGUAGE= 9,
	MAXIMUM_GLOBAL_PREFERENCES_UNKNOWN15C= 15,
	MAXIMUM_GLOBAL_PREFERENCES_UNKNOWN160= 3,
	MAXIMUM_GLOBAL_PREFERENCES_UNKNOWN168= 8
};

/* ---------- globals */

/* 0x00510818 */
struct global_preferences_globals global_preferences_globals;

/* 0x004687ec */
const char *global_preferences_path= "z:\\preferences.dat";

/* 0x00470024: the signature that is being made; NONE if none is */
uint32_t xbox_signature_handle= 0xFFFFFFFF;

/* ---------- code */

/* 0x00120e20: finds the profile of one of the names that the preferences
keep; whether there is one, and its file. The name is a guess */
boolean global_preferences_find_player_profile(
	long index,
	long *file)
{
	return player_profile_find(NULL, global_preferences_globals.current.data.names[index], file);
}

/* the text that is the name of the default game variant, or nothing */
static inline __attribute__((always_inline)) void global_preferences_get_default_variant_name(
	wchar *name)
{
	long tag_index;

	name[0]= 0;
	if (global_game_globals && (tag_index= global_game_globals->unknown16C)!=NONE)
	{
		const struct tag_block *block=
			(const struct tag_block *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;
		long string_list_index= ((const long *)(uintptr_t)block->address)[DEFAULT_VARIANT_STRING_LIST_INDEX];

		if (string_list_index!=NONE)
		{
			unicode_string_list_get_string(string_list_index, DEFAULT_VARIANT_NAME_STRING_ID, name);
		}
	}
}

/* 0x00120e70: the game variant of the name that the preferences keep, and
its file. If there is none, the default variant (its name is a text of the
game's) and NONE. The name is a guess */
long __attribute__((stdcall)) global_preferences_get_game_variant(
	struct game_variant *variant)
{
	struct game_variant result;
	wchar name[DEFAULT_VARIANT_NAME_SIZE];
	long file= NONE;

	if (unknown_00212c20(global_preferences_globals.current.data.name, variant, &file))
	{
		return file;
	}

	memset(&result, 0, sizeof(result));
	global_preferences_get_default_variant_name(name);
	wcsncpy(result.name, name, GAME_VARIANT_NAME_LENGTH);
	result.unknown48= (result.unknown48&0xFFFF8F9A)|0xF9A;
	result.unknown74= 16;
	result.unknown78= 16;
	result.unknownF0= (result.unknownF0&~5UL)|2;
	result.unknown44= 2;
	result.unknownA8= 2;
	result.unknown80= 5;
	result.unknown84= 5;
	result.name[GAME_VARIANT_NAME_LENGTH]= 0;
	result.unknown3= 0xFF;
	result.unknown4C= 0;
	result.unknown50= 25;
	result.unknown54= 480;
	result.unknown58= 0;
	result.unknown7C= 0;
	result.unknown88= 0;
	result.unknownA4= 0;
	result.unknownAC= 10;
	result.unknownB0= 0;
	memset(result.unknownCC, 0, sizeof(result.unknownCC));
	*variant= result;

	return NONE;
}

static inline __attribute__((always_inline)) void global_preferences_wait(
	void)
{
	while (!*(volatile boolean *)&global_preferences_globals.done)
	{
		SwitchToThread();
	}
}

static inline __attribute__((always_inline)) long global_preferences_clamp(
	long value,
	long maximum)
{
	if (value<0)
	{
		return 0;
	}
	if (value>maximum)
	{
		return maximum;
	}

	return value;
}

/* 0x00213380: ends the signature that is being made; whether it was made.
Where this function goes is a guess */
boolean xbox_signature_end(
	unsigned long *signature)
{
	uint32_t result= XCalculateSignatureEnd(xbox_signature_handle, signature);

	xbox_signature_handle= 0xFFFFFFFF;

	return result==0;
}

static inline __attribute__((always_inline)) boolean global_preferences_sign(
	const struct global_preferences_data *data,
	unsigned long *signature)
{
	boolean result= FALSE;

	xbox_signature_handle= XCalculateSignatureBegin(1);
	if (xbox_signature_handle!=0xFFFFFFFF &&
		XCalculateSignatureUpdate(xbox_signature_handle, data, sizeof(*data))==0)
	{
		result= xbox_signature_end(signature);
	}

	return result;
}

/* 0x00121220: the preferences of a console that has none */
void global_preferences_reset(
	void)
{
	struct global_preferences_globals *globals= &global_preferences_globals;

	memset(&globals->current, 0, sizeof(globals->current));
	globals->current.data.language= NONE;
	globals->current.data.unknown8= NONE;
	globals->current.data.version= GLOBAL_PREFERENCES_VERSION;
	globals->current.data.cache_build_number= 0;
	globals->current.data.unknown160= 1;
	globals->current.data.unknown164= 1;
	memset(&globals->written.data, 0, sizeof(globals->written.data));
	globals->dirty= TRUE;
}

/* 0x00121280: puts in their ranges the values of preferences that were
read. The ends of the names are put in the preferences that the game has,
not in those that it is given */
void global_preferences_validate(
	struct global_preferences_file *file)
{
	struct global_preferences_data *data= &file->data;
	long index;

	data->name[GLOBAL_PREFERENCES_NAME_LENGTH]= 0;
	for (index= 0; index<GLOBAL_PREFERENCES_NAME_COUNT; index++)
	{
		global_preferences_globals.current.data.names[index][GLOBAL_PREFERENCES_NAME_LENGTH]= 0;
	}

	if (data->language<0 || data->language>=MAXIMUM_GLOBAL_PREFERENCES_LANGUAGE)
	{
		data->language= NONE;
	}
	if (data->unknown8<0 || data->unknown8>=MAXIMUM_GLOBAL_PREFERENCES_LANGUAGE)
	{
		data->unknown8= NONE;
	}
	data->unknown15C= global_preferences_clamp(data->unknown15C, MAXIMUM_GLOBAL_PREFERENCES_UNKNOWN15C);
	data->unknown160= global_preferences_clamp(data->unknown160, MAXIMUM_GLOBAL_PREFERENCES_UNKNOWN160);
	data->unknown164= global_preferences_clamp(data->unknown164, MAXIMUM_GLOBAL_PREFERENCES_UNKNOWN160);
	if (data->unknown168[0]<0 || data->unknown168[0]>MAXIMUM_GLOBAL_PREFERENCES_UNKNOWN168 ||
		data->unknown168[9]<0 || data->unknown168[9]>MAXIMUM_GLOBAL_PREFERENCES_UNKNOWN168)
	{
		memset(data->unknown168, 0, sizeof(data->unknown168));
	}
}

/* 0x00121120: if the preferences changed, signs them and starts to write
them. Not while the last task of the file is still going */
void global_preferences_flush(
	void)
{
	struct global_preferences_globals *globals= &global_preferences_globals;

	if (globals->dirty && globals->done)
	{
		if (memcmp(&globals->current.data, &globals->written.data, sizeof(globals->current.data))!=0)
		{
			unsigned long signature[GLOBAL_PREFERENCES_SIGNATURE_SIZE/4];
			boolean made;

			memcpy(&globals->written.data, &globals->current.data, sizeof(globals->written.data));
			made= global_preferences_sign(&globals->written.data, signature);
			if (globals->file!=INVALID_HANDLE_VALUE && made)
			{
				memcpy(globals->written.signature, signature, sizeof(globals->written.signature));
				globals->task_id= async_write_position(globals->file, &globals->written, sizeof(globals->written), 0,
					GLOBAL_PREFERENCES_WRITE_FLAGS, GLOBAL_PREFERENCES_TASK_CATEGORY,
					GLOBAL_PREFERENCES_WRITE_PRIORITY, NULL, &globals->done);
			}
		}
		globals->dirty= FALSE;
	}
}

/* 0x00120d80: writes the preferences if they changed, and waits until they
are on the disk */
void global_preferences_flush_blocking(
	void)
{
	if (global_preferences_globals.file!=INVALID_HANDLE_VALUE)
	{
		global_preferences_wait();
		if (global_preferences_globals.dirty)
		{
			global_preferences_flush();
		}
		global_preferences_wait();
		async_flush_file_blocking(GLOBAL_PREFERENCES_TASK_CATEGORY, global_preferences_globals.file);
	}
}

/* 0x00121350: opens the file and reads the preferences. If the file is not
there, or what it has was not signed by this console, or is of another
version, the preferences are those of a console that has none, and the file
is made anew */
void global_preferences_initialize(
	void)
{
	struct global_preferences_globals *globals= &global_preferences_globals;
	union
	{
		struct async_create_file_parameters create;
		uint32_t parameters[ASYNC_TASK_PARAMETER_COUNT];
	} task;

	if (globals->initialized)
	{
		return;
	}
	globals->initialized= TRUE;

	memset(&task, 0, sizeof(task));
	task.create.path= global_preferences_path;
	task.create.access= GENERIC_READ|GENERIC_WRITE;
	task.create.share_mode= 0;
	task.create.creation_disposition= OPEN_ALWAYS;
	task.create.flags= FILE_FLAG_SEQUENTIAL_SCAN;
	task.create.file= &globals->file;
	task.create.create_directories= FALSE;
	async_task_add(GLOBAL_PREFERENCES_TASK_PRIORITY, task.parameters, GLOBAL_PREFERENCES_TASK_CATEGORY,
		async_create_file_work, &globals->done);
	global_preferences_wait();

	if (globals->file!=INVALID_HANDLE_VALUE)
	{
		struct global_preferences_file file;
		unsigned long signature[GLOBAL_PREFERENCES_SIGNATURE_SIZE/4];
		long bytes_read;
		boolean made;

		async_read_position(globals->file, sizeof(file), 0, GLOBAL_PREFERENCES_TASK_CATEGORY,
			GLOBAL_PREFERENCES_TASK_PRIORITY, &bytes_read, &file, &globals->done);
		global_preferences_wait();

		made= global_preferences_sign(&file.data, signature);
		if (file.data.version==GLOBAL_PREFERENCES_VERSION &&
			made && memcmp(signature, file.signature, sizeof(signature))==0 &&
			bytes_read==sizeof(file))
		{
			global_preferences_validate(&file);
			globals->done= TRUE;
			memcpy(&globals->current, &file, sizeof(globals->current));
			memcpy(&globals->written.data, &globals->current.data, sizeof(globals->written.data));
			return;
		}
	}

	global_preferences_reset();
	if (globals->file!=INVALID_HANDLE_VALUE)
	{
		global_preferences_flush_blocking();
		globals->task_id= async_set_file_size(globals->file, sizeof(struct global_preferences_file),
			GLOBAL_PREFERENCES_TASK_CATEGORY, GLOBAL_PREFERENCES_WRITE_PRIORITY, NULL, &globals->done);
	}
}

/* 0x00120df0: one of the four names */
void global_preferences_set_name(
	long index,
	const wchar *name)
{
	wchar *destination= global_preferences_globals.current.data.names[index];

	wcsncpy(destination, name, GLOBAL_PREFERENCES_NAME_LENGTH);
	destination[GLOBAL_PREFERENCES_NAME_LENGTH]= 0;
	global_preferences_globals.dirty= TRUE;
}

/* 0x00120e40: the other name */
void global_preferences_set_other_name(
	const wchar *name)
{
	wchar *destination= global_preferences_globals.current.data.name;

	wcsncpy(destination, name, GLOBAL_PREFERENCES_NAME_LENGTH);
	destination[GLOBAL_PREFERENCES_NAME_LENGTH]= 0;
	global_preferences_globals.dirty= TRUE;
}

/* 0x00121040 */
void global_preferences_set_unknown15C(
	long value)
{
	if (value!=NONE)
	{
		global_preferences_globals.current.data.unknown15C= value;
		global_preferences_globals.dirty= TRUE;
	}
}

/* 0x00121060 */
void global_preferences_set_unknown160(
	long value)
{
	global_preferences_globals.current.data.unknown160=
		global_preferences_clamp(value, MAXIMUM_GLOBAL_PREFERENCES_UNKNOWN160);
	global_preferences_globals.dirty= TRUE;
}

/* 0x001210a0 */
void global_preferences_get_unknown160(
	long *value)
{
	*value= global_preferences_clamp(global_preferences_globals.current.data.unknown160,
		MAXIMUM_GLOBAL_PREFERENCES_UNKNOWN160);
}

/* 0x001210c0 */
void global_preferences_set_unknown164(
	long value)
{
	global_preferences_globals.current.data.unknown164=
		global_preferences_clamp(value, MAXIMUM_GLOBAL_PREFERENCES_UNKNOWN160);
	global_preferences_globals.dirty= TRUE;
}

/* 0x00121100 */
void global_preferences_get_unknown164(
	long *value)
{
	*value= global_preferences_clamp(global_preferences_globals.current.data.unknown164,
		MAXIMUM_GLOBAL_PREFERENCES_UNKNOWN160);
}
