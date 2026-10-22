# mutations of milestone 761: one or two lines of each function

TESTS = "test_small_761"

NETWORKING = "source/interface/user_interface_networking.c"

MUTATIONS = [
    (NETWORKING, "\tuser_interface_networking_unknown_00199e2e(TRUE);", "\tuser_interface_networking_unknown_00199e2e(FALSE);"),
    (NETWORKING, "\tif (unknown0 && !unknown4)", "\tif (unknown0)"),
    (NETWORKING, "\treturn network_life_cycle_unknown_00059890(2, 0, 0);", "\treturn network_life_cycle_unknown_00059890(1, 0, 0);"),
    (NETWORKING, "\treturn index!=NONE ? network_hoppers_get(index) : NULL;", "\treturn network_hoppers_get(index);"),
    (NETWORKING, "\treturn hopper ? network_hopper_unknown_00193470(hopper) : FALSE;", "\treturn hopper ? network_hopper_unknown_00193470(hopper) : TRUE;"),
    (NETWORKING, "\treturn hopper ? network_hopper_unknown_00193490(hopper) : FALSE;", "\treturn hopper ? network_hopper_unknown_001934b0(hopper) : FALSE;"),
    (NETWORKING, "\treturn hopper ? hopper->type==_network_hopper_type5 : FALSE;", "\treturn hopper ? hopper->type==_network_hopper_type4 : FALSE;"),
    (NETWORKING, "\treturn hopper ? network_hopper_unknown_001934b0(hopper) : FALSE;", "\treturn hopper ? !network_hopper_unknown_001934b0(hopper) : FALSE;"),
    (NETWORKING, "\tif (!unknown_004d8ba0)\n\t{\n\t\treturn FALSE;", "\tif (!unknown_004d8ba0)\n\t{\n\t\treturn TRUE;"),
    (NETWORKING, "\treturn state<=_network_life_cycle_state1 || state==_network_life_cycle_state3 ||", "\treturn state<=_network_life_cycle_state2 || state==_network_life_cycle_state3 ||"),
    (NETWORKING, "\t\tstate==_network_life_cycle_state4 || state>_network_life_cycle_state8;", "\t\tstate==_network_life_cycle_state4 || state>=_network_life_cycle_state8;"),
]
