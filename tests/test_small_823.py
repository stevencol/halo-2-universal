# five small functions of the way to the main menu: 0x50 bytes copied, a
# node of a list by a number, an entry of a chain by a short, memory for the
# state of the game and a time of the interface

import random
import struct

from difftest import Buffer, GlobalData

NONE = 0xFFFFFFFF
NODE = 0x80
ENTRY = 0x6C


def run(t) -> None:
    random_ = random.Random(823)

    for n in range(400):
        t.check("unknown_00052020", Buffer(random_.randbytes(0x50)), random_.getrandbits(32), Buffer(random_.randbytes(0x50)))

        count = random_.randrange(5)
        data = bytearray(random_.randbytes(0x20 + 4 * NODE))
        struct.pack_into("<I", data, 0x14, 0x20 if count else 0)
        pointers = [0x14] if count else []
        for node in range(4):
            at = 0x20 + node * NODE
            struct.pack_into("<I", data, at + 0x70, random_.choice((1, 2, 3)))
            if node + 1 < count:
                struct.pack_into("<I", data, at + 0x18, at + NODE)
                pointers.append(at + 0x18)
            else:
                struct.pack_into("<I", data, at + 0x18, 0)
        t.check("unknown_0024c5d0", Buffer(bytes(data), pointers=tuple(pointers)), random_.choice((1, 2, 3, 4)))

        entries = bytearray(random_.randbytes(6 * ENTRY))
        order = random_.sample(range(6), random_.randrange(1, 6))
        for place, entry in enumerate(order):
            struct.pack_into("<hh", entries, entry * ENTRY + 8, order[place + 1] if place + 1 < len(order) else -1, random_.choice((1, 2, 3)))
        t.check(
            "particle_systems_unknown_00173350",
            random_.choice((order[0], order[0], 0xFFFF)) | random_.getrandbits(16) << 16,
            Buffer(bytes(entries)),
            random_.choice((1, 2, 3, 4)) | random_.getrandbits(16) << 16,
        )

        block = Buffer(random_.randbytes(0x1E4))
        t.check(
            "game_unknown_00139130",
            globals={
                "unknown_00510c4c": random_.getrandbits(32),
                "unknown_004e6950": random_.randbytes(0x80),
                "unknown_004e69d0": random_.randbytes(4 * 0x270),
            },
            extra=[block],
            externals={"game_state_malloc": lambda harness, arguments, block=block: (block.address, tuple(arguments))},
        )

        told = random_.randrange(2)
        now = random_.getrandbits(32)
        t.check(
            "user_interface_unknown_0014a250",
            globals={"unknown_0054d5b8": now, "unknown_0054e7fc": (now - random_.choice((0, 199, 200, 201, 5000))) & 0xFFFFFFFF},
            externals={"user_interface_unknown_0014a224": lambda harness, arguments, told=told: (told, tuple(arguments))},
        )
