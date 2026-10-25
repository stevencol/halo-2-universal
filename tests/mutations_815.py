# mutations of milestone 815, written by make_mutations.py

TESTS = "test_small_815"

SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"
SOURCE_UNITS_UNITS_C = "source/units/units.c"
SOURCE_RASTERIZER_RASTERIZER_DEBUG_C = "source/rasterizer/rasterizer_debug.c"

MUTATIONS = [
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tfor (index= 0; index<*(const long *)(bsp+0x22C); index++)', '\t\tfor (index= 0; index<*(const long *)(bsp+0x22d); index++)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\t\t\t&& (element[7] || *(const short *)(element+4)==*(const short *)(object+0xA8)))', '\t\t\t\t&& (element[7] || *(const short *)(element+4)!=*(const short *)(object+0xA8)))'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\t\tif (element[6]==object[0xAA] && element[7]==object[0xAB] && *(const long *)element==*(const long *)(object+0xA4)', '\t\t\tif (element[6]!=object[0xAA] && element[7]==object[0xAB] && *(const long *)element==*(const long *)(object+0xA4)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\t\tconst byte *element= *(const byte *const *)(bsp+0x230)+index*0x5C;', '\t\t\tconst byte *element= *(const byte *const *)(bsp+0x230)+index*0x5d;'),
    (SOURCE_UNITS_UNITS_C, '\tlong place= *(const short *)(definition+0x06)+(signed char)definition[1];', '\tlong place= *(const short *)(definition+0x7)+(signed char)definition[1];'),
    (SOURCE_UNITS_UNITS_C, '\tpoint[1]= ((const unsigned long *)global_zero_vector3d)[1];', '\tpoint[1]= ((const unsigned long *)global_zero_vector3d)[2];'),
    (SOURCE_UNITS_UNITS_C, '\tpoint[2]= ((const unsigned long *)global_zero_vector3d)[2];', '\tpoint[2]= ((const unsigned long *)global_zero_vector3d)[3];'),
    (SOURCE_UNITS_UNITS_C, '\tpoint[0]= ((const unsigned long *)global_zero_vector3d)[0];', '\tpoint[1]= ((const unsigned long *)global_zero_vector3d)[0];'),
    (SOURCE_RASTERIZER_RASTERIZER_DEBUG_C, '\t\t\tunknown_004b6291= FALSE;', '\t\t\tunknown_004b6291= TRUE;'),
    (SOURCE_RASTERIZER_RASTERIZER_DEBUG_C, '\tx= point[0]-unknown_00485618[0];', '\tx= point[0]-unknown_00485618[1];'),
    (SOURCE_RASTERIZER_RASTERIZER_DEBUG_C, '\tplane= unknown_004b6288+unknown_004b6280*0x70;', ''),
    (SOURCE_RASTERIZER_RASTERIZER_DEBUG_C, '\t((unsigned long *)(plane+0x64))[1]= ((const unsigned long *)point)[1];', '\t((unsigned long *)(plane+0x64))[1]= ((const unsigned long *)point)[2];'),
]
