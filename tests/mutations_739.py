# mutations of milestone 739: one or two lines of each function

TESTS = "test_small_739"

DEBUG = "source/rasterizer/rasterizer_debug.c"
RASTERIZER = "source/rasterizer/rasterizer.c"
XBOX = "source/rasterizer/rasterizer_xbox.c"
GAME = "source/game/game_unknown_0002b4a0.c"
REAL = "source/math/real_math.c"
MATRIX = "source/math/matrix_math.c"
UNITS = "source/units/units.c"
GAME2 = "source/game/game_unknown_0003f2b0.c"
DECALS = "source/effects/decals.c"

MUTATIONS = [
    (DEBUG, "\tif (unknown_004b6280<0x400)", "\tif (unknown_004b6280<0x3FF)"),
    (DEBUG, "\t\tentry[0xC]= 0;", "\t\tentry[0xD]= 0;"),
    (DEBUG, "\t\tunknown_004b6291= 0;\n\t}\n\n\treturn NULL;", "\t\tunknown_004b6291= 1;\n\t}\n\n\treturn NULL;"),
    (DEBUG, "\treturn first[1]==second[1] && first[0]==second[0];", "\treturn (first[1]&~1)==(second[1]&~1) && first[0]==second[0];"),
    (RASTERIZER, "\tif (lightmap==NULL || *(long *)(lightmap + 0x80)<=0 || structure==NULL)", "\tif (lightmap==NULL || *(long *)(lightmap + 0x80)<0 || structure==NULL)"),
    (RASTERIZER, "\treturn *(long *)(other + 0x1C)!=NONE && *(long *)(other + 4)==*(long *)(structure + 8);", "\treturn *(long *)(other + 4)==*(long *)(structure + 8);"),
    (RASTERIZER, "\t\t&& unknown_00485648[2]==480 && unknown_004ba04c<=1;", "\t\t&& unknown_00485648[2]==480 && unknown_004ba04c<1;"),
    (RASTERIZER, "\treturn unknown_00485648[1]==0 && unknown_00485648[0]==0 && unknown_00485648[3]==640", "\treturn unknown_00485648[1]==0 && unknown_00485648[3]==640"),
    (XBOX, "\t\treturn 20;", "\t\treturn 18;"),
    (XBOX, "\tif (*(long *)(rasterizer_display_entries[18] + 0x88)!=0 && rasterizer_display_entries[18][0x95]==0)", "\tif (*(long *)(rasterizer_display_entries[18] + 0x88)!=0)"),
    (GAME, "\t*(long *)unknown_004b9970= NONE;\n\tunknown_005093fc= 0;\n}\n\n/* 0x0003ba90", "\tunknown_005093fc= 0;\n}\n\n/* 0x0003ba90"),
    (REAL, "\treturn z*z + x*x + y*y;", "\treturn z*z + x*x + x*y;"),
    (MATRIX, "\t\t\tmatrix->n[column][row]= rows[row*4 + column];", "\t\t\tmatrix->n[column][row]= rows[column*3 + row];"),
    (MATRIX, "\t}\n\tmatrix->scale= 1.0f;", "\t}\n\tmatrix->scale= 0.0f;"),
    (UNITS, "\t\tunknown_004ba014|= 1<<bit;", "\t\tunknown_004ba014^= 1<<bit;"),
    (UNITS, "\t\tunknown_004ba014&= ~(1<<bit);", "\t\tunknown_004ba014&= ~(2<<bit);"),
    (UNITS, "\treturn ((unknown_004ba014>>4)&1) && !((unknown_004ba014>>5)&1) ? 0 : 1;", "\treturn ((unknown_004ba014>>4)&1) ? 0 : 1;"),
    (UNITS, "\tif (!second)\n\t{\n\t\treturn 2;", "\tif (!second)\n\t{\n\t\treturn 3;"),
    (GAME2, "\treturn second ? 2 : NONE;", "\treturn second ? 2 : 3;"),
    (GAME2, "\tif (something==NULL)\n\t{\n\t\treturn 0;", "\tif (something==NULL)\n\t{\n\t\treturn 1;"),
    (DECALS, "\treturn unknown_00480118;", "\treturn unknown_00480118+1;"),
]
