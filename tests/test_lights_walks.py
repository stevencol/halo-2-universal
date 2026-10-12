# three walks over the lights

import random
import struct

from difftest import Buffer

from test_data import array

MAXIMUM, SIZE = 0x15E, 0x110
NONE = 0xFFFFFFFF


def lights(random_) -> Buffer:
    """The table of the lights, with some in use: their two numbers are NONE or not, and their bits are any."""
    maximum_count = random_.choice((MAXIMUM, 40, 33, 1))
    count = random_.randint(0, min(maximum_count, 24))
    in_use = {index: random_.randint(-0x8000, 0x7FFF) or 1 for index in random_.sample(range(maximum_count), count)}
    table = array(random_, maximum_count, SIZE, in_use)
    data = bytearray(table.data)
    for index in range(maximum_count):
        at = 0x4C + index * SIZE
        data[at + 2] = random_.choice((0, 2, 8, 0xA, 0xA, 0xFF, random_.randrange(0x100)))
        struct.pack_into("<I", data, at + 0x4C, random_.choice((NONE, NONE, NONE, random_.getrandbits(32))))
        struct.pack_into("<I", data, at + 0x58, random_.choice((NONE, NONE, NONE, random_.getrandbits(32))))
    return Buffer(bytes(data), pointers=table.pointers)


def given(harness, arguments) -> tuple:
    """What to compare of a call to cluster_partition_remove: that it is given the three things of the lights."""
    return 0, (
        arguments[0] == harness.global_addresses["light_cluster_partition"],
        *map(harness.normalize, arguments[1:]),
    )


def run(t) -> None:
    random_ = random.Random(639)

    for n in range(150):
        t.check(
            "lights_unknown_000c00a0",
            globals={"light_data": lights(random_)},
            externals={"unknown_000c2d00": lambda harness, arguments: 0},
        )
        t.check(
            "lights_unknown_000c0110",
            globals={"light_data": lights(random_), "light_cluster_partition": random_.randbytes(12)},
            externals={"cluster_partition_remove": given},
        )
        t.check(
            "lights_unknown_000c01c0",
            globals={"light_data": lights(random_)},
            externals={"light_delete": lambda harness, arguments: 0},
        )
