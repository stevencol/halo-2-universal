# mutations of milestone 757: one or two lines of each function

TESTS = "test_small_757"

HOPPERS = "source/networking/network_hoppers.c"

MUTATIONS = [
    (HOPPERS, "\treturn network_hoppers_available();\n}", "\treturn unknown_0047d8f4.kind!=0;\n}"),
    (HOPPERS, "\tif (unknown_0047d8f4.kind==0 || unknown_0047d8f4.state!=1)", "\tif (unknown_0047d8f4.kind==0 || unknown_0047d8f4.state!=0)"),
    (HOPPERS, "\t\tnetwork_blob_get_progress(&unknown_0047d8f4, progress);\n", ""),
    (HOPPERS, "language*NETWORK_HOPPER_NAME_SIZE, name,", "language*NETWORK_HOPPER_DESCRIPTION_SIZE, name,"),
    (HOPPERS, "\t\t\tNETWORK_HOPPER_NAME_SIZE);", "\t\t\tNETWORK_HOPPER_NAME_SIZE-1);"),
    (HOPPERS, "\t\tglobal_language= language_from_xbox_language(XGetLanguage());", "\t\tglobal_language= XGetLanguage();"),
    (HOPPERS, "\tif (global_language==NONE)\n\t{\n\t\tglobal_language= language_from", "\tif (global_language!=NONE)\n\t{\n\t\tglobal_language= language_from"),
    (HOPPERS, "NETWORK_HOPPER_DESCRIPTIONS_OFFSET+\n", "NETWORK_HOPPER_DESCRIPTIONS_OFFSET+0x10+\n"),
    (HOPPERS, "description, NETWORK_HOPPER_DESCRIPTION_SIZE);", "description, NETWORK_HOPPER_NAME_SIZE);"),
    (HOPPERS, "\t\treturn network_hoppers[index].unknown5A0[NETWORK_HOPPER_UNKNOWN5E4_INDEX];", "\t\treturn network_hoppers[index].unknown5A0[NETWORK_HOPPER_UNKNOWN5E4_INDEX-1];"),
    (HOPPERS, "\treturn 0;\n}\n\n/* 0x00194610", "\treturn NONE;\n}\n\n/* 0x00194610"),
    (HOPPERS, "\t\treturn *(const byte *)&network_hoppers[index].unknown5A0[NETWORK_HOPPER_UNKNOWN5E8_INDEX];", "\t\treturn *((const byte *)&network_hoppers[index].unknown5A0[NETWORK_HOPPER_UNKNOWN5E8_INDEX]+1);"),
]
