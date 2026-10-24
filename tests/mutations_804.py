# mutations of milestone 804, written by make_mutations.py

TESTS = "test_small_804"

SOURCE_CSERIES_UNKNOWN_0051F40C_C = "source/cseries/unknown_0051f40c.c"
SOURCE_RASTERIZER_RASTERIZER_XBOX_C = "source/rasterizer/rasterizer_xbox.c"
SOURCE_CSERIES_CSERIES_C = "source/cseries/cseries.c"
SOURCE_EFFECTS_EFFECTS_C = "source/effects/effects.c"

MUTATIONS = [
    (SOURCE_CSERIES_UNKNOWN_0051F40C_C, '\t\t&& ((kind>=4 && kind<=7 && display==unknown_005234b0) || display>unknown_005234b0 || !unknown_005234b0))', '\t\t&& ((kind>=4 || kind<=7 && display==unknown_005234b0) || display>unknown_005234b0 || !unknown_005234b0))'),
    (SOURCE_CSERIES_UNKNOWN_0051F40C_C, '\t\tspecial= kind>=4 && kind<=7;', ''),
    # taken out: 0x3fe1/0x20 is the same count as 0x3FE0/0x20; reading 0x3D instead of 0x3C only changes whether an element far out of the table (0xFFFF) would be written, outside what the test compares
    (SOURCE_CSERIES_UNKNOWN_0051F40C_C, '\t\tunknown_005234b0= display;', ''),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\t((long *)(data+0x8C))[index]= fourth;', ''),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tif (three[1]!=second || three[0]!=first || three[2]!=third)', '\tif (three[1]!=second || three[1]!=first || three[2]!=third)'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\t\tdata[0x2D4]= TRUE;', '\t\tdata[0x2D4]= FALSE;'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tlong index= *(const long *)(data+0x2D0);\n\tlong *three= (long *)(data+0x210)+3*index;\n', '\tlong index= *(const long *)(data+0x2D0);\n\tlong *three= (long *)(data+0x211)+3*index;\n'),
    (SOURCE_CSERIES_CSERIES_C, '\tunsigned long *bits= (unsigned long *)(iterator+4);', '\tunsigned long *bits= (unsigned long *)(iterator+5);'),
    (SOURCE_CSERIES_CSERIES_C, '\t\t\tif (*index>=*count)', '\t\t\tif (*index>*count)'),
    (SOURCE_CSERIES_CSERIES_C, '\tshort *index= (short *)(iterator+2);', '\tshort *index= (short *)(iterator+3);'),
    (SOURCE_CSERIES_CSERIES_C, '\t*bits&= ~(1UL<<which);\n\treturn TRUE;\n}', '\t*bits&= ~(1UL<<which);\n\treturn FALSE;\n}'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tif (*first==NONE)', '\tif (*first!=NONE)'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tif (*last!=NONE)', '\tif (*last!=0)'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tif (*(const long *)(element+0x38)!=NONE)', '\tif (*(const long *)(element+0x38)==NONE)'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tif (*last==index)', '\tif (*last!=index)'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tif (*first==index)', '\tif (*first!=index)'),
]
