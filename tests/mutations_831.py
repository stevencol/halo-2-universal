# mutations of milestone 831, written by make_mutations.py

TESTS = "test_small_831"

SOURCE_EFFECTS_EFFECTS_C = "source/effects/effects.c"
SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_MAIN_MAIN_C = "source/main/main.c"

MUTATIONS = [
    (SOURCE_EFFECTS_EFFECTS_C, '\tmemset(data, 0, 0x60);', '\tmemset(data, 1, 0x60);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tif (fourth==1)', '\tif (fourth!=1)'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tlong index= *(const long *)(effect+0xFC);\n\n\twhile (index!=NONE)\n\t{\n\t\tbyte *system= PARTICLE_SYSTEM_GET(index);', '\tlong index= *(const long *)(effect+0xFC);\n\n\twhile (index==NONE)\n\t{\n\t\tbyte *system= PARTICLE_SYSTEM_GET(index);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\tif (kind==*(const short *)(effect+0x5C))', '\t\tif (kind==*(const short *)(effect+0x5d))'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\telse if (kind==*(const short *)(effect+0x5E))', '\t\telse if (kind!=*(const short *)(effect+0x5E))'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\t\tsystem[0x0C]|= 1;', '\t\t\tsystem[0xd]|= 1;'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\tconst byte *thing= *(const byte *const *)(data+0x4C)+index*0x60;', '\t\tconst byte *thing= *(const byte *const *)(data+0x4d)+index*0x60;'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\tmemcpy(entry, thing+0x18, 16);', '\t\tmemcpy(entry, thing+0x19, 16);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\tmemcpy(entry+0x10, thing+0x0C, 12);', '\t\tmemcpy(entry+0x10, thing+0xd, 12);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\tbyte *entry= entries+index*0x20;', '\t\tbyte *entry= entries+index*0x21;'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tif (*index==NONE)', '\tif (*index!=NONE)'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tlocation= (byte *)effect_location_data->data+(*index&0xFFFF)*0x3C;', '\tlocation= (byte *)effect_location_data->data+(*index&0x10000)*0x3C;'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tif (kind==3 || high==(kind==1))', '\tif (kind!=3 || high==(kind==1))'),
    (SOURCE_EFFECTS_EFFECTS_C, '\thigh= flags!=NONE && flags<0;', '\thigh= flags!=0 && flags<0;'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\t\tthing[0xAC]|= 1;', '\t\t\tthing[0xAC]|= 2;'),
    (SOURCE_PHYSICS_HAVOK_C, '\tcomponent= (const byte *)havok_component_data->data+(havok_component_index&0xFFFF)*0xA0;', '\tcomponent= (const byte *)havok_component_data->data+(havok_component_index&0xFFFF)*0xa1;'),
    (SOURCE_PHYSICS_HAVOK_C, '\tfor (index= 0; index<*(const long *)(component+0x74); index++)', '\tfor (index= 0; index<*(const long *)(component+0x74)-1; index++)'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tbyte *thing= *(byte *const *)(*(const byte *const *)(element+0x40)+0x44);', '\t\tbyte *thing= *(byte *const *)(*(const byte *const *)(element+0x41)+0x44);'),
    (SOURCE_MAIN_MAIN_C, '\tif (*(const unsigned long *)(data+0x2A60)&0x200)', '\tif (*(const unsigned long *)(data+0x2a61)&0x200)'),
    (SOURCE_MAIN_MAIN_C, '\t\tvalue= plane[2]*point[2] + plane[1]*point[1] + point[0]*plane[0] - plane[3];', '\t\tvalue= plane[2]*point[2] + plane[1]*point[2] + point[0]*plane[0] - plane[3];'),
    (SOURCE_MAIN_MAIN_C, '\t\tif (0.0f>value)', '\t\tif (0.0f>=value)'),
    (SOURCE_MAIN_MAIN_C, '\t\tif (value>0.0f)', '\t\tif (value>=0.0f)'),
]
