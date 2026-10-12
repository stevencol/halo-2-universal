# the thing of 0x47f048 leaves and joins the world of the physics, and is let go of

import random
import struct

from difftest import Buffer, Mock, mock_address

NOTHING = lambda harness, arguments: 0


def counted(random_, mock: int) -> Buffer:
    """A thing of Havok's that is counted: where its functions are, its count, and after it its first function, a mock."""
    data = bytearray(random_.randbytes(0x10))
    struct.pack_into("<I", data, 0, 0xC)
    struct.pack_into("<h", data, 6, random_.choice((1, 1, 2, 3, 0, -1)))
    struct.pack_into("<I", data, 0xC, mock_address(mock))
    return Buffer(bytes(data), pointers=(0,))


def run(t) -> None:
    random_ = random.Random(680)

    for n in range(200):
        t.check(
            "havok_unknown_001c29d0",
            globals={
                "unknown_0047f048": random_.getrandbits(32),
                "unknown_0051ecac": random_.getrandbits(32),
                "unknown_0051e9a4": random_.getrandbits(32),
                "unknown_0047f050": random_.getrandbits(32),
            },
            externals={
                "unknown_0030bd50": NOTHING,
                # its second argument is a byte of the stack of the function
                "unknown_0030f800": lambda harness, arguments: (0, (arguments[0], arguments[2])),
                "unknown_00278f00": NOTHING,
            },
        )

        stays = [random_.choice((0, 0, 1)) for _ in range(2)]

        def stay(harness, arguments, stays=stays):
            calls = sum(1 for call in harness._calls if call[0] == "unknown_00279740")
            return stays[min(calls, 1)]

        t.check(
            "havok_unknown_001c2890",
            globals={
                "unknown_004f55b0": counted(random_, 0) if random_.random() < 0.7 else 0,
                "unknown_0047f048": counted(random_, 1),
            },
            mocks=[
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
            ],
            externals={"unknown_00279740": stay},
        )

        # a thing: its keys and values, and what it has at 0x44, are after it
        count = random_.choice((0, 1, 4, 4))
        entity = bytearray(random_.randbytes(0x48 + 0x44 + count * 8 + 8))
        struct.pack_into("<Ii", entity, 0x30, 0x48 + 0x44, count)
        struct.pack_into("<I", entity, 0x44, 0x48)
        struct.pack_into("<i", entity, 0x48 + 0x40, random_.choice((0, 1, 1, 2)))
        for index in range(count):
            struct.pack_into(
                "<2I", entity, 0x48 + 0x44 + index * 8, random_.choice((0x2001, 0x2001, 1, random_.getrandbits(32))), random_.getrandbits(32)
            )
        t.check(
            "havok_unknown_001c39c0",
            Buffer(bytes(entity), pointers=(0x30, 0x44)),
            random_.choice((0, 1, 1, 0x100)),
            globals={
                "unknown_0051e9a4": random_.getrandbits(32),
                "unknown_0047f050": random_.choice((0, 0x351, 0x352, 0x353, 0x1000, 0xFFFFFFFF)),
            },
            externals={
                "havok_unknown_001c51c0": NOTHING,
                "unknown_0030f2d0": NOTHING,
                "unknown_0030bc90": NOTHING,
                "unknown_00278f00": NOTHING,
            },
        )
