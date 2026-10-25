# mutations of milestone 834, written by make_mutations.py

TESTS = "test_small_834"

SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_RASTERIZER_RASTERIZER_XBOX_C = "source/rasterizer/rasterizer_xbox.c"

MUTATIONS = [
    (SOURCE_PHYSICS_HAVOK_C, '\tdata[0x1C]= 0;', '\tdata[0x1d]= 0;'),
    (SOURCE_PHYSICS_HAVOK_C, '\tdata[0x19]= 0xFF;', ''),
    (SOURCE_PHYSICS_HAVOK_C, '\tdata[0x1B]= 0;', ''),
    (SOURCE_PHYSICS_HAVOK_C, '\tfor (index= 0; index<3; index++)', '\tfor (index= 1; index<3; index++)'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tunknown_00485b28[4]= 4.0f/height;', ''),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tunknown_00485b28[3]= 2.0f/height;', '\tunknown_00485b28[4]= 2.0f/height;'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tunknown_00485b28[6]= 1.0f/height;', '\tunknown_00485b28[7]= 1.0f/height;'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tunknown_00485b28[0]= 1.0f/height;', ''),
]
