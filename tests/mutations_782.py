# mutations of milestone 782, written by make_mutations.py

TESTS = "test_small_782"

SOURCE_RASTERIZER_RASTERIZER_XBOX_C = "source/rasterizer/rasterizer_xbox.c"
SOURCE_RASTERIZER_RASTERIZER_TEXT_C = "source/rasterizer/rasterizer_text.c"
SOURCE_CSERIES_UNKNOWN_0051F40C_C = "source/cseries/unknown_0051f40c.c"
SOURCE_RASTERIZER_RASTERIZER_C = "source/rasterizer/rasterizer.c"
SOURCE_INTERFACE_SCREEN_SPLIT_C = "source/interface/screen_split.c"
SOURCE_UNITS_UNITS_C = "source/units/units.c"
SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C = "source/game/game_unknown_0003f2b0.c"

MUTATIONS = [
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\t\tthree[0]= 0;', ''),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\t\tthree[1]= 0;', '\t\tthree[1]= 1;'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tdata[0x2D4]= TRUE;', '\tdata[0x2d5]= TRUE;'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tconst byte *element= (const byte *)(uintptr_t)*(const unsigned long *)(tag+0x08)+element_index*0x1C;', '\tconst byte *element= (const byte *)(uintptr_t)*(const unsigned long *)(tag+0x9)+element_index*0x1C;'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tconst byte *tag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;', '\tconst byte *tag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0x10000].address;'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tconst byte *entry= (const byte *)(uintptr_t)*(const unsigned long *)(element+0x08);', '\tconst byte *entry= (const byte *)(uintptr_t)*(const unsigned long *)(element+0x9);'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\t\tbits|= 1UL<<(entry[0]&31);', ''),
    (SOURCE_RASTERIZER_RASTERIZER_TEXT_C, '\t\t\tunknown_004b569d= TRUE;', '\t\t\tunknown_004b569d= FALSE;'),
    (SOURCE_RASTERIZER_RASTERIZER_TEXT_C, '\t\tif (unknown_004b5690 && !unknown_004b569d)', '\t\tif (unknown_004b5690 || !unknown_004b569d)'),
    (SOURCE_RASTERIZER_RASTERIZER_TEXT_C, '\t\tmemset(unknown_005093e0, 0, 0xFF*sizeof(long));', '\t\tmemset(unknown_005093e0, 1, 0xFF*sizeof(long));'),
    (SOURCE_CSERIES_UNKNOWN_0051F40C_C, '\tif (other==NONE)', '\tif (other==0)'),
    (SOURCE_CSERIES_UNKNOWN_0051F40C_C, '\t\tentry[1]|= 0xFF80;', '\t\tentry[1]|= 0xff81;'),
    (SOURCE_CSERIES_UNKNOWN_0051F40C_C, '\tif ((other<0 || other>last) && last!=other && other!=NONE)', '\tif ((other<0 || other>last) || last!=other && other!=NONE)'),
    (SOURCE_CSERIES_UNKNOWN_0051F40C_C, '\tmemset(entry, 0, 8*sizeof(unsigned long));', '\tmemset(entry, 1, 8*sizeof(unsigned long));'),
    (SOURCE_CSERIES_UNKNOWN_0051F40C_C, '\tunsigned long *entry= &unknown_0051f40c[8*index];', '\tunsigned long *entry= &unknown_0051f40c[9*index];'),
    (SOURCE_CSERIES_UNKNOWN_0051F40C_C, '\t\tunknown_005233f0[16*list+(index>>5)]&= ~(1UL<<(index&31));', '\t\tunknown_005233f0[16*list+(index>>6)]&= ~(1UL<<(index&31));'),
    (SOURCE_CSERIES_UNKNOWN_0051F40C_C, '\tmemset(entry, 0, 8*sizeof(unsigned long));\n\tentry[1]|= 0xFF80;\n}', '\tmemset(entry, 0, 8*sizeof(unsigned long));\n\tentry[2]|= 0xFF80;\n}'),
    (SOURCE_RASTERIZER_RASTERIZER_C, '\t\tvalue= *(const unsigned short *)(entries+0x06+4*index);', '\t\tvalue= *(const unsigned short *)(entries+0x06+5*index);'),
    (SOURCE_RASTERIZER_RASTERIZER_C, '\t\tresult[0x4C+index]= (byte)value;', '\t\tresult[0x4d+index]= (byte)value;'),
    (SOURCE_RASTERIZER_RASTERIZER_C, '\t\tif (value>2)', '\t\tif (value>3)'),
    (SOURCE_RASTERIZER_RASTERIZER_C, '\tlong sum= 0;', '\tlong sum= 1;'),
    (SOURCE_INTERFACE_SCREEN_SPLIT_C, '\tgrid[1]= (short)rows;', ''),
    (SOURCE_INTERFACE_SCREEN_SPLIT_C, '\tif (count>1)', '\tif (count>2)'),
    (SOURCE_INTERFACE_SCREEN_SPLIT_C, '\tboolean wide= mode==1;', '\tboolean wide= mode==2;'),
    (SOURCE_INTERFACE_SCREEN_SPLIT_C, '\t\t\tsize[1]= 2;', '\t\t\tsize[2]= 2;'),
    (SOURCE_INTERFACE_SCREEN_SPLIT_C, '\tsize[0]= 1;', ''),
    (SOURCE_INTERFACE_SCREEN_SPLIT_C, '\tcell= skipped+index;', ''),
    (SOURCE_INTERFACE_SCREEN_SPLIT_C, '\tif (count<grid[1]*grid[0])', '\tif (count<grid[1]*grid[1])'),
    (SOURCE_UNITS_UNITS_C, '\tif (((unknown_004ba014>>1)&1) && first)', '\tif (((unknown_004ba014>>2)&1) && first)'),
    (SOURCE_UNITS_UNITS_C, '\t\treturn 4;', '\t\treturn 5;'),
    (SOURCE_UNITS_UNITS_C, '\tif (((unknown_004ba014>>2)&1) && second)\n\t{\n\t\treturn 2;\n\t}\n', '\tif (((unknown_004ba014>>2)&1) && second)\n\t{\n\t\treturn 3;\n\t}\n'),
    (SOURCE_UNITS_UNITS_C, '\tif (((unknown_004ba014>>1)&1) && first)\n\t{\n\t\treturn 0;\n\t}\n\tif (((unknown_004ba014>>2)&1) && second)', '\tif (((unknown_004ba014>>1)&1) && first)\n\t{\n\t\treturn 1;\n\t}\n\tif (((unknown_004ba014>>2)&1) && second)'),
    (SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C, '\tif (value!=NONE && unknown_004b9ee9 && unknown_004b9eec!=NONE && global_structure_bsp_index!=NONE &&', '\tif (value!=NONE && unknown_004b9ee9 && unknown_004b9eec!=NONE && global_structure_bsp_index!=NONE ||'),
    (SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C, '\treturn FALSE;', '\treturn TRUE;'),
    (SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C, '\t\treturn TRUE;', '\t\treturn FALSE;'),
    (SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C, '\t\t((const short *)(uintptr_t)*(const unsigned long *)(structure_bsp+0xB0))[unknown_004b9eec]==value)', '\t\t((const short *)(uintptr_t)*(const unsigned long *)(structure_bsp+0xb1))[unknown_004b9eec]==value)'),
    # by hand: the size of each element of the block
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '+element_index*0x1C;', '+element_index*0x18;'),
    # taken out, none can be noticed: data[0x20C] is set again after the loop; other<=0 only
    # matters with no entries, when the index already returned; without columns= 1 the loop
    # still ends with the same grid (each c*(r+1) before it is below the count)
]
