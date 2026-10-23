# mutations of milestone 781: one or two lines of each function

TESTS = "test_small_781"

EFFECTS = "source/effects/effects.c"
SOUND = "source/sound/sound_manager.c"

MUTATIONS = [
    (EFFECTS, "\t*(short *)(unknown+0x18)= NONE;", "\t*(short *)(unknown+0x18)= 0;"),
    (EFFECTS, "\t*(real *)(unknown+0x44)= 1.0f;", "\t*(real *)(unknown+0x44)= 0.0f;"),
    (EFFECTS, "\t*(unsigned long *)(unknown+0x58)= 0xFF808080;", "\t*(unsigned long *)(unknown+0x58)= 0xFF000000;"),
    (EFFECTS, "\t\tpairs[2*index+1]= NONE;\n\t}\n}", "\t}\n}"),
    (EFFECTS, "\tfor (index= 0; index<NUMBER_OF_EFFECT_PAIRS; index++)\n\t{\n\t\tpairs[2*index]= 0;", "\tfor (index= 0; index<NUMBER_OF_EFFECT_PAIRS-1; index++)\n\t{\n\t\tpairs[2*index]= 0;"),
    (EFFECTS, "\t\tif (pairs[2*index+1]==NONE && pairs[2*index]==0)\n\t\t{\n\t\t\treturn TRUE;", "\t\tif (pairs[2*index+1]==NONE)\n\t\t{\n\t\t\treturn TRUE;"),
    (EFFECTS, "\t\tif (pairs[2*index+1]==NONE && pairs[2*index]==0)\n\t\t{\n\t\t\tpairs[2*index+1]= second;", "\t\tif (pairs[2*index]==0)\n\t\t{\n\t\t\tpairs[2*index+1]= second;"),
    (EFFECTS, "\t\t\tpairs[2*index]= first;\n\t\t\treturn;", "\t\t\tpairs[2*index]= first;"),
    (SOUND, "\treturn values[0]>values[1] ? values[0] : values[1];", "\treturn values[0]<values[1] ? values[0] : values[1];"),
    (SOUND, "(const real *)(unknown_004ed284+gamepad_index*0x94+0x74);", "(const real *)(unknown_004ed284+gamepad_index*0x90+0x74);"),
]
