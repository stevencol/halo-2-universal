# mutations of milestone 838, written by make_mutations.py

TESTS = "test_small_838"

SOURCE_MAIN_MAIN_C = "source/main/main.c"
SOURCE_UNITS_UNITS_C = "source/units/units.c"
SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C = "source/game/game_unknown_0002b4a0.c"

MUTATIONS = [
    (SOURCE_MAIN_MAIN_C, '\tfraction= (time-range[0])/length;', '\tfraction= (time-range[1])/length;'),
    (SOURCE_MAIN_MAIN_C, '\tif (range[0]>=time)', '\tif (range[0]>time)'),
    (SOURCE_MAIN_MAIN_C, '\tconst byte *set= unknown_0050934c[0] ? unknown_0050934c+0x1C : unknown_0050934c+0x04;', '\tconst byte *set= unknown_0050934c[1] ? unknown_0050934c+0x1C : unknown_0050934c+0x04;'),
    (SOURCE_MAIN_MAIN_C, '\tif (time>range[1] || !(length>0.0001f))', '\tif (time>range[2] || !(length>0.0001f))'),
    (SOURCE_UNITS_UNITS_C, '\tif (counter[1] && (byte)up==(counter[3]&1) && ticks==counter[1])', '\tif (counter[1] && (byte)up==(counter[3]&1) || ticks==counter[1])'),
    (SOURCE_UNITS_UNITS_C, '\tcounter[0]= (signed char)(up ? 1 : ticks-1);\n\tcounter[3]= up!=0;\n}', '\tcounter[0]= (signed char)(up ? 1 : ticks-1);\n\tcounter[3]= up==0;\n}'),
    (SOURCE_UNITS_UNITS_C, '\t\tticks= 1;', '\t\tticks= 2;'),
    (SOURCE_UNITS_UNITS_C, '\tcounter[0]= (signed char)(up ? 1 : ticks-1);', '\tcounter[0]= (signed char)(up ? 1 : ticks-2);'),
    (SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C, '\tfor (; first<last && !result; first++)', '\tfor (; first<last && !result-1; first++)'),
    (SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C, '\telse if (!((flags>>12)&1))', '\telse if (!((flags>>13)&1))'),
    (SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C, '\t\tlist= *(const byte *const *)(*(const byte *const *)(definition+0x28)+((place>>9)&0x1FF)*0x5C+0x34);', '\t\tlist= *(const byte *const *)(*(const byte *const *)(definition+0x28)+((place>>9)&0x200)*0x5C+0x34);'),
    (SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C, '\tlast= *(const short *)(element+0x0C)+first;', '\tlast= *(const short *)(element+0xd)+first;'),
]
