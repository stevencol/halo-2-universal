# mutations of milestone 765: one or two lines of each function

TESTS = "test_small_765"

ENTRIES = "source/game/game_unknown_001991f0.c"

MUTATIONS = [
    (ENTRIES, "\t\tif (!entry[GAME_UNKNOWN_001991F0_VALID_OFFSET])", "\t\tif (entry[GAME_UNKNOWN_001991F0_VALID_OFFSET])"),
    (ENTRIES, "\t\t\tmemset(entry, 0, GAME_UNKNOWN_001991F0_ENTRY_SIZE);", "\t\t\tmemset(entry, 0, GAME_UNKNOWN_001991F0_IDENTIFIER_SIZE);"),
    (ENTRIES, "entry[0]==other[0] && entry[1]==other[1] && entry[2]==other[2])", "entry[0]==other[0] && entry[1]==other[1])"),
    (ENTRIES, "\t\tif (entry[GAME_UNKNOWN_001991F0_VALID_OFFSET] && entry[GAME_UNKNOWN_001991F0_UNKNOWN8_OFFSET])", "\t\tif (entry[GAME_UNKNOWN_001991F0_UNKNOWN8_OFFSET])"),
    (ENTRIES, "]= index==marked_index;", "]= index!=marked_index;"),
    (ENTRIES, "\t\tif (unknown_0051e8d4[index][GAME_UNKNOWN_001991F0_VALID_OFFSET])", "\t\tif (TRUE)"),
    (ENTRIES, "\tif (size<4)", "\tif (size<=4)"),
    (ENTRIES, "(bytes>>8&0xFF00)", "(bytes>>16&0xFF00)"),
]
