# mutations of milestone 871, written by make_mutations.py

TESTS = "test_small_871"

SOURCE_UNITS_UNITS_C = "source/units/units.c"
SOURCE_RASTERIZER_RASTERIZER_XBOX_C = "source/rasterizer/rasterizer_xbox.c"

MUTATIONS = [
    (SOURCE_UNITS_UNITS_C, '\tdata[0]= *(const long *)(model+0x90);', '\tdata[1]= *(const long *)(model+0x90);'),
    (SOURCE_UNITS_UNITS_C, '\t\tif (*(const long *)(variant+0x34)!=NONE && data[0]==NONE)\n\t\t{\n\t\t\tunit_unknown342_set_tag(unit_index, *(const long *)(variant+0x34));\n\t\t\tif (value && value!=NONE)\n\t\t\t{\n\t\t\t\tdata[2]= value;\n\t\t\t}', '\t\tif (*(const long *)(variant+0x34)!=NONE && data[0]==NONE)\n\t\t{\n\t\t\tunit_unknown342_set_tag(unit_index, *(const long *)(variant+0x34));\n\t\t\tif (value && value==NONE)\n\t\t\t{\n\t\t\t\tdata[2]= value;\n\t\t\t}'),
    (SOURCE_UNITS_UNITS_C, '\t\tif (*(const long *)variant==name && *(const long *)(variant+0x34)!=NONE)', '\t\tif (*(const long *)variant!=name && *(const long *)(variant+0x34)!=NONE)'),
    (SOURCE_UNITS_UNITS_C, '\t\tif (*(const long *)(variant+0x34)!=NONE && data[0]==NONE)', '\t\tif (*(const long *)(variant+0x34)!=NONE && data[0]!=NONE)'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tbounds[1]= (short)((unsigned short)place[0]*step_x+rasterizer_clip_unknown_00485a94);', '\tbounds[2]= (short)((unsigned short)place[0]*step_x+rasterizer_clip_unknown_00485a94);'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tshort step_y= (short)((rasterizer_clip_unknown_00485a96-rasterizer_clip_unknown_00485a92)/grid[1]);', '\tshort step_y= (short)((rasterizer_clip_unknown_00485a96-rasterizer_clip_unknown_00485a92)/grid[2]);'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\t\tinner[1]+= 4;', '\t\tinner[1]+= 5;'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\t\touter[2]= rasterizer_clip_unknown_00485a8e;', ''),
]
