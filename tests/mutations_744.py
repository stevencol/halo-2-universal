# mutations of milestone 744: one or two lines of each function

TESTS = "test_small_744"

STATISTICS = "source/networking/online_statistics.c"

MUTATIONS = [
    (STATISTICS, "\t\tif (place_index==NONE || place_index==index)", "\t\tif (place_index==index)"),
    (STATISTICS, "\t\t\tif (*(const long *)(record+ONLINE_STATISTICS_RECORD_STATE_OFFSET)==_online_statistics_record_state3)", "\t\t\tif (*(const long *)(record+ONLINE_STATISTICS_RECORD_STATE_OFFSET)>=_online_statistics_record_state3)"),
    (STATISTICS, "\t\t\t\t*(long *)(record+ONLINE_STATISTICS_RECORD_TIME_OFFSET)= online_statistics_time();", "\t\t\t\t*(long *)(record+ONLINE_STATISTICS_RECORD_TIME_OFFSET)= GetTickCount();"),
    (STATISTICS, "\t\tif (value<unknown_004ce7b4[level+1])", "\t\tif (value<=unknown_004ce7b4[level+1])"),
    (STATISTICS, "\t\t\tif (level==maximum_level-1)", "\t\t\tif (level==maximum_level)"),
    (STATISTICS, "\t\t\t\tif (value>middle)", "\t\t\t\tif (value>=middle)"),
    (STATISTICS, "\tfor (level= 0; level<NUMBER_OF_ONLINE_STATISTICS_LEVELS-1; level++)", "\tfor (level= 0; level<NUMBER_OF_ONLINE_STATISTICS_LEVELS-2; level++)"),
    (STATISTICS, "\tif (!entry[ONLINE_STATISTICS_VALUE_VALID_OFFSET] ||\n\t\t*(const long *)(entry+ONLINE_STATISTICS_VALUE_VALUE_OFFSET)!=value)\n\t{\n\t\t*(long *)(entry+ONLINE_STATISTICS_VALUE_VALUE_OFFSET)= value;\n\t\tentry[ONLINE_STATISTICS_VALUE_VALID_OFFSET]= TRUE;", "\tif (!entry[ONLINE_STATISTICS_VALUE_VALID_OFFSET] ||\n\t\t*(const long *)(entry+ONLINE_STATISTICS_VALUE_VALUE_OFFSET)!=value)\n\t{\n\t\t*(long *)(entry+ONLINE_STATISTICS_VALUE_VALUE_OFFSET)= value;\n\t\tentry[ONLINE_STATISTICS_VALUE_VALID_OFFSET]= 2;"),
    (STATISTICS, "\tif (!entry[ONLINE_STATISTICS_VALUE_VALID_OFFSET] ||\n", "\tif (TRUE ||\n"),
    (STATISTICS, "\t\t(record_index*ONLINE_STATISTICS_VALUES_PER_RECORD+kind)*ONLINE_STATISTICS_VALUE_SIZE;", "\t\t(record_index*0x12+kind)*ONLINE_STATISTICS_VALUE_SIZE;"),
    (STATISTICS, "\tif (!entry[ONLINE_STATISTICS_VALUE_VALID_OFFSET] || amount==0)", "\tif (!entry[ONLINE_STATISTICS_VALUE_VALID_OFFSET])"),
    (STATISTICS, "\tvalue= (long)((unsigned long)*(const long *)(entry+ONLINE_STATISTICS_VALUE_VALUE_OFFSET)+(unsigned long)amount);\n\tif (value<unknown_0046722c[kind][0])", "\tvalue= (long)((unsigned long)*(const long *)(entry+ONLINE_STATISTICS_VALUE_VALUE_OFFSET)+(unsigned long)amount);\n\tif (value<unknown_0046722c[kind][1])"),
    (STATISTICS, "\tif (!entry[ONLINE_STATISTICS_VALUE_VALID_OFFSET])\n\t{\n\t\treturn;\n\t}\n\tif (value<", "\tif (entry[ONLINE_STATISTICS_VALUE_VALID_OFFSET])\n\t{\n\t\treturn;\n\t}\n\tif (value<"),
    (STATISTICS, "\tif (value>*(const long *)(entry+ONLINE_STATISTICS_VALUE_VALUE_OFFSET))", "\tif (value!=*(const long *)(entry+ONLINE_STATISTICS_VALUE_VALUE_OFFSET))"),
    (STATISTICS, "\t\tvalue= unknown_0046722c[kind][1];\n\t}\n\tif (value>*(const long", "\t\tvalue= unknown_0046722c[kind][0];\n\t}\n\tif (value>*(const long"),
]
