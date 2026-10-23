# mutations of milestone 775: one or two lines of each function

TESTS = "test_small_775"

FILE = "source/game/game_unknown_001a1c8e.c"

MUTATIONS = [
    (FILE, "\tif (first_count>=GAME_UNKNOWN_001A1C8E_VALUES)", "\tif (first_count>GAME_UNKNOWN_001A1C8E_VALUES)"),
    (FILE, "\t\tif (first_count+player->value_count<GAME_UNKNOWN_001A1C8E_VALUES)", "\t\tif (first_count+player->value_count<=GAME_UNKNOWN_001A1C8E_VALUES)"),
    (FILE, "\t\t\tplayer->value_count--;\n\t\t}\n\t}\n\tplayer->values[player->unknown2E8]= value;", "\t\t}\n\t}\n\tplayer->values[player->unknown2E8]= value;"),
    (FILE, "\tif (index<0)\n\t{\n\t\treturn;\n\t}\n\tfirst_count", "\tfirst_count"),
    (FILE, "\tif (index>=total)", "\tif (index>total)"),
    (FILE, "\t\tif (index!=first_count-1)", "\t\tif (TRUE)"),
    (FILE, "player->values[player->unknown2E8+player->value_count-1];", "player->values[player->unknown2E8+player->value_count];"),
    (FILE, "\t\t\tplayer->values[index]= player->values[total-1];", "\t\t\tplayer->values[index]= player->values[total-2];"),
    (FILE, "\tif (!(object[0xC1]&1))", "\tif (!(object[0xC1]&2))"),
    (FILE, "\t\tvelocity.k*= 0.33f;", "\t\tvelocity.j*= 0.33f;"),
    (FILE, "\t\tminimum= *(const real *)(element+0x3C)*1.05f;", "\t\tminimum= *(const real *)(element+0x3C);"),
    (FILE, "\treturn speed>=minimum;", "\treturn speed>minimum;"),
    (FILE, "\tif (((1<<object[0xAA])&3) && *(const long *)(object+0x13C)!=NONE)", "\tif (((1<<object[0xAA])&3))"),
    (FILE, "\t\tif (*(const long *)(object+0x13C)!=NONE)\n\t\t{\n\t\t\treturn FALSE;", "\t\tif (*(const long *)(object+0x13C)==NONE)\n\t\t{\n\t\t\treturn FALSE;"),
    (FILE, "\telse if (type_bit&0x1000)", "\telse if (type_bit&0x2000)"),
    (FILE, "\t\tif (*(const unsigned long *)(definition+0xBC)>>6&1)", "\t\tif (*(const unsigned long *)(definition+0xBC)>>5&1)"),
    (FILE, "\treturn !(object[0x10A]>>2&1);", "\treturn !(object[0x10A]>>1&1);"),
]
