# mutations of milestone 881, written by make_mutations.py; the last five by hand

TESTS = 'test_small_881'

MUTATIONS = [
    ('source/units/units.c', '\tmemset(control+0x04, 0, 0x20);', '\tmemset(control+0x5, 0, 0x20);'),
    ('source/units/units.c', '\tcontrol[0x02]= 0;', '\tcontrol[0x02]= 1;'),
    ('source/units/units.c', '\tcontrol[0x03]= 0;', ''),
    ('source/units/units.c', '\tdefinition= UNIT_DEFINITION_GET(unit);', ''),
    ('source/units/units.c', '\tmemset(control+0x04, 0, 0x20);', '\tmemset(control+0x04, 0, 0x1F);'),
    ('source/units/units.c', '\tcontrol[0x03]= 0;', ''),
    ('source/units/units.c', '\t*(long *)(control+0x94)= NONE;', ''),
    ('source/units/units.c', '\t*(short *)(control+0x7E)= (short)tag_unknown_0016da90(*(const long *)(definition+0x38), 0x090001A3);', '\t*(short *)(control+0x7E)= (short)tag_unknown_0016da90(*(const long *)(definition+0x38), 0x080001A2);'),
    ('source/units/units.c', '\tobject_unknown_00113870(unit_index);', ''),
]
