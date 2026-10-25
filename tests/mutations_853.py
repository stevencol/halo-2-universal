# mutations of milestone 853, written by make_mutations.py

TESTS = "test_small_853"

SOURCE_UNITS_UNITS_C = "source/units/units.c"
SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"
SOURCE_GAME_GAME_ENGINE_C = "source/game/game_engine.c"

MUTATIONS = [
    (SOURCE_UNITS_UNITS_C, '\tif ((*(const unsigned long *)(biped+0x134)>>23)&1)', '\tif ((*(const unsigned long *)(biped+0x134)>>24)&1)'),
    # taken out: when the change is the crouch both give it
    (SOURCE_UNITS_UNITS_C, '\treal speed= *(const real *)(UNIT_DEFINITION_GET(biped)+0x250);', '\treal speed= *(const real *)(UNIT_DEFINITION_GET(biped)+0x251);'),
    (SOURCE_UNITS_UNITS_C, '\t\t\tchange= 1.0f-crouch;', ''),
    (SOURCE_OBJECTS_OBJECTS_C, '\treal left_x= up[2]*forward[1]-forward[2]*up[1];', '\treal left_x= up[2]*forward[1]-forward[3]*up[1];'),
    (SOURCE_OBJECTS_OBJECTS_C, '\treal up_x= forward[2]*left_y-left_z*forward[1];', '\treal up_x= forward[2]*left_y-left_z*forward[2];'),
    (SOURCE_OBJECTS_OBJECTS_C, '\treal left_z= forward[0]*up[1]-up[0]*forward[1];', '\treal left_z= forward[0]*up[2]-up[0]*forward[1];'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tup[0]= up_x;', '\tup[1]= up_x;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tfor (index= 0; index<4; index++)', '\tfor (index= 0; index<5; index++)'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\tcolors[0]= *player_color_default;', ''),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\tplayer_color_get(&color, appearance[0]);', '\t\tplayer_color_get(&color, appearance[1]);'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\tcolors[1]= *player_color_default;', ''),
]
