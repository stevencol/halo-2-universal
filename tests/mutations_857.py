# mutations of milestone 857, written by make_mutations.py

TESTS = "test_small_857"

SOURCE_SHELL_SHELL_C = "source/shell/shell.c"
SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"

MUTATIONS = [
    (SOURCE_SHELL_SHELL_C, '\treturn shell_bitmap_texture(bitmap, 7, 0.0f);', '\treturn shell_bitmap_texture(bitmap, 8, 0.0f);'),
    (SOURCE_SHELL_SHELL_C, '\treturn shell_bitmap_texture(bitmap, 0x0E, 0.0f);', '\treturn shell_bitmap_texture(bitmap, 0xf, 0.0f);'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (entity==NONE)', '\tif (entity!=NONE)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tbyte record[0x14];', '\tbyte record[0x15];'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (!(OBJECT_HEADER_GET(object_index)->flags&0x40) || !((*(const unsigned long *)(object+0x04)>>9)&1))', '\tif (!(OBJECT_HEADER_GET(object_index)->flags&0x41) || !((*(const unsigned long *)(object+0x04)>>9)&1))'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tcluster_partition_set_extra(*(const long *)(object+0x60), &collideable_object_cluster_partition, record, object_index,', '\tcluster_partition_set_extra(*(const long *)(object+0x61), &collideable_object_cluster_partition, record, object_index,'),
]
