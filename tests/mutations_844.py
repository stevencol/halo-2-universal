# mutations of milestone 844, written by make_mutations.py

TESTS = "test_small_844"

SOURCE_GAME_GAME_ENGINE_C = "source/game/game_engine.c"
SOURCE_PHYSICS_HAVOK_COMPONENT_C = "source/physics/havok_component.c"

MUTATIONS = [
    (SOURCE_GAME_GAME_ENGINE_C, '\t\tif (((long)(short)mask&(unsigned short)list->shorts8[index]) || (mask&0xFFFF)==0xFFFF)', '\t\tif (((long)(short)mask&(unsigned short)list->shorts8[index]) && (mask&0xFFFF)==0xFFFF)'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tlong index;\n\n\tfor (index= 0; index<list->count; index++)\n\t{\n\t\tif (((long)(short)mask&(unsigned short)list->shorts8[index]) || (mask&0xFFFF)==0xFFFF)', '\tlong index;\n\n\tfor (index= 0; index<list->count-1; index++)\n\t{\n\t\tif (((long)(short)mask&(unsigned short)list->shorts8[index]) || (mask&0xFFFF)==0xFFFF)'),
    (SOURCE_PHYSICS_HAVOK_COMPONENT_C, '\tunknown30_set(unknown, 0x2001, value);', '\tunknown30_set(unknown, 0x2002, value);'),
    (SOURCE_PHYSICS_HAVOK_COMPONENT_C, '\tunknown30_set(unknown, 0x2002, value);', '\tunknown30_set(unknown, 0x2003, value);'),
    (SOURCE_PHYSICS_HAVOK_COMPONENT_C, '\tunknown30_set(unknown, 0x2003, value);', '\tunknown30_set(unknown, 0x2004, value);'),
]
