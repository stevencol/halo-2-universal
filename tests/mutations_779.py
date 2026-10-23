# mutations of milestone 779: one or two lines of each function

TESTS = "test_small_779"

SECURITY = "source/networking/transport_security.c"
MATCHMAKING = "source/networking/online_matchmaking.c"
TASKS = "source/networking/online_tasks.c"
ENGINE = "source/game/game_engine.c"
REAL_MATH = "source/math/real_math.c"

MUTATIONS = [
    (SECURITY, "\tmemset(unknown_004cf7d4, 0, sizeof(unknown_004cf7d4));\n", ""),
    (SECURITY, "\tmemset(unknown_004cf7cc, 0, sizeof(unknown_004cf7cc));", "\tmemset(unknown_004cf7cc, 0, 4);"),
    (SECURITY, "\ttransport_register(NULL, transport_security_forget_address, transport_security_forget_address, 0);", "\ttransport_register(NULL, transport_security_forget_address, NULL, 0);"),
    (SECURITY, "\tunknown_004cf791= FALSE;\n\tunknown_004cf792= FALSE;\n\tmemset(unknown_004cf793", "\tunknown_004cf792= FALSE;\n\tmemset(unknown_004cf793"),
    (MATCHMAKING, "\treturn unknown ? (const char *)(unknown+0xC) : \"\";", "\treturn unknown ? (const char *)(unknown+0x8) : \"\";"),
    (MATCHMAKING, "\treturn unknown ? (const char *)(unknown+0xC) : \"\";", "\treturn unknown ? (const char *)(unknown+0xC) : \"?\";"),
    (TASKS, "\treturn online_task_continue(task)==ONLINE_TASK_UNKNOWN8015B108;", "\treturn online_task_continue(task)!=ONLINE_TASK_UNKNOWN8015B108;"),
    (TASKS, "\tstruct online_task *task= index!=NONE ? online_task_get(index) : NULL;\n\n\treturn online_task_continue", "\tstruct online_task *task= index!=NONE ? online_task_get(index&0xFFFF) : NULL;\n\n\treturn online_task_continue"),
    (ENGINE, "\tif ((*flags&4) && least>6)", "\tif ((*flags&4) && least>7)"),
    (ENGINE, "\t\tleast= 21;", "\t\tleast= 20;"),
    (ENGINE, "\treturn least==0x7FFFFFFF ? 0 : least;", "\treturn least;"),
    (REAL_MATH, "\tfor (index= 0; index<3; index++)\n\t{\n\t\tuint32_t bits;", "\tfor (index= 0; index<2; index++)\n\t{\n\t\tuint32_t bits;"),
    (REAL_MATH, "\t\tif ((bits&0x7F800000)==0x7F800000)", "\t\tif ((bits&0x7FC00000)==0x7FC00000)"),
]
