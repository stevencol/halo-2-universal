# mutations of milestone 764: one line of each function

TESTS = "test_small_764"

NETWORKING = "source/interface/user_interface_networking.c"

MUTATIONS = [
    (NETWORKING, "\treturn unknown==1 || unknown==3 || unknown==5;", "\treturn unknown==1 || unknown==3 || unknown==4;"),
    (NETWORKING, "\treturn unknown_0019989d()==6;", "\treturn unknown_0019989d()==5;"),
    (NETWORKING, "\treturn unknown==0 || unknown==2 || unknown==4;", "\treturn unknown==0 || unknown==2;"),
    (NETWORKING, "\treturn unknown==0 || unknown==1;", "\treturn unknown==1;"),
    (NETWORKING, "\treturn unknown==4 || unknown==5 || unknown==6;", "\treturn unknown==4 || unknown==5 || unknown==7;"),
]
