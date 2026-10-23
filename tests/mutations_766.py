# mutations of milestone 766: one or more lines of each function

TESTS = "test_small_766"

LEVELS = "source/main/levels.c"
CONTROLLER = "source/interface/user_interface_controller.c"

MUTATIONS = [
    (LEVELS, "\tif (*(const long *)data!=LEVELS_UNKNOWN_0019BDC0_VERSION)", "\tif (*(const long *)data!=3)"),
    (LEVELS, "\t*(unsigned long *)(data+LEVELS_UNKNOWN_0019BDC0_CHECKSUM_OFFSET)= 0;\n", ""),
    (LEVELS, "\tcrc= CRC_NEW;", "\tcrc= 0;"),
    (LEVELS, "\t*(unsigned long *)(data+LEVELS_UNKNOWN_0019BDC0_CHECKSUM_OFFSET)= checksum;\n", ""),
    (LEVELS, "\treturn checksum==crc;", "\treturn checksum!=crc;"),
    (LEVELS, "\t\tif (number!=NONE && (lowest==NONE || number<lowest))", "\t\tif (number!=NONE && (lowest==NONE || number>lowest))"),
    (LEVELS, "\t\tif (number!=NONE && (lowest==NONE", "\t\tif ((lowest==NONE"),
    (LEVELS, "\tconst byte *level0= levels_get_multiplayer_level(*number0);", "\tconst byte *level0= levels_get_multiplayer_level(*number1);"),
    (LEVELS, "\tif (value0>value1)\n\t{\n\t\treturn 1;", "\tif (value0>=value1)\n\t{\n\t\treturn 1;"),
    (LEVELS, "\treturn value0>=value1 ? 0 : -1;", "\treturn value0>value1 ? 0 : -1;"),
    (CONTROLLER, "\t\t\tprofile_count++;", "\t\t\tprofile_count+= 2;"),
    (CONTROLLER, "\t\tif ((counted->flags>>_user_interface_controller_profile_bit)&1)", "\t\tif ((counted->flags>>_user_interface_controller_online_bit)&1)"),
]
