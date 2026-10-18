# eight small functions of the network session, blobs and players

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
NODE = 0x1A0 + 0x24
PEER = 0x10C


def run(t) -> None:
    random_ = random.Random(741)

    for n in range(300):
        # something with a list at 0xC: its nodes are in the same buffer
        count = random_.choice((0, 1, 2, 4))
        data = bytearray(random_.randbytes(0x10 + count * NODE))
        pointers = []
        struct.pack_into("<I", data, 0xC, 0x10 if count else 0)
        if count:
            pointers.append(0xC)
        for index in range(count):
            at = 0x10 + index * NODE
            if index + 1 < count:
                struct.pack_into("<I", data, at, at + NODE)
                pointers.append(at)
            else:
                struct.pack_into("<I", data, at, 0)
        other = bytearray(random_.randbytes(0x188 + 0x24))
        if count and random_.random() < 0.6:
            index = random_.randrange(count)
            other[0x188:] = data[0x10 + index * NODE + 0x1A0 : 0x10 + index * NODE + NODE]
            if random_.random() < 0.3:
                other[0x188 + random_.randrange(0x24)] ^= 1
        t.check("network_session_unknown_0006de10", Buffer(bytes(data), pointers=tuple(pointers)), Buffer(bytes(other)))

        size = random_.choice((0, -1, NONE & -5, 1, 3, 4, 7, 0x40, random_.randrange(0x41)))
        t.check(
            "network_session_unknown_0006df60",
            Buffer(random_.randbytes(0x54 + 0x40)),
            random_.getrandbits(32),
            size & 0xFFFFFFFF,
            Buffer(random_.randbytes(0x40)),
        )

        # something, what it has at 8 (at 0x108) and the session (at 0x148)
        data = bytearray(random_.randbytes(0x148 + 0x7420))
        struct.pack_into("<I", data, 8, 0x108)
        struct.pack_into("<I", data, 0x108 + 0x34, 0x148)
        struct.pack_into("<I", data, 0x148 + 0x741C, random_.choice((0, 1, 2, 3, 4, 8, 9, NONE, 0x80000000)))
        t.check("network_life_cycle_unknown_0006fc80", Buffer(bytes(data), pointers=(8, 0x108 + 0x34)))

        peers = random_.choice((0, 1, 2, 5, 16))
        data = bytearray(random_.randbytes(0x148 + 0x58 + 16 * PEER))
        struct.pack_into("<I", data, 8, 0x108)
        struct.pack_into("<I", data, 0x108 + 0x38, 0x148)
        struct.pack_into("<i", data, 0x148 + 0x54, random_.choice((peers, peers, -1)))
        for index in range(16):
            struct.pack_into("<I", data, 0x148 + 0x58 + index * PEER + 0xEC, random_.choice((2, 2, 2, 2, 2, 1, 3)))
        t.check("network_life_cycle_unknown_00072140", Buffer(bytes(data), pointers=(8, 0x108 + 0x38)))

        t.check("network_blob_unknown_00080de0", Buffer(random_.randbytes(16)), Buffer(random_.randbytes(0x54)))

        index = random_.randrange(32)
        t.check(
            "simulation_player_unknown_0008b830",
            Buffer(random_.randbytes(0x720 + 32 * 0x40)),
            index,
            Buffer(random_.randbytes(0x40)),
        )

        t.check("network_blob_task_unknown_0008eeb0", Buffer(random_.randbytes(0x220)))

        t.check(
            "network_blob_task_unknown_0008ef60",
            Buffer(random_.randbytes(0x220)),
            random_.choice((0, 1, 0x4999, 0x499A, 0x499B, NONE, 0x80000000, random_.getrandbits(32))),
            random_.getrandbits(32),
            random_.getrandbits(32),
        )
