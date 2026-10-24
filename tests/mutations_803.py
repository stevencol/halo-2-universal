# mutations of milestone 803, written by make_mutations.py

TESTS = "test_small_803"

SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_MAIN_MAIN_C = "source/main/main.c"
SOURCE_GAME_GAME_UNKNOWN_00134D90_C = "source/game/game_unknown_00134d90.c"
SOURCE_GAME_GAME_UNKNOWN_00131FF0_C = "source/game/game_unknown_00131ff0.c"

MUTATIONS = [
    (SOURCE_PHYSICS_HAVOK_C, '\thavok_unknown_clear(data+0x40);', '\thavok_unknown_clear(data+0x41);'),
    (SOURCE_PHYSICS_HAVOK_C, '\thavok_unknown_clear(data+0x20);', '\thavok_unknown_clear(data+0x21);'),
    (SOURCE_MAIN_MAIN_C, '\t\tif (*(const long *)(player+0x2C)!=NONE || (player[2]&8))', '\t\tif (*(const long *)(player+0x2C)!=NONE || (player[3]&8))'),
    (SOURCE_MAIN_MAIN_C, '\tfor (index= 0; index>=0 && index<players->first_unallocated; index++)', '\tfor (index= 0; index>=1 && index<players->first_unallocated; index++)'),
    (SOURCE_MAIN_MAIN_C, '\tglobals[4]= TRUE;', '\tglobals[4]= FALSE;'),
    (SOURCE_MAIN_MAIN_C, '\t\t\tglobals[5]= TRUE;', '\t\t\tglobals[6]= TRUE;'),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\tif (*(const short *)(element+0x22)>0)', '\tif (*(const short *)(element+0x22)>1)'),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\telement= *(const byte *const *)(tag+0x40)+(which%count)*0x3C;', '\telement= *(const byte *const *)(tag+0x41)+(which%count)*0x3C;'),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\tif (count<=0)', '\tif (count<=1)'),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\tif (tag_index==NONE)\n\t{\n\t\treturn NONE;\n\t}\n\ttag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;', '\tif (tag_index==NONE)\n\t{\n\t\treturn 0;\n\t}\n\ttag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;'),
    (SOURCE_GAME_GAME_UNKNOWN_00131FF0_C, '\tunsigned short index= *(const unsigned short *)(thing+0xA5E);', '\tunsigned short index= *(const unsigned short *)(thing+0xa5f);'),
    (SOURCE_GAME_GAME_UNKNOWN_00131FF0_C, '\tbits[bit>>5]|= 1UL<<(bit&0x1F);', ''),
    (SOURCE_GAME_GAME_UNKNOWN_00131FF0_C, '\tbits[1]= 0;', '\tbits[1]= 1;'),
    (SOURCE_GAME_GAME_UNKNOWN_00131FF0_C, '\tbits[3]= 0;', ''),
]
