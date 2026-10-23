# mutations of milestone 770: one or more lines of each function

TESTS = "test_small_770"

LISTS = "source/interface/user_interface_lists.c"
TEXTS = "source/interface/user_interface_text_values.c"

MUTATIONS = [
    (LISTS, "<=unknown_0054d5b8;", "<unknown_0054d5b8;"),
    (LISTS, "\treturn thing[USER_INTERFACE_THING_UNKNOWN6D_OFFSET] && thing[USER_INTERFACE_THING_UNKNOWN6E_OFFSET] &&", "\treturn thing[USER_INTERFACE_THING_UNKNOWN6D_OFFSET] &&"),
    (LISTS, "\treturn offset[2];", "\treturn offset[1];"),
    (LISTS, "\tmemcpy(bounds, thing+USER_INTERFACE_THING_BOUNDS_OFFSET, 4*sizeof(short));", "\tmemcpy(bounds, thing+USER_INTERFACE_THING_BOUNDS_OFFSET, 3*sizeof(short));"),
    (LISTS, "\ty= (short)(long)offset[1];", "\ty= (short)(long)offset[2];"),
    (LISTS, "\tbounds[3]+= x;", "\tbounds[3]+= y;"),
    (TEXTS, "\tlong kind_text= 0;", "\tlong kind_text= 1;"),
    (TEXTS, "\t\t(*(const unsigned long *)unknown || *(const unsigned long *)(unknown+4)))", "\t\t*(const unsigned long *)unknown)"),
    (TEXTS, "\t\t\tkind_text= USER_INTERFACE_TEXT_UNKNOWNB82_KIND2;", "\t\t\tkind_text= USER_INTERFACE_TEXT_UNKNOWNB82_KIND3;"),
]
