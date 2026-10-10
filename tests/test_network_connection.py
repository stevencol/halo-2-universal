# source/networking/network_connection.c

import random
import struct

from difftest import Buffer

SIZE = 0x97C


def run(t) -> None:
    random_ = random.Random(432)

    def number() -> int:
        return random_.choice((0, 1, 50, 100, 1000, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 0xFFFFFF00, random_.getrandbits(32), random_.getrandbits(10)))

    for n in range(3000):
        data = bytearray(random_.randbytes(SIZE))
        for offset in (0x94C, 0x968, 0x96C, 0x970, 0x978):
            struct.pack_into("<I", data, offset, number())
        time = number()
        if random_.random() < 0.2:
            # just as the least, the most and the smoothed ones
            time = struct.unpack_from("<I", data, random_.choice((0x968, 0x96C)))[0]
        t.check(
            "network_connection_update_times",
            random_.choice((1, 1, 3, 3, 4, 5, 0, 2, 6, random_.getrandbits(32))),
            Buffer(bytes(data)),
            time,
            random_.choice((number(), struct.unpack_from("<I", data, 0x94C)[0])),
            number(),
            globals={
                "unknown_004cf6f8": struct.pack("<I", random_.choice((0, 1, 3, 3, 31, 32, 35))),
                "unknown_004cf6fc": struct.pack("<I", random_.choice((0, 1, 2, 2, 31, 34))),
                "unknown_004cf700": struct.pack("<I", random_.choice((0, 1, 4, 4, 0xFFFFFFFF, 1000))),
                "unknown_004cf704": struct.pack("<I", number()),
                "unknown_004cf708": struct.pack("<I", number()),
                "unknown_004cf71c": struct.pack("<I", number()),
                "unknown_004cf720": struct.pack("<I", number()),
                "unknown_004cf724": struct.pack("<I", number()),
            },
        )
