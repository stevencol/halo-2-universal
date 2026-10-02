/*
FILES.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The names are those of Halo 1's files.c. path_remove_last_element
(0x001373c0), which is Halo 1's file_path_remove_name, is in cseries.c.
*/

#include "files.h"

#include <string.h>

/* ---------- code */

/* 0x00137320: a name after a path, with a separator between the two if the
path has none */
void file_path_add_name(
	char *path,
	const char *name)
{
	if (*name)
	{
		unsigned long length= strlen(path);
		char *end= path+length;

		if (end!=path && end[-1]!='\\')
		{
			*end++= '\\';
			*end= 0;
			length++;
		}
		strncpy(end, name, MAXIMUM_FILENAME_LENGTH+1-length);
		path[MAXIMUM_FILENAME_LENGTH]= 0;
	}
}

/* 0x00137370: an extension after a path */
void file_path_add_extension(
	char *path,
	const char *extension)
{
	if (*extension)
	{
		unsigned long length= strlen(path);
		char *end= path+length;

		if (end!=path)
		{
			*end++= '.';
			*end= 0;
			length++;
		}
		strncpy(end, extension, MAXIMUM_FILENAME_LENGTH+1-length);
		path[MAXIMUM_FILENAME_LENGTH]= 0;
	}
}

/* 0x00136f00: sets up the search for the files of a directory whose names
are as a pattern says */
void find_files_start(
	struct find_files_state *state,
	unsigned long flags,
	const struct file_reference *directory,
	const char *pattern)
{
	long index;

	for (index= 0; index<MAXIMUM_FIND_FILES_DEPTH; index++)
	{
		state->handles[index]= (uint32_t)NONE;
	}
	state->flags= flags;
	state->depth= 0;
	state->location= directory->location;
	strncpy(state->path, directory->path, MAXIMUM_FILENAME_LENGTH+1);
	state->path[MAXIMUM_FILENAME_LENGTH]= 0;
	strncpy(state->pattern, pattern, MAXIMUM_FILENAME_LENGTH+1);
	state->pattern[MAXIMUM_FILENAME_LENGTH]= 0;
}

/* 0x00137400: finds the parts of a path, and cuts it into them: zeros are
written where the name and the extension start. Each part that is not there
is left at the end of the path, which is an empty string. Of a path with no
file name, only where its last directory starts is told. The first character
of the path is not looked at */
void file_path_split(
	char *path,
	char **directory,
	char **parent_directory,
	char **filename,
	char **extension,
	boolean has_filename)
{
	short length= (short)strlen(path);
	char *character= path+length;

	*directory= character;
	*parent_directory= character;
	*filename= character;
	*extension= character;
	if (length>0)
	{
		unsigned long count;

		for (count= (word)length; count; count--, character--)
		{
			if (*character=='.')
			{
				if (has_filename && !**filename && !**extension)
				{
					*character= 0;
					*extension= character+1;
				}
			}
			else if (*character=='\\')
			{
				if (has_filename && !**filename)
				{
					*character= 0;
					*filename= character+1;
				}
				else if (!**parent_directory)
				{
					*parent_directory= character+1;
				}
			}
		}
	}

	if (has_filename && !**filename)
	{
		*filename= path;
	}
	else if (*filename!=path)
	{
		*directory= path;
	}
}
