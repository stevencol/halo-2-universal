# mutations of milestone 754: one or two lines of each function

TESTS = "test_small_754"

SEARCH = "source/networking/network_search.c"

MUTATIONS = [
    (SEARCH, "\t\tif (memcmp(identifiers+index*8, identifier, 8)==0)", "\t\tif (memcmp(identifiers+index*8, identifier, 8)!=0)"),
    (SEARCH, "index<*(const long *)(search+NETWORK_SEARCH_IDENTIFIER_COUNT_OFFSET) && !found;", "index<*(const long *)(search+NETWORK_SEARCH_IDENTIFIER_COUNT_OFFSET)-1 && !found;"),
    (SEARCH, "\tresult[NETWORK_SEARCH_RESULT_UNKNOWN1_OFFSET]= FALSE;", "\tresult[NETWORK_SEARCH_RESULT_UNKNOWN1_OFFSET]= TRUE;"),
    (SEARCH, "\tif (count<*(const long *)(search+NETWORK_SEARCH_MAXIMUM_IDENTIFIERS_OFFSET))", "\tif (count<=*(const long *)(search+NETWORK_SEARCH_MAXIMUM_IDENTIFIERS_OFFSET))"),
    (SEARCH, "\t\t*(long *)(search+NETWORK_SEARCH_IDENTIFIER_COUNT_OFFSET)= count+1;", "\t\t*(long *)(search+NETWORK_SEARCH_IDENTIFIER_COUNT_OFFSET)= count+2;"),
    (SEARCH, "result[NETWORK_SEARCH_RESULT_UNKNOWN74_OFFSET] && memcmp(data+4, result+0xBC, 8)==0)", "result[NETWORK_SEARCH_RESULT_UNKNOWN74_OFFSET] && memcmp(data, result+0xBC, 8)==0)"),
    (SEARCH, "\t\tif (result[0] && result[NETWORK_SEARCH_RESULT_UNKNOWN74_OFFSET] && memcmp", "\t\tif (result[NETWORK_SEARCH_RESULT_UNKNOWN74_OFFSET] && memcmp"),
    (SEARCH, "\t\tsecond_value= *(const long *)(search+NETWORK_SEARCH_UNKNOWN20_OFFSET)+1;", "\t\tsecond_value= *(const long *)(search+NETWORK_SEARCH_UNKNOWN20_OFFSET);"),
    (SEARCH, "\t\tif (first_value==NONE)", "\t\tif (first_value!=NONE)"),
    (SEARCH, "\t\t\tsum+= *(const unsigned long *)(result+NETWORK_SEARCH_RESULT_UNKNOWN48_OFFSET);", "\t\t\tsum= *(const unsigned long *)(result+NETWORK_SEARCH_RESULT_UNKNOWN48_OFFSET);"),
    (SEARCH, ">=*(const long *)(search+NETWORK_SEARCH_UNKNOWN34_OFFSET) &&", ">*(const long *)(search+NETWORK_SEARCH_UNKNOWN34_OFFSET) &&"),
    (SEARCH, "\t\tif (ready_count<(count>>2))", "\t\tif (ready_count<=(count>>2))"),
    (SEARCH, "*(const unsigned long *)(search+NETWORK_SEARCH_STARTED_OFFSET))<=unknown_004ce394)", "*(const unsigned long *)(search+NETWORK_SEARCH_STARTED_OFFSET))<unknown_004ce394)"),
    (SEARCH, "\t\ttime= unknown_00510548 ? unknown_0051054c : GetTickCount();", "\t\ttime= GetTickCount();"),
    (SEARCH, "\t\t*(long *)data= *(const short *)(best+0xAA);", "\t\t*(long *)data= *(const unsigned short *)(best+0xAA);"),
    (SEARCH, "\t\tmemcpy(data+0x1C, best+0xD4, 0x24);", "\t\tmemcpy(data+0x1C, best+0xD4, 0x20);"),
]
