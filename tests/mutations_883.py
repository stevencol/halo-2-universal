# mutations of milestone 883, written by make_mutations.py

TESTS = "test_small_883"

SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"

MUTATIONS = [
    (SOURCE_PHYSICS_HAVOK_C, '\tif (main_body>=0 && main_body<*(const long *)(component+0x74))', '\tif (main_body>=0 || main_body<*(const long *)(component+0x74))'),
    (SOURCE_PHYSICS_HAVOK_C, '{\n\tsigned char main_body= (signed char)component[0x18];\n\treal_matrix4x3 matrix;', '{\n\tsigned char main_body= (signed char)component[0x19];\n\treal_matrix4x3 matrix;'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tobject_get_world_matrix(*(const long *)(component+0x08), &matrix);', '\t\tobject_get_world_matrix(*(const long *)(component+0x9), &matrix);'),
]
