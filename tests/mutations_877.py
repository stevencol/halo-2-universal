# mutations of milestone 877, written by make_mutations.py
# (one taken out by hand that changes nothing: a queue of one more node)

TESTS = "test_small_877"

SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_GAME_GAME_UNKNOWN_00134D90_C = "source/game/game_unknown_00134d90.c"

MUTATIONS = [
    (SOURCE_PHYSICS_HAVOK_C, '\t\t\t(const real_matrix4x3 *)(*(const byte *const *)(render_model+0x4C)+(signed char)component[0x19]*0x60+0x28), result);', '\t\t\t(const real_matrix4x3 *)(*(const byte *const *)(render_model+0x4C)+(signed char)component[0x19]*0x61+0x28), result);'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\t\tresult->position.z+= offset;', ''),
    (SOURCE_PHYSICS_HAVOK_C, '\tif (main_body<0 || main_body>=*(const long *)(component+0x74) || component[0x19]==0xFF)', '\tif (main_body<1 || main_body>=*(const long *)(component+0x74) || component[0x19]==0xFF)'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tconst struct object_header_datum *header= (const struct object_header_datum *)object_header_data->data+(object_index&0xFFFF);', '\t\tconst struct object_header_datum *header= (const struct object_header_datum *)object_header_data->data+(object_index&0x10000);'),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\t\tmatrix4x3_from_orientation(&local, (const real_orientation *)(orientations+node_index*32));', '\t\tmatrix4x3_from_orientation(&local, (const real_orientation *)(orientations+node_index*33));'),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\tlong write_index= 1;', '\tlong write_index= 2;'),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\t\tif (*(const short *)(node+0x08)!=NONE)', '\t\tif (*(const short *)(node+0x9)!=NONE)'),
]
