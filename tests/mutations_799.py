# mutations of milestone 799, written by make_mutations.py

TESTS = "test_small_799"

SOURCE_SOUND_SOUND_MANAGER_C = "source/sound/sound_manager.c"

MUTATIONS = [
    (SOURCE_SOUND_SOUND_MANAGER_C, '\treturn ((1UL<<(type&0x1F))&4)!=0;', '\treturn ((1UL<<(type&0x1F))&4)!=1;'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\t|| object_index==NONE)\n\t{\n\t\treturn FALSE;\n\t}\n\ttype= ((const struct object_header_datum *)object_header_data->data)[object_index&0xFFFF].type;', '\t\t|| object_index==NONE)\n\t{\n\t\treturn TRUE;\n\t}\n\ttype= ((const struct object_header_datum *)object_header_data->data)[object_index&0xFFFF].type;'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\t|| object_index==NONE)', '\t\t&& object_index==NONE)'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\t|| !(*(const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address&0x40)', '\t\t|| !(*(const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address&0x41)'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\tif (first[index]!=second[index])\n\t\t{\n\t\t\treturn FALSE;\n\t\t}\n\t}', '\t\tif (first[index]!=second[index])\n\t\t{\n\t\t\treturn TRUE;\n\t\t}\n\t}'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tfor (index= 0; index<10; index++)', '\tfor (index= 0; index<10-1; index++)'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\tif (first[index]!=second[index])', '\t\tif (first[index]==second[index])'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\t}\n\t}\n\n\treturn TRUE;\n}\n\n/* 0x0018d360: whether any element (0x58 bytes, of the block at 0x1C) of a tag', '\t\t}\n\t}\n\n\treturn FALSE;\n}\n\n/* 0x0018d360: whether any element (0x58 bytes, of the block at 0x1C) of a tag'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\tif (other!=NONE)', '\t\tif (other==NONE)'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\t}\n\t}\n\n\treturn FALSE;\n}\n\n/* 0x0018d630: which element (0x38 bytes, of the block at 0x00) of a tag', '\t\t}\n\t}\n\n\treturn TRUE;\n}\n\n/* 0x0018d630: which element (0x38 bytes, of the block at 0x00) of a tag'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\tlong other= *(const long *)(*(const byte *const *)(tag+0x20)+index*0x58+0x20);', '\t\tlong other= *(const long *)(*(const byte *const *)(tag+0x21)+index*0x58+0x20);'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\t\tsigned char kind= *(const signed char *)((const byte *)(uintptr_t)global_tag_instances[other&0xFFFF].address+2);', '\t\t\tsigned char kind= *(const signed char *)((const byte *)(uintptr_t)global_tag_instances[other&0xFFFF].address+3);'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\tif (*(const long *)(*(const byte *const *)(tag+4)+index*0x38)==name)', '\t\tif (*(const long *)(*(const byte *const *)(tag+4)+index*0x38)!=name)'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tfor (index= 0; index<*(const long *)tag; index++)', '\tfor (index= 0; index<*(const long *)tag-1; index++)'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tif (tag_index==NONE)\n\t{\n\t\treturn NONE;\n\t}\n\ttag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;', '\tif (tag_index==NONE)\n\t{\n\t\treturn 0;\n\t}\n\ttag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t}\n\ttag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;\n\tfor (index= 0; index<*(const long *)tag; index++)', '\t}\n\ttag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0x10000].address;\n\tfor (index= 0; index<*(const long *)tag; index++)'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\t\treal z= ((const real *)entry)[0]-point[2];', '\t\t\treal z= ((const real *)entry)[1]-point[2];'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\t\tif (squared>=x*x + y*y + z*z)\n\t\t\t{\n\t\t\t\treturn TRUE;\n\t\t\t}\n\t\t}\n\t}\n\n\treturn FALSE;\n}\n', '\t\t\tif (squared>=x*x + y*y + z*z)\n\t\t\t{\n\t\t\t\treturn TRUE;\n\t\t\t}\n\t\t}\n\t}\n\n\treturn TRUE;\n}\n'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\tconst byte *entry= game_sound_globals+0xC0+index*0x48;', '\t\tconst byte *entry= game_sound_globals+0xc1+index*0x48;'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\t\tif (squared>=x*x + y*y + z*z)', '\t\t\tif (squared>x*x + y*y + z*z)'),
]
