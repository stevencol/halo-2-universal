# mutations of milestone 848, written by make_mutations.py

TESTS = "test_small_848"

SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_MATH_REAL_MATH_C = "source/math/real_math.c"
SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"

MUTATIONS = [
    (SOURCE_PHYSICS_HAVOK_C, '\thavok_unknown_001ddb90(first_list, graph, mode, 0x030000D9, weapon_type, second_list);', '\thavok_unknown_001ddb90(first_list, graph, mode, 0x30000da, weapon_type, second_list);'),
    (SOURCE_PHYSICS_HAVOK_C, '\thavok_unknown_001ddb90(first_list, graph, 0x030000D9, 0x030000D9, 0x030000D9, second_list);', '\thavok_unknown_001ddb90(first_list, graph, 0x30000da, 0x030000D9, 0x030000D9, second_list);'),
    (SOURCE_PHYSICS_HAVOK_C, '\thavok_unknown_001ddb90(first_list, graph, mode, weapon_class, 0x030000D9, second_list);', '\thavok_unknown_001ddb90(first_list, graph, mode, weapon_class, 0x30000da, second_list);'),
    (SOURCE_PHYSICS_HAVOK_C, '\thavok_unknown_001ddb90(first_list, graph, mode, 0x030000D9, 0x030000D9, second_list);', '\thavok_unknown_001ddb90(first_list, graph, mode, 0x030000D9, 0x30000da, second_list);'),
    (SOURCE_PHYSICS_HAVOK_C, '\tcounter[1]= 0;', '\tcounter[2]= 0;'),
    (SOURCE_PHYSICS_HAVOK_C, '\tif (node_count<=0 || !(seconds>0.0f))', '\tif (node_count<0 || !(seconds>0.0f))'),
    (SOURCE_PHYSICS_HAVOK_C, '\tif (counter[1] && !(counter[3]&2) && seconds<unit_counter_seconds(counter))', '\tif (counter[1] && !(counter[3]&2) && seconds<=unit_counter_seconds(counter))'),
    (SOURCE_PHYSICS_HAVOK_C, '\tcounter[3]= 0;', ''),
    (SOURCE_MATH_REAL_MATH_C, '\tp= (1.0f-hsv[1])*hsv[2];', '\tp= (1.0f-hsv[1])*hsv[3];'),
    (SOURCE_MATH_REAL_MATH_C, '\t\trgb->red= p;\n\t\trgb->green= hsv[2];\n\t\trgb->blue= t;', '\t\trgb->red= p;\n\t\trgb->blue= t;'),
    (SOURCE_MATH_REAL_MATH_C, '\tcase 5:\n\t\trgb->red= hsv[2];\n\t\trgb->green= p;', '\tcase 5:\n\t\trgb->red= hsv[3];\n\t\trgb->green= p;'),
    (SOURCE_MATH_REAL_MATH_C, '\tcase 3:\n\t\trgb->red= p;\n\t\trgb->green= q;', '\tcase 3:\n\t\trgb->green= q;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tentries[(index+count)*3]= value[0];', '\tentries[(index+count)*3]= value[1];'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tentries[index*3]= value[0];', '\tentries[index*4]= value[0];'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tunsigned long *entries= (unsigned long *)(object+*(const short *)(object+0x126));\n\tlong count= (long)((unsigned long)(long)*(const short *)(object+0x124)/12)/2;\n\n\tif (index<0 || index>=count)\n\t{\n\t\treturn FALSE;\n\t}', '\tunsigned long *entries= (unsigned long *)(object+*(const short *)(object+0x126));\n\tlong count= (long)((unsigned long)(long)*(const short *)(object+0x124)/12)/2;\n\n\tif (index<1 || index>=count)\n\t{\n\t\treturn FALSE;\n\t}'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tcached_object_render_state_unknown_0003dd10(object_index, TRUE);\n\n\treturn TRUE;\n}\n', '\tcached_object_render_state_unknown_0003dd10(object_index, TRUE);\n\n\treturn FALSE;\n}\n'),
]
