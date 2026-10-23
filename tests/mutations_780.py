# mutations of milestone 780: one or two lines of each function

TESTS = "test_small_780"

GAME = "source/game/game.c"
CSERIES = "source/cseries/cseries.c"
FONTS = "source/text/fonts.c"
POINTS = "source/game/game_unknown_0016efb0.c"

MUTATIONS = [
    (GAME, "\tlong index= *(const short *)(unknown+4);", "\tlong index= *(const short *)(unknown+6);"),
    (GAME, "\treturn (main_game_globals->unknown1178[index>>5]&(1u<<(index&31)))!=0;", "\treturn (main_game_globals->unknown1138[index>>5]&(1u<<(index&31)))!=0;"),
    (GAME, "\t\t*point= *(const real_point3d *)unknown;", "\t\tpoint->x= *(const real *)unknown;"),
    (GAME, "\t\t*unknownC= *(const long *)(unknown+0xC);", "\t\t*unknownC= *(const long *)(unknown+0x8);"),
    (CSERIES, "\tif (global_language==NONE)\n\t{\n\t\tglobal_language= language_from_xbox_language(XGetLanguage());", "\tif (global_language==NONE)\n\t{\n\t\tglobal_language= XGetLanguage();"),
    (CSERIES, "\tif (global_language==NONE)\n\t{\n\t\tglobal_language= language_from_xbox_language", "\tif (TRUE)\n\t{\n\t\tglobal_language= language_from_xbox_language"),
    (FONTS, "\tcsnzprintf(path, sizeof(path), \"%sfonts\\\\\", \"d:\\\\maps\\\\\");", "\tcsnzprintf(path, sizeof(path)-1, \"%sfonts\\\\\", \"d:\\\\maps\\\\\");"),
    (FONTS, "\tfolder->location= NONE;", "\tfolder->location= 0;"),
    (FONTS, "\tunknown_0055e718= \"d:\\\\maps\\\\\";", "\tunknown_0055e718= \"d:\\\\maps\";"),
    (POINTS, "\tif (local_player_index==NONE || global_structure_bsp_index==NONE)", "\tif (local_player_index==NONE)"),
    (POINTS, "\treturn (real_point3d *)(unknown_004e9bd4[local_player_index]+0xB8);", "\treturn (real_point3d *)(unknown_004e9bd4[local_player_index]+0xC4);"),
]
