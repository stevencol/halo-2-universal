# mutations of milestone 873, written by make_mutations.py; it found nothing to break in
# real_math_unknown_00170c00 (its work is in an inline helper) nor in unknown_00132b80: the
# last ones are written by hand (not one of the scale of the matrices, which the rotation
# between them does not look at)

TESTS = "test_small_873"

MUTATIONS = [
    ('source/math/real_math.c', '\t\tdifference[index]= first[index]-second[index];', ''),
    ('source/math/real_math.c', '\tfor (index= 0; index<10; index++)', '\tfor (index= 1; index<10; index++)'),
    ('source/math/real_math.c', '\t\t(const real_vector3d *)(first+13), (const real_vector3d *)(first+10), (real_vector3d *)(difference+10));', '\t\t(const real_vector3d *)(first+13), (const real_vector3d *)(first+11), (real_vector3d *)(difference+10));'),
    ('source/math/real_math.c', '\treal_math_unknown_00170c00((const real_vector3d *)(second+13), (const real_vector3d *)(second+10),', '\treal_math_unknown_00170c00((const real_vector3d *)(second+13), (const real_vector3d *)(second+11),'),
    ('source/math/real_math.c', '\tmatrix->left.i= up->j*forward->k - forward->j*up->k;', '\tmatrix->left.i= up->j*forward->k + forward->j*up->k;'),
    ('source/math/real_math.c', '\tmatrix->left.k= forward->j*up->i - forward->i*up->j;', '\tmatrix->left.k= forward->i*up->j - forward->j*up->i;'),
    ('source/math/real_math.c', '\tmatrix4x3_from_forward_and_up(&to, other_forward, other_up);', '\tmatrix4x3_from_forward_and_up(&to, other_up, other_forward);'),
    ('source/game/game_unknown_00131ff0.c', '\t*(short *)(data+0x0A)= global_structure_bsp_index;', '\t*(short *)(data+0x0A)= 0;'),
    ('source/game/game_unknown_00131ff0.c', '\t\t*(real *)(data+0x2A94)+= plane_offset;', ''),
    ('source/game/game_unknown_00131ff0.c', '\t\tmemcpy(destination, frusta, count*0x1BC);', '\t\tmemcpy(destination, frusta, count*0x1B8);'),
    ('source/game/game_unknown_00131ff0.c', '\tif (cluster_index==NONE)', '\tif (cluster_index==0)'),
    ('source/game/game_unknown_00131ff0.c', '\t\tmemset(data+0x2A98, 0, 13*sizeof(long));', '\t\tmemset(data+0x2A98, 0, 12*sizeof(long));'),
    ('source/game/game_unknown_00131ff0.c', '\tif (*(const long *)(data+0x2A78)==0 && (data[0x2A60]&0x80))', '\tif (*(const long *)(data+0x2A78)==0 && (data[0x2A60]&0x40))'),
    ('source/game/game_unknown_00131ff0.c', '\telse if (count && (!(3.0f>radius) || (*(const unsigned long *)(data+0x2A60)&0x100)))', '\telse if (count && (!(3.0f>=radius) || (*(const unsigned long *)(data+0x2A60)&0x100)))'),
    ('source/game/game_unknown_00131ff0.c', '\t\t*(unsigned long *)(data+0x2A60)= flags|0x200;', '\t\t*(unsigned long *)(data+0x2A60)= flags;'),
    ('source/game/game_unknown_00131ff0.c', '\t\t*(unsigned long *)(data+0x2A60)|= 1;', ''),
]
