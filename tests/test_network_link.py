# source/networking/network_link.c

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
CHANNEL = 0xF8
CHANNELS = 6


def run(t) -> None:
    random_ = random.Random(493)

    for n in range(3000):
        # a few addresses, so that the ones of the list are often the one looked for; some of no size, or of less
        pool = []
        for size in (4, 4, 4, 16, 16, 0, 0x8000):
            address = bytearray(random_.randbytes(0x14))
            address[1:4] = bytes(3)
            struct.pack_into("<H", address, 0x12, size)
            pool.append(bytes(address))
        pool.append(pool[0][:0x10] + random_.randbytes(2) + pool[0][0x12:])
        pool.append(pool[3][:0x12] + struct.pack("<H", 4))

        def channels() -> Buffer:
            data = bytearray(random_.randbytes(CHANNELS * CHANNEL))
            for index in range(CHANNELS):
                data[index * CHANNEL + 0x48] = random_.choice((0, 0x40, 0x80, 0xC0, 0xFF, 0x10))
                struct.pack_into("<i", data, index * CHANNEL + 0x4C, random_.choice((1, 2, 3)))
                struct.pack_into("<i", data, index * CHANNEL + 0x54, random_.choice((0, 1, 2, 3, 4, 5, 5)))
            return Buffer(bytes(data))

        def link() -> Buffer:
            data = bytearray(random_.randbytes(0x224))
            struct.pack_into("<i", data, 0x20, random_.choice((0, 1, 2, 5, 15, 16, 16, -1)))
            for index in range(16):
                struct.pack_into("<I", data, 0x24 + index * 0x20, random_.randrange(CHANNELS))
                data[0x30 + index * 0x20:0x44 + index * 0x20] = random_.choice(pool)
            return Buffer(bytes(data))

        kind = random_.choice((0, 1, 2, 2, 1, 3, NONE))
        t.check(
            "network_link_find_channel",
            link(),
            kind,
            Buffer(random_.choice(pool)),
            globals={"unknown_004d87d4": channels()},
        )
        t.check(
            "network_link_channel_of_address",
            link(),
            Buffer(random_.choice(pool)),
            kind,
            globals={"unknown_004d87d4": channels()},
        )
        t.check("network_link_remove_channel", link(), random_.randrange(CHANNELS + 1))
        t.check(
            "network_link_add_channel",
            link(),
            random_.randrange(CHANNELS),
            random_.getrandbits(32),
            Buffer(random_.choice(pool)),
            globals={"unknown_004d87d4": channels()},
            externals={"network_channel_release": lambda harness, arguments: (0, (harness.normalize(arguments[0]),))},
        )
        t.check(
            "network_link_close_channels",
            link(),
            globals={"unknown_004d87d4": channels()},
            externals={
                "network_channel_close": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
            },
        )
