# source/networking/network_hoppers.c

import random
import struct

from difftest import Buffer, GlobalData

NONE = 0xFFFFFFFF
COUNT, SIZE = 16, 0x614


def hopper(random_) -> bytearray:
    data = bytearray(random_.randbytes(SIZE))
    struct.pack_into("<I", data, 0, random_.choice((0, 1, 2, 3, 4, 4, 5, 5, 6, NONE)))
    struct.pack_into("<3I", data, 0x5EC, *(random_.choice((0, 1, 2, 8, 16, 17, NONE, 0x80000000)) for index in range(3)))
    data[0x604] = random_.choice((0, 0, 1, 0xFF))
    return data


def run(t) -> None:
    random_ = random.Random(541)

    for n in range(2500):
        one = Buffer(bytes(hopper(random_)))
        for name in (
            "network_hopper_get_unknown5F4",
            "network_hopper_get_maximum",
            "network_hopper_unknown_00193470",
            "network_hopper_unknown_00193490",
            "network_hopper_unknown_001934b0",
            "network_hopper_unknown_001934d0",
            "network_hopper_players_valid",
        ):
            t.check(name, one)

        blob = bytearray(random_.randbytes(0x30))
        struct.pack_into("<HHH", blob, 4, random_.choice((2, 2, 2, 3, 0, 0xFFFD, 0xFFFF)), random_.getrandbits(16), random_.choice((0, 1, 1, 4, 0xFFFF)))
        table = b"".join(bytes(hopper(random_)) for index in range(COUNT))
        globals_ = {"unknown_0047d8f4": bytes(blob), "network_hoppers": GlobalData(table)}
        good = lambda harness, arguments, so=random_.choice((0, 1, 1)): (so, (harness.normalize(arguments[0]),))
        index = random_.choice((0, 1, 7, 15, 16, 17, NONE, 0x80000000, random_.randrange(16)))
        t.check("network_hoppers_index_valid", index, globals=globals_, externals={"unknown_001934f0": good})
        t.check("network_hoppers_get", index, globals=globals_, externals={"unknown_001934f0": good})
