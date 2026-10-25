# mutations of milestone 824, written by make_mutations.py

TESTS = "test_small_824"

SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_EFFECTS_EFFECTS_C = "source/effects/effects.c"
SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"

MUTATIONS = [
    (SOURCE_PHYSICS_HAVOK_C, '\t\thigh= *(const real *)(definition+0x08);', ''),
    # taken out: >= gives the same real when both are the same
    (SOURCE_PHYSICS_HAVOK_C, '\tif (!(definition[1]>>4))', '\tif (!(definition[1]>>5))'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tlow= *(const real *)(definition+0x04);', '\t\tlow= *(const real *)(definition+0x5);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\topenings[2]= 0;', '\t\topenings[3]= 0;'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\topenings[0]= 0;', '\t\topenings[1]= 0;'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tbits= *(const byte *const *)(structure_bsp+0x228);', ''),
    (SOURCE_EFFECTS_EFFECTS_C, '\tmemcpy(openings, *(const unsigned long *const *)(bits+0x18)+longs*cluster_index, longs*4);', '\tmemcpy(openings, *(const unsigned long *const *)(bits+0x19)+longs*cluster_index, longs*4);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tlong index= *(const long *)(element+0x40);', '\tlong index= *(const long *)(element+0x41);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\tindex= *(const long *)(other+0x38);', ''),
    (SOURCE_EFFECTS_EFFECTS_C, '\twhile (index!=NONE)', '\twhile (index!=0)'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tbyte *effect= (byte *)effect_data->data+(effect_index&0xFFFF)*0x190;', '\tbyte *effect= (byte *)effect_data->data+(effect_index&0x10000)*0x190;'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tparticle_system_list_remove(PARTICLE_SYSTEM_GET(particle_system_index), (long *)(effect+0xFC), (long *)(effect+0x100));', '\tparticle_system_list_remove(PARTICLE_SYSTEM_GET(particle_system_index), (long *)(effect+0xfd), (long *)(effect+0x100));'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tsigned char number= (signed char)OBJECT_DATA(object_index)[0x3AA];', '\tsigned char number= (signed char)OBJECT_DATA(object_index)[0x3ab];'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (!number)\n\t{\n\t\treturn FALSE;\n\t}\n\t*value= game_time_globals->seconds_per_tick*0.15f*(real)number;', '\tif (!number)\n\t{\n\t\treturn TRUE;\n\t}\n\t*value= game_time_globals->seconds_per_tick*0.15f*(real)number;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t*value= game_time_globals->seconds_per_tick*0.15f*(real)number;\n\treturn TRUE;\n}', '\t*value= game_time_globals->seconds_per_tick*0.15f*(real)number;\n\treturn FALSE;\n}'),
]
