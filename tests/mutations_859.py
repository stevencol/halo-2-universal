# mutations of milestone 859, written by make_mutations.py

TESTS = "test_small_859"

SOURCE_UNITS_UNITS_C = "source/units/units.c"
SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"

MUTATIONS = [
    (SOURCE_UNITS_UNITS_C, '\t\t\tkept= unit_unknown_000d5990(datum[0x2E], *(const long *)(entry+0x04));', '\t\t\tkept= unit_unknown_000d5990(datum[0x2E], *(const long *)(entry+0x5));'),
    (SOURCE_UNITS_UNITS_C, '\treturn creatable && (kept || other);', '\treturn creatable && (kept && other);'),
    (SOURCE_UNITS_UNITS_C, '\t\tif (entry && *(const long *)(entry+0x04)!=NONE)', '\t\tif (entry && *(const long *)(entry+0x04)!=0)'),
    (SOURCE_UNITS_UNITS_C, '\tboolean kept= FALSE;', '\tboolean kept= TRUE;'),
    (SOURCE_UNITS_UNITS_C, '\treturn *(const short *)(definition+0x1FE)!=0;', '\treturn *(const short *)(definition+0x1FE)!=1;'),
    (SOURCE_UNITS_UNITS_C, '\tif (weapon_unknown_00100880(weapon_index, NONE))\n\t{\n\t\treturn FALSE;\n\t}\n', '\tif (weapon_unknown_00100880(weapon_index, NONE))\n\t{\n\t\treturn TRUE;\n\t}\n'),
    (SOURCE_UNITS_UNITS_C, '\tif (weapon_unknown_00100880(weapon_index, NONE))', '\tif (weapon_unknown_00100880(weapon_index, 0))'),
    (SOURCE_UNITS_UNITS_C, '\tif (unit_channel_animation_has_bit(UNIT_UNKNOWN33E(unit)+0x9C, bit))', '\tif (unit_channel_animation_has_bit(UNIT_UNKNOWN33E(unit)+0x9d, bit))'),
    (SOURCE_UNITS_UNITS_C, '\tif (unit_channel_animation_has_bit(UNIT_UNKNOWN33E(unit)+0x9C, bit))\n\t{\n\t\treturn FALSE;\n\t}\n', '\tif (unit_channel_animation_has_bit(UNIT_UNKNOWN33E(unit)+0x9C, bit))\n\t{\n\t\treturn TRUE;\n\t}\n'),
    (SOURCE_PHYSICS_HAVOK_C, '\tdata[0x08]= animation[0x13];', '\tdata[0x08]= animation[0x14];'),
    (SOURCE_PHYSICS_HAVOK_C, '\tconst byte *animation;\n\n\tif ((short)(id>>16)==NONE)\n\t{\n\t\treturn;', '\tconst byte *animation;\n\n\tif ((short)(id>>17)==NONE)\n\t{\n\t\treturn;'),
    (SOURCE_PHYSICS_HAVOK_C, '\tdata[0x09]= animation[0x11];', '\tdata[0x09]= animation[0x12];'),
    (SOURCE_PHYSICS_HAVOK_C, '\tanimation= havok_unknown_001daea0(graph, id);', ''),
]
