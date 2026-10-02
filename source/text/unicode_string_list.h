/*
UNICODE_STRING_LIST.H

The texts of the game, in each of its languages. The texts of all the tags
of a map, in a language, are together in the map: the globals tag tells
where, and a tag that has texts (a list of them) tells which are its own.
The names are guesses.
*/

#ifndef __UNICODE_STRING_LIST_H
#define __UNICODE_STRING_LIST_H

#include <stdint.h>

#include "cseries/cseries.h"
#include "text/unicode.h"

/* ---------- constants */

enum
{
	/* in the globals tag: the texts of the first language */
	GAME_GLOBALS_LANGUAGE_TEXTS_OFFSET= 0x188,

	/* how many characters a text is given with, at most */
	UNICODE_STRING_LENGTH= 256
};

/* ---------- structures */

/* a text of a language: which it is, and where it is among the others */
struct language_text_reference
{
	unsigned long string_id;
	long offset;
};

/* the texts of a language: in the map, and in memory once they are read */
struct language_texts
{
	struct language_text_reference *references;
	/* the texts, one after another, each with its end: they are UTF-8 */
	char *data;
	long count;
	long data_size;
	/* in the file of the map */
	uint32_t references_offset;
	uint32_t data_offset;
	boolean loaded;
	byte pad19[3];
};

/* the texts of a tag in a language: so many, from one on */
struct unicode_string_list_range
{
	short first;
	short count;
};

/* a tag that has texts */
struct unicode_string_list
{
	byte unknown0[0x10];
	/* by language */
	struct unicode_string_list_range ranges[9];
};

typedef char verify_language_texts_size[
	sizeof(struct language_texts) == 0x1C ? 1 : -1];

/* ---------- prototypes/UNICODE_STRING_LIST.C */

boolean unicode_string_list_has_string(long string_list_index, unsigned long string_id);
void unicode_string_list_get_string(long string_list_index, unsigned long string_id, wchar *string);
void language_texts_load(struct language_texts *texts);
const char *language_texts_find(const struct language_texts *texts, long first, long count, unsigned long string_id);
boolean language_texts_has(const struct language_texts *texts, long first, unsigned long string_id, long count);
void language_texts_get(const struct language_texts *texts, unsigned long string_id, long count, wchar *string,
	long first);

#endif
