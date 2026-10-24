# mutations of milestone 793, written by make_mutations.py

TESTS = "test_small_793"

SOURCE_MAIN_MAIN_C = "source/main/main.c"

MUTATIONS = [
    (SOURCE_MAIN_MAIN_C, '\tmain_game_globals->unknown11FA= 2;', '\tmain_game_globals->unknown11FA= 3;'),
    (SOURCE_MAIN_MAIN_C, '\tif (location.cluster_index==NONE)', '\tif (location.cluster_index!=NONE)'),
    (SOURCE_MAIN_MAIN_C, '\tif (location.cluster_index==NONE)\n\t{\n\t\tmain_game_globals->unknown11FA= 0;\n\t\treturn;\n\t}', '\tif (location.cluster_index==NONE)\n\t{\n\t\treturn;\n\t}'),
    (SOURCE_MAIN_MAIN_C, '\t\t(const real_point3d *)(*(const byte *const *)((const byte *)global_scenario+0x1EC)+index*0x40+0x28));', '\t\t(const real_point3d *)(*(const byte *const *)((const byte *)global_scenario+0x1EC)+index*0x40+0x29));'),
    (SOURCE_MAIN_MAIN_C, '\tindex= **(const short *const *)(tag+0x60);\n\tif (index==NONE)\n\t{', '\tindex= **(const short *const *)(tag+0x60);\n\tif (index!=NONE)\n\t{'),
    (SOURCE_MAIN_MAIN_C, '\tindex= **(const short *const *)(tag+0x60);', '\tindex= **(const short *const *)(tag+0x61);'),
    (SOURCE_MAIN_MAIN_C, '\tif (!data || *(const long *)(data+0x0C)==NONE)', '\tif (!data || *(const long *)(data+0xd)==NONE)'),
    (SOURCE_MAIN_MAIN_C, '\treturn *(const long *)(*(const byte *const *)(tag+0x08)+index*0x0C);', '\treturn *(const long *)(*(const byte *const *)(tag+0x9)+index*0x0C);'),
    (SOURCE_MAIN_MAIN_C, '\tshort index= PLAYER_UNKNOWN28(player_index);\n\n\tif (index!=NONE)\n\t{\n\t\tbyte *entry= unknown_004ed284+index*0x94+0x14;', '\tshort index= PLAYER_UNKNOWN28(player_index);\n\n\tif (index!=0)\n\t{\n\t\tbyte *entry= unknown_004ed284+index*0x94+0x14;'),
    (SOURCE_MAIN_MAIN_C, '\t\t\tentry[0x8F]= 1;', '\t\t\tentry[0x8F]= 2;'),
    (SOURCE_MAIN_MAIN_C, '\t\tbyte *entry= unknown_004ed284+index*0x94+0x14;', '\t\tbyte *entry= unknown_004ed284+index*0x94+0x15;'),
    (SOURCE_MAIN_MAIN_C, '\t\tif (entry[0x8E])', '\t\tif (entry[0x8f])'),
    (SOURCE_MAIN_MAIN_C, '\t\tunknown_000b58c0(*(const long *)(unit+0xD4), 0x800000);', '\t\tunknown_000b58c0(*(const long *)(unit+0xd5), 0x800000);'),
    (SOURCE_MAIN_MAIN_C, '\tunit_index= *(const long *)((const byte *)player_data->data+(player_index&0xFFFF)*0x21C+0x2C);', ''),
    (SOURCE_MAIN_MAIN_C, '\tif (*(const long *)(unit+0xD4)!=NONE)', '\tif (*(const long *)(unit+0xD4)==NONE)'),
    (SOURCE_MAIN_MAIN_C, '\tunit= (byte *)((const struct object_header_datum *)object_header_data->data)[unit_index&0xFFFF].datum;', ''),
    (SOURCE_MAIN_MAIN_C, '\tentry= unknown_00510c5c+index*0xA0+0x30;', '\tentry= unknown_00510c5c+index*0xA0+0x31;'),
    (SOURCE_MAIN_MAIN_C, '\tbyte *entry;\n\n\tif (index==NONE)\n\t{\n\t\treturn;', '\tbyte *entry;\n\n\tif (index!=NONE)\n\t{\n\t\treturn;'),
    (SOURCE_MAIN_MAIN_C, '\t\tfor (index_= 0; index_<8; index_++)', '\t\tfor (index_= 1; index_<8; index_++)'),
    (SOURCE_MAIN_MAIN_C, '\tentry[0x88]&= ~1;', '\tentry[0x88]&= ~2;'),
    (SOURCE_MAIN_MAIN_C, '\tshort index= PLAYER_UNKNOWN28(player_index);\n\n\tif (index!=NONE)\n\t{\n\t\tbyte *entry= unknown_00502120+index*0x88;', '\tshort index= PLAYER_UNKNOWN28(player_index);\n\n\tif (index!=0)\n\t{\n\t\tbyte *entry= unknown_00502120+index*0x88;'),
    (SOURCE_MAIN_MAIN_C, '\t\tbyte *entry= unknown_00502120+index*0x88;', '\t\tbyte *entry= unknown_00502120+index*0x89;'),
    (SOURCE_MAIN_MAIN_C, '\t\t\tempty= FALSE;', ''),
    (SOURCE_MAIN_MAIN_C, '\tfor (index= 0; index<unknown_00510c64; index++)', '\tfor (index= 1; index<unknown_00510c64; index++)'),
    (SOURCE_MAIN_MAIN_C, '\tboolean empty= TRUE;', '\tboolean empty= FALSE;'),
]
