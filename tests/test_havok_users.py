# one of two users of the physics' second memory lets go of it

import random
import struct


def run(t) -> None:
    random_ = random.Random(687)

    for n in range(400):
        numbers = [random_.getrandbits(32) for _ in range(3)]
        users = bytearray(random_.randbytes(0x18))
        for index in range(2):
            users[index * 0xC] = random_.choice((0, 0, 1, 0x80))
            struct.pack_into("<I", users, index * 0xC + 4, random_.choice(numbers))
        t.check(
            "havok_unknown_001cab80",
            random_.choice(numbers),
            globals={
                "unknown_004f55b4": bytes(users),
                "unknown_0051e9b0": random_.choice((0, 0, 1, random_.getrandbits(32))),
                "unknown_0051e9b4": random_.randbytes(1),
                "unknown_00510c48": random_.randbytes(1),
                # the thing that gives the memory would be at the address zero: none is made
                "unknown_00479888": (-0x105410) & 0xFFFFFFFF,
                "unknown_00510c44": random_.getrandbits(32),
                "unknown_0047989c": random_.getrandbits(32),
            },
        )
