# mutations of milestone 742: one or two lines of each function

TESTS = "test_small_742"

SESSION = "source/networking/network_session.c"
LIFE = "source/networking/network_life_cycle.c"

MUTATIONS = [
    (SESSION, "\tmemset(something+4, 0, 10*sizeof(long));", "\tmemset(something+4, 0, 9*sizeof(long));"),
    (SESSION, "\t*(long *)(something+0x2C)= unknown2C;", "\t*(long *)(something+0x2C)= unknown30;"),
    (SESSION, "\tsomething[0x4A]= FALSE;\n}", "}"),
    (SESSION, "\treturn network_session_time()-start;", "\treturn start-network_session_time();"),
    (LIFE, "\t*(byte **)(manager+0x18)= something;", "\t*(byte **)(manager+0x1C)= something;"),
    (LIFE, "\t*(long *)(something+0xF4)= NONE;", "\t*(long *)(something+0xF4)= 0;"),
    (LIFE, "\tif (manager_state!=1 && manager_state!=0 && manager_state!=4 && manager_state!=9)", "\tif (manager_state!=1 && manager_state!=0 && manager_state!=4)"),
    (LIFE, "\tif (state<_network_session_state3 || state>_network_session_state8 ||", "\tif (state<_network_session_state2 || state>_network_session_state8 ||"),
    (LIFE, "\t\t*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET)!=", "\t\t*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET+4)!="),
    (LIFE, "\t\t(*(long *)(something+0xA90))++;", "\t\t(*(long *)(something+0xA90))--;"),
    (LIFE, "\t\t*(long *)(something+0xA94)+= now-start;", "\t\t*(long *)(something+0xA94)+= now;"),
    (LIFE, "\tsomething[0xA78]= FALSE;", "\tsomething[0xA78]= TRUE;"),
    (LIFE, "\t\t*(long *)(something+0xA88)+= now-start;", "\t\t*(long *)(something+0xA88)= now-start;"),
    (LIFE, "\tif (count)\n\t{\n\t\tlong start= *(const long *)(something+0xA68);", "\tif (!count)\n\t{\n\t\tlong start= *(const long *)(something+0xA68);"),
    (LIFE, "\treturn unknown_00510548 ? unknown_0051054c : GetTickCount();", "\treturn GetTickCount();"),
]
