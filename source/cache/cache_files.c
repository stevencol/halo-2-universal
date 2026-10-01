/*
CACHE_FILES.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.
*/

#include "cache_files.h"

#include <string.h>

/* ---------- globals */

/* 0x0043e86c: the builds whose maps the game accepts */
static const char *const cache_file_builds[]=
{
	"02.09.27.09809"
};

/* ---------- code */

/* 0x00214ed0 */
int32_t cache_file_maximum_length(
	long scenario_type)
{
	switch (scenario_type)
	{
	case _scenario_type_solo:
		return 0x11800000;
	case _scenario_type_multiplayer:
	case _scenario_type_main_menu:
		return 0x05000000;
	case _scenario_type_shared:
		return 0x0B400000;
	case _scenario_type_single_player_shared:
		return 0x20800000;
	}

	return 0;
}

/* 0x00012130 */
bool cache_file_build_verify(
	const char *build)
{
	size_t build_index;

	for (build_index= 0; build_index<sizeof(cache_file_builds)/sizeof(cache_file_builds[0]); build_index++)
	{
		if (strcmp(build, cache_file_builds[build_index])==0)
		{
			return true;
		}
	}

	return false;
}

/* 0x00122870 */
bool cache_file_header_verify(
	const struct cache_file_header *header)
{
	return
		header->header_signature==CACHE_FILE_HEADER_SIGNATURE &&
		header->footer_signature==CACHE_FILE_FOOTER_SIGNATURE &&
		header->file_length>=0 &&
		header->file_length<=0x20800000 &&
		header->scenario_type>=0 &&
		header->scenario_type<NUMBER_OF_SCENARIO_TYPES &&
		header->file_length<=cache_file_maximum_length(header->scenario_type) &&
		memchr(header->name, 0, CACHE_FILE_NAME_LENGTH)!=NULL &&
		header->version==CACHE_FILE_VERSION;
}
