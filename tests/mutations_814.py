# mutations of milestone 814, written by make_mutations.py

TESTS = "test_small_814"

SOURCE_GAME_GAME_ENGINE_C = "source/game/game_engine.c"
SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C = "source/networking/network_life_cycle.c"

MUTATIONS = [
    (SOURCE_GAME_GAME_ENGINE_C, '\tif ((kind==2 || kind==1) && *element<*(const long *)(lists+0x18))', '\tif ((kind==2 || kind==2) && *element<*(const long *)(lists+0x18))'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\t\t\tresult= TRUE;', ''),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\t\tif (*element>=*(const long *)(lists+0x18))', '\t\t\tif (*element>*(const long *)(lists+0x18))'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\t\tif (*place<*(const long *)((*(const byte *const *const *)(lists+0x08))[*element]+0x40))', '\t\t\tif (*place<*(const long *)((*(const byte *const *const *)(lists+0x08))[*element]+0x41))'),
    (SOURCE_PHYSICS_HAVOK_C, '\treturn first_bit && second_bit;', '\treturn first_bit || second_bit;'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tfirst_bit= (*(const unsigned long *)((const byte *)havok_component_data->data+(first&0xFFFF)*0xA0+4)>>11)&1;', '\t\tfirst_bit= (*(const unsigned long *)((const byte *)havok_component_data->data+(first&0xFFFF)*0xA0+5)>>11)&1;'),
    (SOURCE_PHYSICS_HAVOK_C, '\tboolean second_bit= TRUE;', '\tboolean second_bit= FALSE;'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tsecond_bit= (*(const unsigned long *)((const byte *)havok_component_data->data+(second&0xFFFF)*0xA0+4)>>11)&1;', '\t\tsecond_bit= (*(const unsigned long *)((const byte *)havok_component_data->data+(second&0xFFFF)*0xa1+4)>>11)&1;'),
    # taken out: clearing from 0x61 leaves the same bytes after havok_unknown_001cafc0 writes them
    (SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C, '\tdata[0x0D]= 1;', '\tdata[0xe]= 1;'),
    (SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C, '\t*(long *)(data+0x978)= 1;\n\tseed= global_random_seed_reference[1]*0x19660D+0x3C6EF35F;\n\tglobal_random_seed_reference[1]= seed;', '\t*(long *)(data+0x978)= 1;\n\tseed= global_random_seed_reference[1]*0x19660D+0x3c6ef360;\n\tglobal_random_seed_reference[1]= seed;'),
    (SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C, '\t*(long *)(data+0x96C)= (short)((unsigned long)((long)(seed>>16)*(long)(short)unknown_004ce1dc)>>16);\n\tseed= global_random_seed_reference[1]*0x19660D+0x3C6EF35F;\n\tglobal_random_seed_reference[1]= seed;', '\t*(long *)(data+0x96C)= (short)((unsigned long)((long)(seed>>16)*(long)(short)unknown_004ce1dc)>>16);\n\tseed= global_random_seed_reference[1]*0x19660e+0x3C6EF35F;\n\tglobal_random_seed_reference[1]= seed;'),
    (SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C, '\tdata[0x0C]= 0;', ''),
]
