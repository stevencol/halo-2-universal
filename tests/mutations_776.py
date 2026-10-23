# mutations of milestone 776: one or two lines of each function

TESTS = "test_small_776"

FILE = "source/structures/structure_lightmap_predict.c"

MUTATIONS = [
    (FILE, "*(const long *)(lightmap+STRUCTURE_LIGHTMAP_GROUP_COUNT_OFFSET)<=0", "*(const long *)(lightmap+STRUCTURE_LIGHTMAP_GROUP_COUNT_OFFSET)<0"),
    (FILE, "|| !global_structure_bsp)", ")"),
    (FILE, "\tif (*(const long *)(group+STRUCTURE_LIGHTMAP_GROUP_BITMAP_TAG_OFFSET)==NONE ||", "\tif ("),
    (FILE, "\t\t*(const long *)(group+STRUCTURE_LIGHTMAP_GROUP_CHECKSUM_OFFSET)!=", "\t\t*(const long *)(group+STRUCTURE_LIGHTMAP_GROUP_CHECKSUM_OFFSET+4)!="),
    (FILE, "\tif (bitmap_index==NONE)\n\t{\n\t\treturn TRUE;", "\tif (bitmap_index==NONE)\n\t{\n\t\treturn FALSE;"),
    (FILE, "\t\tbitmap_index*BITMAP_DATA_SIZE));", "\t\tbitmap_index*0x70));"),
    (FILE, "(group+block_offset)+index*12+2);", "(group+block_offset)+index*12);"),
    (FILE, "\t\tgeometry_index*STRUCTURE_LIGHTMAP_GEOMETRY_SIZE+STRUCTURE_LIGHTMAP_GEOMETRY_INFO_OFFSET));", "\t\tgeometry_index*STRUCTURE_LIGHTMAP_GEOMETRY_SIZE));"),
    (FILE, "\tif (!group)\n\t{\n\t\treturn TRUE;\n\t}\n\tgeometry_index", "\tif (!group)\n\t{\n\t\treturn FALSE;\n\t}\n\tgeometry_index"),
    (FILE, "*(const byte *const *)(model+0x144)+part_index*0x58+0x34);", "*(const byte *const *)(model+0x144)+part_index*0x58+0x36);"),
    (FILE, "(model+0x13C)+section_index*0xC8+0x28));", "(model+0x13C)+section_index*0xC8+0x24));"),
    (FILE, "\treturn structure_lightmap_predict_bitmap(0x4C, index);", "\treturn structure_lightmap_predict_bitmap(0x2C, index);"),
    (FILE, "\treturn structure_lightmap_predict_geometry(0x54, index);", "\treturn structure_lightmap_predict_geometry(0x64, index);"),
]
