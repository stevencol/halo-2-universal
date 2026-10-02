/*
UNICODE_STRING_LIST.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The texts of the game in the language of the console. The names of the file
and of the functions are guesses.
*/

#include "unicode_string_list.h"

#include "cache/cache_files.h"
#include "memory/physical_memory.h"
#include "scenario/scenario.h"

#include <stddef.h>

/* ---------- constants */

enum
{
	/* what cache_file_blocking_read is told first. What is read from the map
	is read in whole sectors of the disc (CACHE_FILE_SECTOR_SIZE) */
	_cache_file_read_unknown_none= NONE
};

/* ---------- prototypes of functions of the console's libraries */

/* 0x0033039a */
uint32_t XGetLanguage(void);

/* ---------- prototypes of functions of the C library */

/* 0x003207ed: of characters of 16 bits */
wchar *wcsncpy(wchar *destination, const wchar *source, size_t count);

/* ---------- code */

/* the texts of the language of the console, which is asked for the first
time. The game has this wherever it gets a text */
static inline __attribute__((always_inline)) long unicode_string_list_language(
	void)
{
	if (global_language==NONE)
	{
		global_language= language_from_xbox_language((long)XGetLanguage());
	}

	return global_language;
}

static inline __attribute__((always_inline)) struct language_texts *language_texts_get_current(
	long language)
{
	return (struct language_texts *)((byte *)global_game_globals+GAME_GLOBALS_LANGUAGE_TEXTS_OFFSET)+language;
}

static inline __attribute__((always_inline)) const struct unicode_string_list *unicode_string_list_get(
	long string_list_index)
{
	return (const struct unicode_string_list *)(uintptr_t)global_tag_instances[string_list_index&0xFFFF].address;
}

/* whether the texts of a language have one at a place */
static inline __attribute__((always_inline)) boolean language_texts_has_index(
	const struct language_texts *texts,
	long index)
{
	return index>=0 && index<=(long)((unsigned long)texts->count-1);
}

static inline __attribute__((always_inline)) uint32_t cache_file_round_to_sector(
	uint32_t size)
{
	return (size&(CACHE_FILE_SECTOR_SIZE-1)) ? (size|(CACHE_FILE_SECTOR_SIZE-1))+1 : size;
}

/* 0x001a0120: whether a tag has a text, in the language of the console */
boolean unicode_string_list_has_string(
	long string_list_index,
	unsigned long string_id)
{
	const struct unicode_string_list *list;
	long language;

	if (string_list_index==NONE)
	{
		return FALSE;
	}
	language= unicode_string_list_language();
	list= unicode_string_list_get(string_list_index);

	return language_texts_has(language_texts_get_current(language), list->ranges[language].first, string_id,
		list->ranges[language].count);
}

/* 0x001a0180: a text of a tag, in the language of the console. Nothing is
written if there is no tag */
void unicode_string_list_get_string(
	long string_list_index,
	unsigned long string_id,
	wchar *string)
{
	if (string_list_index!=NONE)
	{
		long language= unicode_string_list_language();
		const struct unicode_string_list *list= unicode_string_list_get(string_list_index);

		language_texts_get(language_texts_get_current(language), string_id, list->ranges[language].count, string,
			list->ranges[language].first);
	}
}

/* 0x001a01e0: reads the texts of a language from the map, into memory that
is taken for them */
void language_texts_load(
	struct language_texts *texts)
{
	uint32_t size= (uint32_t)texts->count*sizeof(struct language_text_reference);

	texts->references= physical_memory_allocate_high(size);
	cache_file_blocking_read(_cache_file_read_unknown_none, texts->references_offset, texts->references,
		(long)cache_file_round_to_sector((uint32_t)texts->count*sizeof(struct language_text_reference)));

	texts->data= physical_memory_allocate_high((uint32_t)texts->data_size);
	cache_file_blocking_read(_cache_file_read_unknown_none, texts->data_offset, texts->data,
		(long)cache_file_round_to_sector((uint32_t)texts->data_size));
	texts->loaded= TRUE;
}

/* 0x001a02d0: a text among so many of a language, from one on: an empty
text if it is not there */
const char *language_texts_find(
	const struct language_texts *texts,
	long first,
	long count,
	unsigned long string_id)
{
	long index;

	for (index= first; index<first+count; index++)
	{
		if (language_texts_has_index(texts, index) && texts->references[index].string_id==string_id)
		{
			return texts->data+texts->references[index].offset;
		}
	}

	return "";
}

/* 0x001a0310: whether a text is among so many of a language */
boolean language_texts_has(
	const struct language_texts *texts,
	long first,
	unsigned long string_id,
	long count)
{
	long index;

	if (!texts->loaded)
	{
		return FALSE;
	}
	for (index= first; index<first+count; index++)
	{
		if (language_texts_has_index(texts, index) && texts->references[index].string_id==string_id)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* 0x001a0350: a text among so many of a language, as characters of 16 bits.
Nothing is written if the texts are not read */
void language_texts_get(
	const struct language_texts *texts,
	unsigned long string_id,
	long count,
	wchar *string,
	long first)
{
	if (texts->loaded)
	{
		wchar text[UNICODE_STRING_LENGTH];

		utf8_string_to_wchar_string(language_texts_find(texts, first, count, string_id), text, UNICODE_STRING_LENGTH);
		wcsncpy(string, text, UNICODE_STRING_LENGTH-1);
		string[UNICODE_STRING_LENGTH-1]= 0;
	}
}
