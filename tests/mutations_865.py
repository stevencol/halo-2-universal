# mutations of milestone 865, written by make_mutations.py

TESTS = "test_small_865"

SOURCE_UNITS_UNITS_C = "source/units/units.c"
SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"
SOURCE_EFFECTS_EFFECTS_C = "source/effects/effects.c"

MUTATIONS = [
    (SOURCE_UNITS_UNITS_C, '\t\tcase 0x0E00002A: case 0x0E00003B: case 0x0E0000C3: case 0x0F00003A:', '\t\tcase 0x0E00002A: case 0x0E00003B: case 0x0E0000C3: case 0xf00003b:'),
    (SOURCE_UNITS_UNITS_C, '\tif (*(const long *)(unit+0x14)==NONE || *(const short *)(unit+0x1FC)==NONE)', '\tif (*(const long *)(unit+0x14)==NONE || *(const short *)(unit+0x1fd)==NONE)'),
    (SOURCE_UNITS_UNITS_C, '\t{\n\t\tresult= FALSE;\n\t}', '\t{\n\t\tresult= TRUE;\n\t}'),
    (SOURCE_UNITS_UNITS_C, '\t\tcase 0x050000C3: case 0x05000768: case 0x05000769: case 0x0500076A: case 0x0500076B:', '\t\tcase 0x050000C3: case 0x05000768: case 0x500076a: case 0x0500076A: case 0x0500076B:'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tplacement[0x14]= 0;', '\tplacement[0x14]= 1;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tif ((1UL<<(object[0xAA]&31))&3)', '\t\tif ((1UL<<(object[0xAA]&31))&4)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tobject= object_try_and_get_and_verify_type(object_index, NONE);', ''),
    (SOURCE_OBJECTS_OBJECTS_C, '\tplacement[0xB8]= 0;', '\tplacement[0xB8]= 1;'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tcluster_index= *(const short *)(source+0x20);', ''),
    (SOURCE_EFFECTS_EFFECTS_C, '\tfor (index= 0; index<4; index++)', '\tfor (index= 1; index<4; index++)'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tlower= *(const real *)(definition+0x28);', ''),
    (SOURCE_EFFECTS_EFFECTS_C, '\tdefinition= (const byte *)(uintptr_t)global_tag_instances[*(const long *)(parameters+0x04)&0xFFFF].address;', '\tdefinition= (const byte *)(uintptr_t)global_tag_instances[*(const long *)(parameters+0x5)&0xFFFF].address;'),
]
