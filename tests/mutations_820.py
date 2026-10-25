# mutations of milestone 820, written by make_mutations.py

TESTS = "test_small_820"

SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"
SOURCE_EFFECTS_EFFECTS_C = "source/effects/effects.c"
SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C = "source/networking/network_life_cycle.c"

MUTATIONS = [
    (SOURCE_OBJECTS_OBJECTS_C, '\tplace[1]= 0;', ''),
    (SOURCE_OBJECTS_OBJECTS_C, '\tplace[0]= 0;', '\tplace[0]= 1;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tplace[3]= 0;', ''),
    (SOURCE_OBJECTS_OBJECTS_C, '\tbyte *place= object+*(const short *)(object+0x12A)+0x64;', '\tbyte *place= object+*(const short *)(object+0x12A)+0x65;'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\temitter_index= *(const long *)(emitter+0x08);', '\t\temitter_index= *(const long *)(emitter+0x9);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\ttotal+= *(const unsigned short *)(emitter+0x02);', '\t\ttotal+= *(const unsigned short *)(emitter+0x3);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\twhile (emitter_index!=NONE)', '\twhile (emitter_index==NONE)'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\tconst byte *emitter= (const byte *)particle_emitter_data->data+(emitter_index&0xFFFF)*0x4C;', '\t\tconst byte *emitter= (const byte *)particle_emitter_data->data+(emitter_index&0x10000)*0x4C;'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\ttotal+= (at[0]-at[0x44/4])*at[-0xF02DC/4];', '\t\ttotal+= (at[1]-at[0x44/4])*at[-0xF02DC/4];'),
    (SOURCE_PHYSICS_HAVOK_C, '\tfor (index= 0; index<16; index++)', '\tfor (index= 0; index<17; index++)'),
    (SOURCE_PHYSICS_HAVOK_C, '\tlong total= 0;', '\tlong total= 1;'),
    (SOURCE_PHYSICS_HAVOK_C, '\tconst long *at= (const long *)(data+0xF0384);', '\tconst long *at= (const long *)(data+0xf0385);'),
    # taken out: copying one character more is the same: it is made 0 after
    (SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C, '\tunknown_004cd88a[31]= 0;', '\tunknown_004cd88a[31]= 1;'),
    (SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C, '\tunknown_004cd86a[15]= 0;', '\tunknown_004cd86a[16]= 0;'),
]
