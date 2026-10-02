# an example of a file of mutations for tools/mutation_check.py: three lines
# of the checksum, each broken in one way
#
#   python tools/mutation_check.py tests/mutations_example.py

TESTS = "test_game_state"

CRC = "source/memory/crc.c"

MUTATIONS = [
    (CRC, "value= (value>>1)^CRC_POLYNOMIAL;", "value= (value>>1)^(CRC_POLYNOMIAL+1);"),
    (CRC, "crc= (crc>>8)^crc_table[(*bytes++^crc)&0xFF];", "crc= (crc>>8)^crc_table[(*bytes++)&0xFF];"),
    (CRC, "\t\tcrc_table_initialized= TRUE;\n", "\t\t;\n"),
]
