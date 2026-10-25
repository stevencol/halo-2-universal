# mutations of milestone 836, written by make_mutations.py

TESTS = "test_small_836"

SOURCE_RASTERIZER_RASTERIZER_TEXT_C = "source/rasterizer/rasterizer_text.c"
SOURCE_HS_HS_RUNTIME_C = "source/hs/hs_runtime.c"
SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"
SOURCE_EFFECTS_EFFECTS_C = "source/effects/effects.c"

MUTATIONS = [
    (SOURCE_RASTERIZER_RASTERIZER_TEXT_C, '\tunknown_005093e0= game_state_malloc(0x3FC);', ''),
    (SOURCE_RASTERIZER_RASTERIZER_TEXT_C, '\t\tmemset(unknown_005093e0, 0, 0x3FC);', '\t\tmemset(unknown_005093e0, 1, 0x3FC);'),
    (SOURCE_RASTERIZER_RASTERIZER_TEXT_C, '\tdata[0x00]= 0;', ''),
    (SOURCE_RASTERIZER_RASTERIZER_TEXT_C, '\tdata[0x1A]= 0;', '\tdata[0x1A]= 1;'),
    (SOURCE_RASTERIZER_RASTERIZER_TEXT_C, '\tdata[0x18]= 1;', '\tdata[0x18]= 2;'),
    (SOURCE_RASTERIZER_RASTERIZER_TEXT_C, '\tdata[0x01]= 0;', ''),
    (SOURCE_HS_HS_RUNTIME_C, '\tthread= (byte *)hs_thread_data->data+(thread_index&0xFFFF)*0x418;', '\tthread= (byte *)hs_thread_data->data+(thread_index&0xFFFF)*0x419;'),
    (SOURCE_HS_HS_RUNTIME_C, '\tif (thread_index==NONE)', '\tif (thread_index!=NONE)'),
    (SOURCE_HS_HS_RUNTIME_C, '\tthread[3]= 0;', '\tthread[4]= 0;'),
    (SOURCE_HS_HS_RUNTIME_C, '\tstack= thread+0x18;', ''),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tif (parent_index!=NONE && ((definition[2]>>4)&1))', '\t\tif (parent_index!=0 && ((definition[2]>>4)&1))'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tif ((object[4]&1) && ((1UL<<(object[0xAA]&0x1F))&0x1C) && object_unknown_0010cf50(object_index))', '\t\tif ((object[4]&1) || ((1UL<<(object[0xAA]&0x1F))&0x1C) && object_unknown_0010cf50(object_index))'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tlong parent_index= *(const long *)(object+0x14);', '\t\tlong parent_index= *(const long *)(object+0x15);'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\t\treturn *(const long *)(object+0x154);', '\t\t\treturn *(const long *)(object+0x155);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\tif (*(const short *)(node+0x06)!=NONE)', '\t\tif (*(const short *)(node+0x06)==NONE)'),
    (SOURCE_EFFECTS_EFFECTS_C, '\twhile (child>=0 && child<*(const long *)(data+0x48) && child<count)', '\twhile (child>=0 && child<=*(const long *)(data+0x48) && child<count)'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tlong child= *(const short *)(*(const byte *const *)(data+0x4C)+node_index*0x60+0x06);', '\tlong child= *(const short *)(*(const byte *const *)(data+0x4C)+node_index*0x61+0x06);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\tconst byte *node= *(const byte *const *)(data+0x4C)+child*0x60;', '\t\tconst byte *node= *(const byte *const *)(data+0x4d)+child*0x60;'),
]
