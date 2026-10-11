# source/shell/shell.c: the console's static buffer takes its memory

import random
import struct


def run(t) -> None:
    random_ = random.Random(574)

    for n in range(600):
        memory = random_.choice((0x80400000, 0x80400004, 0x8040000F, 0x80400010, random_.getrandbits(32)))
        size = random_.choice((0, 1, 6, 15, 16, 0xCC, random_.getrandbits(20)))

        def checksum(harness, arguments):
            where = arguments[2] - harness.global_addresses["shell_memory_globals"]
            return 0, (bytes(harness.mu.mem_read(arguments[0], 4)), arguments[1], where)

        t.check(
            "xmem_static_buffer_allocate",
            globals={
                "shell_memory_globals": struct.pack("<4I", random_.getrandbits(32), memory, size, random_.getrandbits(32)),
                "xmem_static_buffer": random_.getrandbits(32),
                "xmem_static_buffer_used": bytes((random_.choice((0, 1)),)),
            },
            externals={"crc_checksum_buffer": checksum},
        )
