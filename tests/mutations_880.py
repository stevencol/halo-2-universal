# mutations of milestone 880, written by make_mutations.py
# (one taken out by hand that changes nothing: clearing the bit 0 at 0x11 as well, which is 0 there as the bits 9 were looked at)

TESTS = "test_small_880"

SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_MATH_REAL_MATH_C = "source/math/real_math.c"

MUTATIONS = [
    (SOURCE_PHYSICS_HAVOK_C, '\tif ((channel[0x12]&1) && !(channel[0x11]&9))', '\tif ((channel[0x12]&1) && !(channel[0x12]&9))'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\t\t>=(real)*(const short *)(animation+0x14)-0.0001f)', '\t\t\t>(real)*(const short *)(animation+0x14)-0.0001f)'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tif (game_time_globals->seconds_per_tick**(const real *)(channel+0x18)*30.0f + position', '\t\tif (game_time_globals->seconds_per_tick**(const real *)(channel+0x19)*30.0f + position'),
    (SOURCE_MATH_REAL_MATH_C, '\t\tresult[0]= result[0]*tint[0];', '\t\tresult[0]= result[0]*tint[1];'),
    (SOURCE_MATH_REAL_MATH_C, '\t\tresult[0]= result[0]*alpha + tint[0]*weight;', '\t\tresult[0]= result[1]*alpha + tint[0]*weight;'),
    (SOURCE_MATH_REAL_MATH_C, '\t\treal alpha= (1.0f-t)*first[0] + second[0]*t;', '\t\treal alpha= (1.0f-t)*first[0] + second[1]*t;'),
    (SOURCE_MATH_REAL_MATH_C, '\t\tresult[2]= tint[2]*weight + result[2]*alpha;', ''),
]
