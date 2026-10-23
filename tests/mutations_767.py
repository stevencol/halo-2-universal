# mutations of milestone 767: one or two lines of each function

TESTS = "test_small_767"

ONLINE = "source/networking/online_unknown_001a2f70.c"

MUTATIONS = [
    (ONLINE, "\t*result= task ? online_task_continue(task) : (long)ONLINE_TASK_NOT_THERE;", "\t*result= task ? online_task_continue(task) : 0;"),
    (ONLINE, "\tonline_task_close(*task_index);\n", ""),
    (ONLINE, "\t*task_index= NONE;", "\t*task_index= 0;"),
    (ONLINE, "\tif (unknown_0046e7d4==NONE || !unknown_0046e7bc)", "\tif (unknown_0046e7d4==NONE)"),
    (ONLINE, "\treturn status==ONLINE_UNKNOWN_TASK_STATUS1 || status==ONLINE_UNKNOWN_TASK_STATUS2;", "\treturn status==ONLINE_UNKNOWN_TASK_STATUS1;"),
    (ONLINE, "\treturn unknown_0046eeb6[0] ? unknown_0046eeb6[1] : 1;", "\treturn unknown_0046eeb6[0] ? unknown_0046eeb6[1] : 0;"),
    (ONLINE, "\t\tmemset(thing, 0, ONLINE_UNKNOWN_0046E814_SIZE);", "\t\tmemset(thing, 0, ONLINE_UNKNOWN_0046E814_SIZE-2);"),
    (ONLINE, "\t\tmemcpy(thing, unknown_0046e814, ONLINE_UNKNOWN_0046E814_SIZE);", "\t\tmemcpy(thing, unknown_0046e814, ONLINE_UNKNOWN_0046E814_SIZE-1);"),
    (ONLINE, "\tunknown_0046eeb6[0]= FALSE;", "\tunknown_0046eeb6[1]= FALSE;"),
    (ONLINE, "\tmemset(unknown_0046e814, 0, ONLINE_UNKNOWN_0046E814_SIZE);", "\tmemset(unknown_0046e814, 0, 0x6A0);"),
]
