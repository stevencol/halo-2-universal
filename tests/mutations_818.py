# mutations of milestone 818, written by make_mutations.py

TESTS = "test_small_818"

SOURCE_CACHE_CACHE_FILES_C = "source/cache/cache_files.c"
SOURCE_MAIN_MAIN_C = "source/main/main.c"

MUTATIONS = [
    (SOURCE_CACHE_CACHE_FILES_C, '\treturn table+(cache_files_language()<<6)+0x10;', '\treturn table+(cache_files_language()<<7)+0x10;'),
    (SOURCE_CACHE_CACHE_FILES_C, '\treturn table+(cache_files_language()<<8)+0x250;', '\treturn table+(cache_files_language()<<9)+0x250;'),
    (SOURCE_CACHE_CACHE_FILES_C, '\treturn table+(cache_files_language()<<6)+0x0C;', '\treturn table+(cache_files_language()<<7)+0x0C;'),
    (SOURCE_MAIN_MAIN_C, '\tif (havok_unknown_00147090()!=unknown_0055e504)', '\tif (havok_unknown_00147090()==unknown_0055e504)'),
    (SOURCE_MAIN_MAIN_C, '\t\tunknown_0055e4fc= TRUE;', '\t\tunknown_0055e4fc= FALSE;'),
    (SOURCE_MAIN_MAIN_C, '\t\tdata[0xAE]= TRUE;', ''),
    (SOURCE_MAIN_MAIN_C, '\tunknown_0055e508= 0;', '\tunknown_0055e508= 1;'),
]
