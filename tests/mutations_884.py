# mutations of milestone 884, written by make_mutations.py (one taken out by hand that changes
# nothing: the size of the array of the projected point); the last five by hand

TESTS = 'test_small_884'

MUTATIONS = [
    ('source/math/real_math.c', '\treturn ai_point_in_polygon2d(point, &points[0][0], (short)*(const long *)(polygon+0x1C), 0.0001f)!=FALSE;', '\treturn ai_point_in_polygon2d(point, &points[0][0], (short)*(const long *)(polygon+0x1C), 0.0001f)!=TRUE;'),
    ('source/math/real_math.c', '\t\tmemcpy(&points[index][0], &vertex[axes[0]], sizeof(real));', '\t\tmemcpy(&points[index][0], &vertex[axes[1]], sizeof(real));'),
    ('source/math/real_math.c', '\tprojected[1]= distance*plane[1] + origin[1];', '\tprojected[1]= distance*plane[2] + origin[1];'),
    ('source/math/real_math.c', '\tprojected[0]= plane[0]*distance + origin[0];', '\tprojected[0]= plane[0]*distance - origin[0];'),
    ('source/math/real_math.c', '\tpoint[1]= projected[axes[1]];', '\tpoint[1]= projected[axes[0]];'),
    ('source/math/real_math.c', '(short)*(const long *)(polygon+0x1C), 0.0001f)!=FALSE;', '(short)*(const long *)(polygon+0x1C), 0.5f)!=FALSE;'),
    ('source/math/real_math.c', '\tconst short *axes= projection_axes[axis][plane[axis]>0.0f];', '\tconst short *axes= projection_axes[axis][plane[axis]<0.0f];'),
    ('source/math/real_math.c', '\t\tmemcpy(&points[index][1], &vertex[axes[1]], sizeof(real));', '\t\tmemcpy(&points[index][1], &vertex[axes[0]], sizeof(real));'),
]
