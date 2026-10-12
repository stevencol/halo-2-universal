# the decals of no place find their place when a structure bsp comes

import random
import struct

from difftest import Buffer

from test_data import array

NONE = 0xFFFFFFFF
COUNT, SIZE = 24, 0x40
LEAVES, LEAF_COUNT = 0x40, 12


def some(random_) -> tuple:
    """The marks, each in one list at most, and what is kept of them: the list of the marks of no place, and those of some places."""
    in_use = {index: random_.randint(1, 0x7FFF) for index in range(COUNT) if random_.random() < 0.9}
    table = array(random_, COUNT, SIZE, in_use)
    data = bytearray(table.data)
    data[0x29] = random_.choice((0, 1, 1, 1, 1, 0x80))
    marks = [random_.getrandbits(16) << 16 | index for index in range(COUNT)]
    random_.shuffle(marks)
    lists = []
    while marks:
        length = random_.randint(1, 8)
        lists.append(marks[:length])
        marks = marks[length:]
    for marks in lists:
        for at_list, mark in enumerate(marks):
            at = 0x4C + (mark & 0xFFFF) * SIZE
            data[at + 3] = random_.randrange(7)
            struct.pack_into(
                "<2I",
                data,
                at + 0x30,
                marks[at_list - 1] if at_list else NONE,
                marks[at_list + 1] if at_list + 1 < len(marks) else NONE,
            )

    globals_ = bytearray(random_.randbytes(0x3C2C + 0x10))
    globals_[0 : 0xE00 * 4] = b"\xff" * (0xE00 * 4)
    struct.pack_into("<I", globals_, 0x3800, NONE)
    if lists and random_.random() < 0.85:
        struct.pack_into("<I", globals_, 0x3800, lists.pop()[0])
    for place, marks in zip(random_.sample(range(0xE00), len(lists)), lists):
        struct.pack_into("<I", globals_, place * 4, marks[0])
    return Buffer(bytes(data), pointers=table.pointers), Buffer(bytes(globals_))


def run(t) -> None:
    random_ = random.Random(666)

    for n in range(300):
        decals, globals_ = some(random_)
        t.check(
            "decal_add_to_place",
            random_.getrandbits(16) << 16 | random_.randrange(COUNT),
            random_.getrandbits(16) << 16 | random_.randrange(0x200),
            random_.getrandbits(16) << 16 | random_.randrange(7),
            globals={"decal_data": decals, "decal_globals": globals_},
        )

        decals, globals_ = some(random_)
        bsp = bytearray(random_.randbytes(LEAVES + LEAF_COUNT * 8))
        struct.pack_into("<I", bsp, 0x30, LEAVES)
        for leaf in range(LEAF_COUNT):
            struct.pack_into("<h", bsp, LEAVES + leaf * 8, random_.choice((-1, 0, 0x1FF, random_.randrange(0x200))))
        leaves = [random_.choice((NONE, random_.randrange(LEAF_COUNT), random_.randrange(LEAF_COUNT))) for _ in range(COUNT + 1)]

        def leaf_of(harness, arguments, leaves=leaves):
            # by how many times it has been asked
            asked = sum(1 for call in harness._calls if call[0] == "bsp3d_test_point")
            return leaves[min(asked, COUNT)]

        t.check(
            "decals_initialize_for_new_structure_bsp",
            globals={
                "decal_data": decals,
                "decal_globals": globals_,
                "global_structure_bsp_index": struct.pack("<h", random_.choice((-1, 0, 0, 0, 3))),
                "global_bsp3d": random_.getrandbits(32),
                "global_structure_bsp": Buffer(bytes(bsp), pointers=(0x30,)),
            },
            externals={"bsp3d_test_point": leaf_of},
        )
