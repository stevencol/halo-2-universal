# mutations of milestone 864, written by make_mutations.py

TESTS = "test_small_864"

SOURCE_EFFECTS_PARTICLE_SYSTEMS_C = "source/effects/particle_systems.c"

MUTATIONS = [
    (SOURCE_EFFECTS_PARTICLE_SYSTEMS_C, '\tdata= *(const byte *const *)(property+0x08+4);', '\tdata= *(const byte *const *)(property+0x08+5);'),
    (SOURCE_EFFECTS_PARTICLE_SYSTEMS_C, '\tif (!(data[1]&0xF0))', '\tif (!(data[2]&0xF0))'),
    (SOURCE_EFFECTS_PARTICLE_SYSTEMS_C, '\tif (index==NONE || index<0)', '\tif (index!=NONE || index<0)'),
    (SOURCE_EFFECTS_PARTICLE_SYSTEMS_C, '\t\t\tresult= 1.0f;', ''),
    (SOURCE_EFFECTS_PARTICLE_SYSTEMS_C, '\t\tfirst.red*= 0.5f;', ''),
    (SOURCE_EFFECTS_PARTICLE_SYSTEMS_C, '\tfirst.green*= second.green;', ''),
    (SOURCE_EFFECTS_PARTICLE_SYSTEMS_C, '\tfirst.blue*= second.blue;', ''),
    (SOURCE_EFFECTS_PARTICLE_SYSTEMS_C, '\tfirst.red*= second.red;', ''),
]
