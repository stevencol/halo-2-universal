# mutations of milestone 851, written by make_mutations.py

TESTS = "test_small_851"

SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C = "source/networking/network_life_cycle.c"
SOURCE_AI_ACTORS_C = "source/ai/actors.c"

MUTATIONS = [
    # taken out: for NONE the game reads past the table of tags, where the test has nothing (NULL either way)
    (SOURCE_PHYSICS_HAVOK_C, '\thavok_unknown_001ddc70(first_list, graph, mode, weapon_class, weapon_type, second_list);\n\tfor (index= 0; index<*(const long *)(graph+0x4C); index++)\n\t{', '\thavok_unknown_001ddc70(first_list, graph, mode, weapon_class, weapon_type, second_list);\n\tfor (index= 1; index<*(const long *)(graph+0x4C); index++)\n\t{'),
    (SOURCE_PHYSICS_HAVOK_C, '{\n\tlong index;\n\n\tif (!first_list && !second_list)\n\t{\n\t\treturn;\n\t}', '{\n\tlong index;\n\n\tif (!first_list || !second_list)\n\t{\n\t\treturn;\n\t}'),
    (SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C, '\t\tfor (index= 0; index<count; index++)', '\t\tfor (index= 1; index<count; index++)'),
    (SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C, '\t\tstatus= 4;', '\t\tstatus= 5;'),
    (SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C, '\t\t\t\t\tlowest= member[1];', ''),
    (SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C, '\t\t\telse if (member[0]==2)', '\t\t\telse if (member[0]==3)'),
    (SOURCE_AI_ACTORS_C, '\tmemset(bits, 0, (unsigned long)(((long)count+31)>>5)*4);', '\tmemset(bits, 0, (unsigned long)(((long)count+31)>>6)*4);'),
    (SOURCE_AI_ACTORS_C, '\t\t\tbits[number>>5]|= 1UL<<(number&0x1F);', '\t\t\tbits[number>>5]|= 1UL<<(number&0x20);'),
    # taken out: a bigger iterator changes nothing, and the byte at 0x10 is left as the stack has it, which the test cannot tell
]
