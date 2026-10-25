# mutations of milestone 885, written by make_mutations.py
# (one taken out by hand that changes nothing: >= instead of > for the side, as the distance there is more than 1/64 and never 0)

TESTS = "test_small_885"

SOURCE_MATH_REAL_MATH_C = "source/math/real_math.c"

MUTATIONS = [
    (SOURCE_MATH_REAL_MATH_C, '\t\tif (state<3)', '\t\tif (state<4)'),
    (SOURCE_MATH_REAL_MATH_C, '\t\t\treturn 3;', '\t\t\treturn 4;'),
    (SOURCE_MATH_REAL_MATH_C, '\tlong double exact= (long double)origin[2]*plane[2] + (long double)origin[1]*plane[1]', '\tlong double exact= (long double)origin[3]*plane[2] + (long double)origin[1]*plane[1]'),
]
