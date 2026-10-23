# mutations of milestone 768: one or two lines of each function

TESTS = "test_small_768"

TEXTS = "source/interface/user_interface_text_values.c"
CONTROLLER = "source/interface/user_interface_controller.c"

MUTATIONS = [
    (TEXTS, "\t\tunknown_0055c2e0= index;", "\t\tunknown_0055c2e0= 0;"),
    (TEXTS, "\tif (network_hoppers_get_name(unknown_0055c2e0, name))", "\tif (network_hoppers_get_name(index, name))"),
    (TEXTS, "\tif (hopper_index!=NONE)", "\tif (TRUE)"),
    (TEXTS, "\tunknown_0023620d(USER_INTERFACE_TEXT_UNKNOWN_060000B8, text);", "\tunknown_0023620d(0x060000B9, text);"),
    (TEXTS, "\tcase USER_INTERFACE_TEXT_PROFILE_NAME2:\n\t\tindex= 2;", "\tcase USER_INTERFACE_TEXT_PROFILE_NAME2:\n\t\tindex= 3;"),
    (TEXTS, "\t\tnetwork_motd_text_copy(text, unknown_0054e5e0);", "\t\tnetwork_motd_text_copy(text, unknown_0054e5e0+1);"),
    (TEXTS, "\t\tcontroller_index= 3;", "\t\tcontroller_index= 2;"),
    (TEXTS, "\tascii_string_to_wchar_string(user.name, text, USER_INTERFACE_TEXT_ONLINE_NAME_LENGTH);", "\tascii_string_to_wchar_string(user.name, text, 0xFF);"),
    (CONTROLLER, "\tboolean unavailable= TRUE;\n\n\tif (user_interface_controller_is_guest(index))\n\t{\n\t\treturn TRUE;", "\tboolean unavailable= TRUE;\n\n\tif (user_interface_controller_is_guest(index))\n\t{\n\t\treturn FALSE;"),
    (CONTROLLER, "\tif (online_signed_in())\n\t{\n\t\tunavailable= FALSE;", "\tif (online_signed_in())\n\t{\n\t\tunavailable= TRUE;"),
]
