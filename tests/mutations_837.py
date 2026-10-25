# mutations of milestone 837, written by make_mutations.py

TESTS = "test_small_837"

SOURCE_EFFECTS_EFFECTS_C = "source/effects/effects.c"
SOURCE_AI_VOCALIZATIONS_C = "source/ai/vocalizations.c"

MUTATIONS = [
    (SOURCE_EFFECTS_EFFECTS_C, '\tglobal_random_seed_reference[1]= seed;', '\tglobal_random_seed_reference[2]= seed;'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tlocation[3]= (byte)(((seed>>16)*0xFF)>>16);', '\tlocation[3]= (byte)(((seed>>16)*0xFF)>>17);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tif (index==NONE)', '\tif (index!=NONE)'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tlocation= (byte *)particle_location_data->data+(index&0xFFFF)*0x34;', ''),
    (SOURCE_EFFECTS_EFFECTS_C, '\tfor (slot= 0; slot<*(const long *)(definition+0x0C); slot++)', '\tfor (slot= 0; slot<*(const long *)(definition+0x0C)-1; slot++)'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tlong slot;\n\n\tif (effect_index==NONE || (effect_index&0xFFFF)>=effect_data->first_unallocated)\n\t{\n\t\treturn;', '\tlong slot;\n\n\tif (effect_index==NONE || (effect_index&0xFFFF)>effect_data->first_unallocated)\n\t{\n\t\treturn;'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\twhile (index!=NONE)', '\t\twhile (index!=0)'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\tlong *first= (long *)(effect+0x7C+slot*4);', '\t\tlong *first= (long *)(effect+0x7d+slot*4);'),
    (SOURCE_AI_VOCALIZATIONS_C, '\t\t\t\tif (*(const long *)(place+0x1C)==index)', '\t\t\t\tif (*(const long *)(place+0x1C)!=index)'),
    (SOURCE_AI_VOCALIZATIONS_C, '\t\t\tif (unit_index!=NONE)', '\t\t\tif (unit_index!=0)'),
    (SOURCE_AI_VOCALIZATIONS_C, '\t\t\t\tbyte *unit= (byte *)((const struct object_header_datum *)object_header_data->data)[unit_index&0xFFFF].datum;', '\t\t\t\tbyte *unit= (byte *)((const struct object_header_datum *)object_header_data->data)[unit_index&0x10000].datum;'),
    (SOURCE_AI_VOCALIZATIONS_C, '\t\t\tlong unit_index= *(const long *)(record+0x04);', '\t\t\tlong unit_index= *(const long *)(record+0x5);'),
]
