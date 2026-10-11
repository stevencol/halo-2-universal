/*
NETWORK_MOTD.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The message of the day. The names are guesses.
*/

#include "network_motd.h"

#include "cache/cache_files.h"
#include "cseries/cseries.h"
#include "networking/network_blob.h"

#include <string.h>

/* ---------- globals */

/* 0x004672b8 */
const char *network_motd_file_name= "message_of_the_day.dat";

/* 0x00479780, of online_tasks.c: what has the block of the message */
extern struct network_blob unknown_00479780;

/* 0x0033039a: the language of the console */
long XGetLanguage(void);

/* ---------- code */

/* 0x0008ca00: the file of the message of the day, on the hard disk. The
game has the making of a file from a path written out here */
void __attribute__((stdcall)) network_motd_get_file(
	struct file_reference *file)
{
	memset(file, 0, sizeof(*file));
	file->signature= FILE_REFERENCE_SIGNATURE;
	file->location= NONE;
	file_path_add_name(file->path, "n:\\");
	if (file->flags&1)
	{
		path_remove_last_element(file->path);
	}
	file_path_add_name(file->path, network_motd_file_name);
	file->flags|= 1;
}

/* 0x0008ca50: whether there is a message of the day: its block is of a kind
and is good */
boolean network_motd_available(
	void)
{
	return unknown_00479780.kind!=0 && (unknown_00479780.flags&_network_blob_good_bit);
}

/* 0x0008c900: the message of the day is written to its file: only the two
texts of the language of the game. Whether it was; a file that could not be
written whole is deleted. The game has the closing of the file written out
here */
boolean __attribute__((stdcall)) network_motd_write_file(
	const byte *message)
{
	struct file_reference file;
	boolean written= FALSE;
	long error;

	network_motd_get_file(&file);
	if (!file_create(&file))
	{
		return FALSE;
	}
	if (file_open(&file, NETWORK_MOTD_FILE_OPEN_FLAGS, &error))
	{
		long language;

		if (global_language==NONE)
		{
			global_language= language_from_xbox_language(XGetLanguage());
		}
		language= global_language;
		if (file_write(&file, NETWORK_MOTD_TITLE_SIZE, message+language*NETWORK_MOTD_TITLE_SIZE) &&
			file_write(&file, NETWORK_MOTD_TEXT_SIZE,
				message+NETWORK_MOTD_TEXTS_OFFSET+language*NETWORK_MOTD_TEXT_SIZE))
		{
			written= TRUE;
		}
		file_close(&file);
		if (written)
		{
			return TRUE;
		}
	}
	file_delete(&file);

	return FALSE;
}
