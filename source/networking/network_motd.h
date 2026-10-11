/*
NETWORK_MOTD.H

The message of the day: a file of the hard disk that one of the blocks of
the service of games fills (unknown_00479780 of online_tasks.c). Worked out
from halo2ship.exe; the name of the file is a guess, and so are the names of
its functions.
*/

#ifndef __NETWORK_MOTD_H
#define __NETWORK_MOTD_H

#include "cseries/cseries.h"
#include "tag_files/files.h"
#include "text/unicode.h"

/* ---------- constants */

enum
{
	/* the message, as it comes from the service of games: for each of nine
	languages, a text of 0x200 bytes, and after them all, for each, one of
	0x4004 */
	NUMBER_OF_NETWORK_MOTD_LANGUAGES= 9,
	NETWORK_MOTD_TITLE_SIZE= 0x200,
	NETWORK_MOTD_TEXT_SIZE= 0x4004,
	NETWORK_MOTD_TEXTS_OFFSET= NUMBER_OF_NETWORK_MOTD_LANGUAGES*NETWORK_MOTD_TITLE_SIZE,
	NETWORK_MOTD_SIZE= NETWORK_MOTD_TEXTS_OFFSET+NUMBER_OF_NETWORK_MOTD_LANGUAGES*NETWORK_MOTD_TEXT_SIZE,
	/* how its file is opened: to write */
	NETWORK_MOTD_FILE_OPEN_FLAGS= 2,
	/* and to read. What is read of it: its first text, or one of the texts
	of the second, which is a number of them, 32 at most, and the texts */
	NETWORK_MOTD_FILE_OPEN_READ_FLAGS= 1,
	_network_motd_title= 0,
	_network_motd_random_text,
	MAXIMUM_NETWORK_MOTD_TEXTS= 32,
	NETWORK_MOTD_TEXT_LENGTH= 255
};

/* ---------- structures */

struct network_motd_texts
{
	long count;
	wchar texts[MAXIMUM_NETWORK_MOTD_TEXTS][NETWORK_MOTD_TEXT_LENGTH+1];
};

typedef char verify_network_motd_texts_size[sizeof(struct network_motd_texts) == NETWORK_MOTD_TEXT_SIZE ? 1 : -1];

/* ---------- prototypes/NETWORK_MOTD.C */

void __attribute__((stdcall)) network_motd_get_file(struct file_reference *file);
boolean network_motd_available(void);
boolean __attribute__((stdcall)) network_motd_write_file(const byte *message);
boolean __attribute__((stdcall)) network_motd_read(long which, wchar *text);

/* ---------- globals */

extern const char *network_motd_file_name;

#endif
