# mutations of milestone 875, written by make_mutations.py
# (one taken out by hand that changes nothing: >= instead of > when the real at 0x2B4 goes down to 0, which only puts 0 where 0 is)

TESTS = "test_small_875"

SOURCE_UNITS_UNITS_C = "source/units/units.c"
SOURCE_SCENARIO_SCENARIO_C = "source/scenario/scenario.c"

MUTATIONS = [
    (SOURCE_UNITS_UNITS_C, '\t\tif (*(const real *)(unit+0x2B4)>1.0f)', '\t\tif (*(const real *)(unit+0x2b5)>1.0f)'),
    (SOURCE_UNITS_UNITS_C, '\tif (*(const long *)(unit+0x12C)!=NONE && *(const long *)((const byte *)main_game_globals+0x08)==1 && unknown_004f55dc[2])', '\tif (*(const long *)(unit+0x12C)!=NONE && *(const long *)((const byte *)main_game_globals+0x08)==2 && unknown_004f55dc[2])'),
    (SOURCE_UNITS_UNITS_C, '\t\tif (team==NONE || team<0 || team>=0x10', '\t\tif (team==NONE || team<=0 || team>=0x10'),
    (SOURCE_SCENARIO_SCENARIO_C, '\t\tfor (portal_index= 0; portal_index<*(const long *)(cluster+0x8C); portal_index++)', '\t\tfor (portal_index= 0; portal_index<*(const long *)(cluster+0x8d); portal_index++)'),
    (SOURCE_SCENARIO_SCENARIO_C, '\t\tconst byte *cluster= *(const byte *const *)(structure_bsp+0xA0)+*(const long *)entry*0xB0;', '\t\tconst byte *cluster= *(const byte *const *)(structure_bsp+0xA0)+*(const long *)entry*0xb1;'),
    (SOURCE_SCENARIO_SCENARIO_C, '\t\t\t\tvisited[other>>5]|= 1UL<<(other&31);', '\t\t\t\tvisited[other>>5]|= 1UL<<(other&32);'),
    (SOURCE_SCENARIO_SCENARIO_C, '\t\t\t\tbyte *added= clusters+count*0x18;', '\t\t\t\tbyte *added= clusters+count*0x19;'),
]
