# mutations of milestone 819, written by make_mutations.py

TESTS = "test_small_819"

SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C = "source/networking/network_life_cycle.c"
SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"

MUTATIONS = [
    (SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C, '\tif (!unknown_00527330)\n\t{\n\t\treturn FALSE;\n\t}\n\tstate= *(const long *)(network_life_cycle_session+0x741C);', '\tif (!unknown_00527330)\n\t{\n\t\treturn TRUE;\n\t}\n\tstate= *(const long *)(network_life_cycle_session+0x741C);'),
    (SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C, '\tif (!state || state<=2 || state>8)', '\tif (!state || state<2 || state>8)'),
    (SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C, '\tif (!state || state<=2 || state>8)\n\t{\n\t\treturn FALSE;\n\t}\n', '\tif (!state || state<=2 || state>8)\n\t{\n\t\treturn TRUE;\n\t}\n'),
    (SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C, '\tstate= *(const long *)(network_life_cycle_session+0x741C);', '\tstate= *(const long *)(network_life_cycle_session+0x741d);'),
    (SOURCE_PHYSICS_HAVOK_C, '\tfor (next= *(const byte *const *)(node+0x0C); next; next= *(const byte *const *)(node+0x0C))', '\tfor (next= *(const byte *const *)(node+0xd); next; next= *(const byte *const *)(node+0x0C))'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tif (*(const unsigned long *)(*(const byte *const *)next+0x08)==0xCABCABB0)', '\t\tif (*(const unsigned long *)(*(const byte *const *)next+0x9)==0xCABCABB0)'),
    (SOURCE_PHYSICS_HAVOK_C, '\treturn *(const byte *const *)(node+0x0C) ? *(const long *)(node+0x04) : NONE;', '\treturn *(const byte *const *)(node+0x0C) ? *(const long *)(node+0x04) : 0;'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tnode= next;', ''),
]
