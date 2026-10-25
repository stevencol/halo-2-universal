# mutations of milestone 826, written by make_mutations.py

TESTS = "test_small_826"

SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_MATH_REAL_MATH_C = "source/math/real_math.c"
SOURCE_CACHE_CACHE_FILES_C = "source/cache/cache_files.c"
SOURCE_EFFECTS_EFFECTS_C = "source/effects/effects.c"

MUTATIONS = [
    (SOURCE_PHYSICS_HAVOK_C, '\t\t\thigh= middle-1;', ''),
    (SOURCE_PHYSICS_HAVOK_C, '\t\t\tlow= middle+1;', '\t\t\tlow= middle+2;'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\t\tfound= element;', ''),
    (SOURCE_PHYSICS_HAVOK_C, '\twhile (!last && low<=high)', '\twhile (!last && low<high)'),
    (SOURCE_PHYSICS_HAVOK_C, '\tif (index!=NONE)', '\tif (index==NONE)'),
    (SOURCE_PHYSICS_HAVOK_C, '\tlong tag= *(const long *)(data+0x1C);', '\tlong tag= *(const long *)(data+0x1d);'),
    (SOURCE_PHYSICS_HAVOK_C, '\tif (tag!=NONE)', '\tif (tag!=0)'),
    (SOURCE_PHYSICS_HAVOK_C, '\tshort index= *(const short *)(data+0x18);', '\tshort index= *(const short *)(data+0x19);'),
    (SOURCE_MATH_REAL_MATH_C, '\treal square= vector[0]*vector[0]+vector[1]*vector[1]+vector[2]*vector[2];', '\treal square= vector[1]*vector[0]+vector[1]*vector[1]+vector[2]*vector[2];'),
    (SOURCE_MATH_REAL_MATH_C, '\t\tvector[2]= (real)(scale*vector[2]);', '\t\tvector[2]= (real)(scale*vector[3]);'),
    (SOURCE_MATH_REAL_MATH_C, '\t\tvector[0]= (real)(scale*vector[0]);', '\t\tvector[0]= (real)(scale*vector[1]);'),
    (SOURCE_MATH_REAL_MATH_C, '\t\tvector[1]= (real)(scale*vector[1]);', '\t\tvector[1]= (real)(scale*vector[2]);'),
    (SOURCE_CACHE_CACHE_FILES_C, '\tlong last= 0x80061000+(globals[0] ? *(const long *)(globals+0x24) : 0);', '\tlong last= 0x80061000+(globals[1] ? *(const long *)(globals+0x24) : 0);'),
    (SOURCE_CACHE_CACHE_FILES_C, '\treturn address>=(long)0x80061000 && address<=last;', '\treturn address>=(long)0x80061001 && address<=last;'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tlong index= (long)*(const short *)location<<16 | (long)(location-(byte *)particle_location_data->data)/0x34;', '\tlong index= (long)*(const short *)location<<16 | (long)(location-(byte *)particle_location_data->data)/0x35;'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\t*first= index;\n\t}\n\tif (*last!=NONE)\n\t{\n\t\t*(long *)((byte *)particle_location_data->data+(*last&0xFFFF)*0x34+0x0C)= index;', '\t\t*first= index;\n\t}\n\tif (*last!=0)\n\t{\n\t\t*(long *)((byte *)particle_location_data->data+(*last&0xFFFF)*0x34+0x0C)= index;'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t*(long *)(location+0x0C)= NONE;\n\tif (*first==NONE)\n\t{', '\t*(long *)(location+0x0C)= NONE;\n\tif (*first!=NONE)\n\t{'),
]
