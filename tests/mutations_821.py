# mutations of milestone 821, written by make_mutations.py

TESTS = "test_small_821"

SOURCE_EFFECTS_EFFECTS_C = "source/effects/effects.c"
SOURCE_MATH_REAL_MATH_C = "source/math/real_math.c"

MUTATIONS = [
    (SOURCE_EFFECTS_EFFECTS_C, '\t\tconst byte *location= (const byte *)particle_location_data->data+(location_index&0xFFFF)*0x34;', '\t\tconst byte *location= (const byte *)particle_location_data->data+(location_index&0xFFFF)*0x35;'),
    # taken out: without the next location the walk never ends (the emulator hangs)
    (SOURCE_EFFECTS_EFFECTS_C, '\twhile (location_index!=NONE)', '\twhile (location_index!=0)'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tlong location_index= *(const long *)(data+0x30);', '\tlong location_index= *(const long *)(data+0x31);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tlong particle_index= *(const long *)(data+0x04);', '\tlong particle_index= *(const long *)(data+0x5);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\tlong next= *(const long *)((const byte *)particle_data->data+(particle_index&0xFFFF)*0x40+0x04);', '\t\tlong next= *(const long *)((const byte *)particle_data->data+(particle_index&0x10000)*0x40+0x04);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\twhile (particle_index!=NONE)', '\twhile (particle_index==NONE)'),
    # taken out: without the next particle the walk never ends (the emulator hangs, which is caught by hand)
    (SOURCE_EFFECTS_EFFECTS_C, '\t\t\tpair[0]= 0;', ''),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\tif (pair[1]==value)', '\t\tif (pair[2]==value)'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tlong *pair= (long *)((byte *)effect_data->data+(effect_index&0xFFFF)*0x190+0x110);', '\tlong *pair= (long *)((byte *)effect_data->data+(effect_index&0xFFFF)*0x190+0x111);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tfor (index= 0; index<16; index++, pair+= 2)', '\tfor (index= 0; index<16; index++, pair+= 3)'),
    # taken out: >= clamps to the same 0 and 1
]
