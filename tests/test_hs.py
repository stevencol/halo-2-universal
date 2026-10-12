# source/hs/hs.c: the scripts close and stop for a map

import random
import struct

from difftest import Buffer, Mock, mock_address


def run(t) -> None:
    random_ = random.Random(6192)

    def note(harness, arguments):
        nodes = struct.unpack("<I", harness.mu.mem_read(harness.global_addresses["script_node_data"], 4))[0]
        return 0, (bytes(harness.mu.mem_read(nodes + 0x28, 4)),)

    for n in range(400):
        # the table of the nodes, and after it what its memory came from, with its two functions; or one that nothing gave
        data = bytearray(random_.randbytes(0x4C + 12))
        given = random_.choice((False, True, True))
        struct.pack_into("<I", data, 0x30, 0x4C if given else 0)
        struct.pack_into("<3I", data, 0x4C, 0x50, mock_address(0), mock_address(1))
        t.check(
            "hs_dispose",
            globals={
                "script_node_data": Buffer(bytes(data), pointers=(0x30, 0x4C) if given else (0x4C,)),
                "unknown_004f9380": Buffer(random_.randbytes(0x4C)),
            },
            mocks=[
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
            ],
        )

        nodes = bytearray(random_.randbytes(0x4C))
        struct.pack_into("<H", nodes, 0x2A, random_.choice((0, 2, 4, 6, 0xFFFD, 0xFFFF)))
        t.check(
            "hs_dispose_from_old_map",
            globals={
                "script_node_data": Buffer(bytes(nodes)),
                "object_list_header_data": Buffer(random_.randbytes(0x4C)),
                "list_object_data": Buffer(random_.randbytes(0x4C)),
            },
            externals={"unknown_0020a850": note, "unknown_00209290": note},
        )
