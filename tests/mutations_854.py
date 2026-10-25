# mutations of milestone 854, written by make_mutations.py

TESTS = "test_small_854"

SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_RASTERIZER_RASTERIZER_XBOX_C = "source/rasterizer/rasterizer_xbox.c"

MUTATIONS = [
    (SOURCE_PHYSICS_HAVOK_C, '\t\tshort next= *(const short *)(animation+0x32);', '\t\tshort next= *(const short *)(animation+0x33);'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tgraph= unknown4C_get_tag(inherited, graph);', ''),
    (SOURCE_PHYSICS_HAVOK_C, '\tif (inherited!=NONE)', '\tif (inherited!=0)'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tanimation= *(const byte *const *)(graph+0x30)+next*0x6C;', '\t\tanimation= *(const byte *const *)(graph+0x31)+next*0x6C;'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tfor (index= 0; index<(*range>>9); index++, entry+= 4)', '\tfor (index= 0; index<(*range>>9); index++, entry+= 5)'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\t\tif (source[0]!=NONE)', '\t\tif (source[0]!=0)'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tconst byte *entry= *(const byte *const *)(*(const byte *const *)(state+0x10)+0x24)+(*range&0x1FF)*4;', '\tconst byte *entry= *(const byte *const *)(*(const byte *const *)(state+0x10)+0x24)+(*range&0x200)*4;'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tconst byte *values= *(const byte *const *)(*(const byte *const *)(state+0x0C)+0x08);', '\tconst byte *values= *(const byte *const *)(*(const byte *const *)(state+0x0C)+0x9);'),
]
