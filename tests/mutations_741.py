# mutations of milestone 741: one or two lines of each function

TESTS = "test_small_741"

SESSION = "source/networking/network_session.c"
LIFE = "source/networking/network_life_cycle.c"
BLOB = "source/networking/network_blob.c"
PLAYER = "source/networking/simulation_player.c"
TASK = "source/networking/network_blob_task.c"

MUTATIONS = [
    (SESSION, "\t\tif (memcmp(node+0x1A0, other+0x188, 0x24)==0)", "\t\tif (memcmp(node+0x1A0, other+0x188, 0x20)==0)"),
    (SESSION, "\t\tnode= *(byte *const *)node;\n\t}\n\n\treturn found;", "\t\tnode= NULL;\n\t}\n\n\treturn found;"),
    (SESSION, "\t*(long *)(something+0x54)= 0;\n\tif (size>0)", "\tif (size>0)"),
    (SESSION, "\t\tmemcpy(something+0x54, data, size);", "\t\tmemcpy(something+0x54, data, size-1);"),
    (SESSION, "\tsomething[0x4A]= TRUE;", "\tsomething[0x4B]= TRUE;"),
    (LIFE, "\telse if (state>=_network_session_state3 && state<=_network_session_state8)\n\t{\n\t\tsomething[0xF8]= TRUE;", "\telse if (state>=_network_session_state2 && state<=_network_session_state8)\n\t{\n\t\tsomething[0xF8]= TRUE;"),
    (LIFE, "\t\t*(long *)(something+0x104)= 0x10;", "\t\t*(long *)(something+0x104)= 0x11;"),
    (LIFE, "\t\tsomething[0xF8]= TRUE;", "\t\tsomething[0xF9]= TRUE;"),
    (LIFE, "\tfor (peer_index= 0; peer_index<peer_count && all; peer_index++)\n\t{\n\t\tconst byte *peer= session+NETWORK_SESSION_PEER_ARRAY_OFFSET+peer_index*NETWORK_SESSION_PEER_SIZE;\n\n\t\tif (*(const long *)(peer+NETWORK_SESSION_PEER_UNKNOWNEC_OFFSET)!=2)", "\tfor (peer_index= 0; peer_index<peer_count-1 && all; peer_index++)\n\t{\n\t\tconst byte *peer= session+NETWORK_SESSION_PEER_ARRAY_OFFSET+peer_index*NETWORK_SESSION_PEER_SIZE;\n\n\t\tif (*(const long *)(peer+NETWORK_SESSION_PEER_UNKNOWNEC_OFFSET)!=2)"),
    (LIFE, "\t\tif (*(const long *)(peer+NETWORK_SESSION_PEER_UNKNOWNEC_OFFSET)!=2)", "\t\tif (*(const long *)(peer+NETWORK_SESSION_PEER_UNKNOWNEC_OFFSET)>2)"),
    (BLOB, "\tmemset(result, 0, 16);", "\tmemset(result, 0, 8);"),
    (BLOB, "\tresult[7]= blob[0x53];", "\tresult[7]= blob[0x52];"),
    (PLAYER, "\tmemcpy(something+0x720+index*0x40, data, 0x40);", "\tmemcpy(something+0x720+index*0x40, data, 0x3C);"),
    (PLAYER, "\t*(unsigned long *)(something+0x718)|= 1<<index;", "\t*(unsigned long *)(something+0x718)= 1<<index;"),
    (TASK, "\t*(long *)(something+0x218)= NONE;", "\t*(long *)(something+0x218)= 0;"),
    (TASK, "\t*(short *)(something+0xC)= 0;", "\t*(byte *)(something+0xC)= 0;"),
    (TASK, "\tif (first<0x499A)", "\tif (first<=0x499A)"),
    (TASK, "\t\t*(long *)(something+0x214)= third;", "\t\t*(long *)(something+0x214)= second;"),
]
