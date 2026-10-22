# mutations of milestone 748: one or two lines of each function

TESTS = "test_small_748"

ENCODING = "source/networking/simulation_encoding.c"
TASKS = "source/networking/online_tasks.c"

MUTATIONS = [
    (ENCODING, "\tif (mask>=0x10000)\n\t{\n\t\tchar text[256];", "\tif (mask>0x10000)\n\t{\n\t\tchar text[256];"),
    (ENCODING, "\tbitstream_write_integer(stream, mask, 16);", "\tbitstream_write_integer(stream, mask, 15);"),
    (ENCODING, "\t\tif (*(const unsigned long *)data&(1<<index))\n\t\t{\n\t\t\tbitstream_write_bits(stream, data+4+index*6, 0x30);", "\t\tif (*(const unsigned long *)data&(2<<index))\n\t\t{\n\t\t\tbitstream_write_bits(stream, data+4+index*6, 0x30);"),
    (ENCODING, "\t\t\tbitstream_write_bits(stream, data+4+index*6, 0x30);", "\t\t\tbitstream_write_bits(stream, data+4+index*6, 0x28);"),
    (ENCODING, "\t*(unsigned long *)data= bitstream_read_integer(stream, 16);", "\t*(unsigned long *)data= bitstream_read_integer(stream, 15);"),
    (ENCODING, "\t\t\tbitstream_read_bits(stream, thing, 0x30);", "\t\t\tbitstream_read_bits(stream, thing+1, 0x28);"),
    (ENCODING, "\t\t\t*(short *)(thing+4)= 0;", "\t\t\t*(byte *)(thing+4)= 0;"),
    (ENCODING, "\treturn !stream->error && stream->position<=(long)((unsigned long)stream->size<<3);", "\treturn stream->position<=(long)((unsigned long)stream->size<<3);"),
    (ENCODING, "\treturn !stream->error && stream->position<=(long)((unsigned long)stream->size<<3);", "\treturn !stream->error && stream->position<(long)((unsigned long)stream->size<<3);"),
    (TASKS, "\treturn result<0 && result!=(long)ONLINE_TASK_UNKNOWN80155100;", "\treturn result<0;"),
    (TASKS, "\treturn result<0 && result!=(long)ONLINE_TASK_UNKNOWN80155100;", "\treturn result!=(long)ONLINE_TASK_UNKNOWN80155100;"),
    (TASKS, "\tlong result= online_task_continue(task);\n\n\treturn result<0 &&", "\tlong result= online_task_continue(NULL);\n\n\treturn result<0 &&"),
]
