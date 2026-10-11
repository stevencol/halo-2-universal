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
        t.check("network_hoppers_index_valid", index, globals=globals_, externals={"network_hopper_valid": good})
        t.check("network_hoppers_get", index, globals=globals_, externals={"network_hopper_valid": good})

    # ---------- what one is asked by its kind: the game does not look at whether the kind is one of the five

    for n in range(2500):
        data = hopper(random_)
        struct.pack_into("<I", data, 0, random_.randrange(1, 6))
        struct.pack_into("<3I", data, 0x5EC, random_.choice((1, 1, 2, 4)), random_.choice((1, 2, 3, 4)), random_.choice((1, 2, 3, 4)))
        struct.pack_into("<I", data, 0x5F8, random_.choice((0, 1, 0x100, 0x1FF, random_.getrandbits(32))))
        one = Buffer(bytes(data))
        t.check("network_hopper_get_players_value", one, random_.choice((0, 1, 2, 3, 4, 6, 8, 12, 16, 17, NONE, random_.randrange(18))))
        for name in (
            "network_hopper_unknown_00193250",
            "network_hopper_get_minimum_players",
            "network_hopper_get_maximum_players",
            "network_hopper_get_minimum_teams",
            "network_hopper_get_maximum_teams",
            "network_hopper_unknown_00193400",
            "network_hopper_get_team_size",
        ):
            t.check(name, one)
        says = lambda so: (lambda harness, arguments: (so, (bytes(harness.mu.mem_read(arguments[0], 4)),)))
        t.check(
            "network_hopper_valid",
            one,
            externals={
                "unknown_00193560": says(random_.choice((0, 1, 1, 1))),
                "network_hopper_players_valid": says(random_.choice((0, 1))),
                "unknown_00193630": says(random_.choice((0, 1))),
                "unknown_001936a0": says(random_.choice((0, 1))),
            },
        )
