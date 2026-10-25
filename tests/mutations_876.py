# mutations of milestone 876, written by make_mutations.py
# (one taken out by hand that changes nothing: <= instead of < for the lowest byte, -0x80 either way)

TESTS = "test_small_876"

SOURCE_UNITS_UNITS_C = "source/units/units.c"
SOURCE_SOUND_SOUND_MANAGER_C = "source/sound/sound_manager.c"

MUTATIONS = [
    (SOURCE_UNITS_UNITS_C, '\t\t\tdata[0x02]= (byte)((((global_random_seed_reference[1]>>16)*225)>>16)+30);', '\t\t\tdata[0x02]= (byte)((((global_random_seed_reference[1]>>16)*225)>>17)+30);'),
    (SOURCE_UNITS_UNITS_C, '\t\t\tvalue= *(const real *)(data+0x40)+0.15f;', ''),
    (SOURCE_UNITS_UNITS_C, '\t\treal scale= *(const real *)(definition+0x1E8)>0.0f ? *(const real *)(definition+0x1E8) : 0.9f;', '\t\treal scale= *(const real *)(definition+0x1e9)>0.0f ? *(const real *)(definition+0x1E8) : 0.9f;'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tvalue= motors[0]*65535.0f;', '\tvalue= motors[1]*65535.0f;'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tright= (unsigned short)lrintf(0.0f>value ? 0.0f : value>65535.0f ? 65535.0f : value);', ''),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\tif (effect_index<0 || effect_index>=*(const long *)(definition+0x6C))', '\t\tif (effect_index<1 || effect_index>=*(const long *)(definition+0x6C))'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\tmotors[0]= *(const real *)(unknown_00502120+0x220)**(const real *)(unknown_00502120+0x228) + motors[0];', '\t\tmotors[0]= *(const real *)(unknown_00502120+0x220)**(const real *)(unknown_00502120+0x229) + motors[0];'),
]
