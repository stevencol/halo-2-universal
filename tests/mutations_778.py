# mutations of milestone 778: one or two lines of each function

TESTS = "test_small_778"

ONLINE = "source/networking/online_unknown_000b36c0.c"
STORAGE = "source/networking/online_storage.c"

MUTATIONS = [
    (ONLINE, "\tif (unknown_00467214==NONE || online_tasks_get_logon_state(unknown_00467214)!=_online_session_signed_in)", "\tif (unknown_00467214==NONE)"),
    (ONLINE, "\t\t\tavailable= unknown_00467178[index].available;", "\t\t\tavailable= TRUE;"),
    (ONLINE, "\tfor (index= 0; index<NUMBER_OF_ONLINE_SERVICES; index++)", "\tfor (index= 0; index<NUMBER_OF_ONLINE_SERVICES-1; index++)"),
    (ONLINE, "\t\tXNetUnregisterInAddr(unknown_005107e0);\n", ""),
    (ONLINE, "\tXNetUnregisterKey(key+ONLINE_UNKNOWN_000B36C0_KEY_OFFSET);", "\tXNetUnregisterKey(key);"),
    (ONLINE, "\tunknown_005107e4--;", "\tunknown_005107e4++;"),
    (ONLINE, "\tunknown_005107dc= NONE;\n", ""),
    (ONLINE, "\tfor (length= 0; length<ONLINE_UNKNOWN_000B36C0_MAXIMUM_STRING_LENGTH; length++)", "\tfor (length= 0; length<0x1000; length++)"),
    (STORAGE, "\treturn ONLINE_ERROR_MESSAGE_DEFAULT;", "\treturn 0;"),
    (STORAGE, "{0x80151000, 0x23}, {0x80151001, 0x23}", "{0x80151000, 0x23}, {0x80151001, 0x24}"),
    (STORAGE, "\t\tdestination[index]= source[2*index];", "\t\tdestination[index]= source[2*index+1];"),
]
