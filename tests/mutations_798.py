# mutations of milestone 798, written by make_mutations.py

TESTS = "test_small_798"

SOURCE_PHYSICS_BREAKABLE_SURFACES_C = "source/physics/breakable_surfaces.c"
SOURCE_SOUND_SOUND_MANAGER_C = "source/sound/sound_manager.c"

MUTATIONS = [
    (SOURCE_PHYSICS_BREAKABLE_SURFACES_C, '\tif (!collision || surface<0 || surface>=*(const long *)(collision+0x28))', '\tif (!collision || surface<=0 || surface>=*(const long *)(collision+0x28))'),
    (SOURCE_PHYSICS_BREAKABLE_SURFACES_C, '\treturn ((*(const byte *const *)(collision+0x2C))[surface*8+4]&8)', '\treturn ((*(const byte *const *)(collision+0x2C))[surface*8+5]&8)'),
    (SOURCE_PHYSICS_BREAKABLE_SURFACES_C, '\t\tif (element<0 || element>=*(const long *)(bsp+0x140))', '\t\tif (element<1 || element>=*(const long *)(bsp+0x140))'),
    (SOURCE_PHYSICS_BREAKABLE_SURFACES_C, '\t\t\t+*(const short *)(*(const byte *const *)(bsp+0x144)+element*0x58+0x34)*0xC8+0x70;', '\t\t\t+*(const short *)(*(const byte *const *)(bsp+0x144)+element*0x59+0x34)*0xC8+0x70;'),
    (SOURCE_PHYSICS_BREAKABLE_SURFACES_C, '\treturn (((const unsigned long *)BREAKABLE_SURFACES_BITS(element))[surface>>5]&(1UL<<(surface&0x1F)))!=0;', '\treturn (((const unsigned long *)BREAKABLE_SURFACES_BITS(element))[surface>>5]&(1UL<<(surface&0x20)))!=0;'),
    (SOURCE_PHYSICS_BREAKABLE_SURFACES_C, '\tif (surface==NONE)', '\tif (surface!=NONE)'),
    (SOURCE_PHYSICS_BREAKABLE_SURFACES_C, '\t\treturn TRUE;', '\t\treturn FALSE;'),
    (SOURCE_PHYSICS_BREAKABLE_SURFACES_C, '\tif (surface!=NONE)', '\tif (surface!=0)'),
    (SOURCE_PHYSICS_BREAKABLE_SURFACES_C, '\t\tindex= element==NONE ? unknown_004ea960[surface] : unknown_004eaa60[element*8+surface];', ''),
    (SOURCE_PHYSICS_BREAKABLE_SURFACES_C, '\tbyte index= 0xFF;', '\tbyte index= 0x100;'),
    (SOURCE_PHYSICS_BREAKABLE_SURFACES_C, '\treturn unknown_004eca60[index>>5];', '\treturn unknown_004eca60[index>>6];'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tlong player_index= *(const long *)(unknown_004ed284+index*0x94+0x60);', '\tlong player_index= *(const long *)(unknown_004ed284+index*0x95+0x60);'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tif (!*(const short *)player || *(const short *)player!=player_index>>16 || *(const long *)(player+0x2C)==NONE)', '\tif (!*(const short *)player || *(const short *)player!=player_index>>17 || *(const long *)(player+0x2C)==NONE)'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tif (player_index==NONE || (player_index&0xFFFF)>=player_data->first_unallocated)', '\tif (player_index!=NONE || (player_index&0xFFFF)>=player_data->first_unallocated)'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tplayer= (const byte *)player_data->data+(player_index&0xFFFF)*player_data->size;', ''),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tif (index==NONE)', '\tif (index!=NONE)'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\treturn *(const short *)(unknown_004ed284+index*0x94+0x42);', '\treturn *(const short *)(unknown_004ed284+index*0x95+0x42);'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tif (index==NONE)\n\t{\n\t\treturn NONE;\n\t}\n', '\tif (index==NONE)\n\t{\n\t\treturn 0;\n\t}\n'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\tif (value==*(const long *)(*(const byte *const *)(tag+0x154)+index*0xB4))', '\t\tif (value==*(const long *)(*(const byte *const *)(tag+0x155)+index*0xB4))'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tfor (index= 0; index<*(const long *)(tag+0x150); index++)', '\tfor (index= 0; index<*(const long *)(tag+0x151); index++)'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tif (index==NONE || index<0 || index>=*(const long *)(globals+0x150))', '\tif (index==NONE || index<0 && index>=*(const long *)(globals+0x150))'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\treturn *(byte *const *)(globals+0x154)+index*0xB4;', '\treturn *(byte *const *)(globals+0x155)+index*0xB4;'),
]
