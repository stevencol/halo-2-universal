# the decals, when the structure bsp goes

import random
import struct

from difftest import Buffer

from test_data import array

NONE = 0xFFFFFFFF
COUNT, SIZE = 24, 0x40


def run(t) -> None:
    random_ = random.Random(665)

    def one(low: int = 0) -> int:
        return random_.getrandbits(16) << 16 | random_.randrange(low, COUNT)

    for n in range(200):
        # the marks, each in one list at most; a list is of a place, or is that of the marks of no place
        in_use = {index: random_.randint(1, 0x7FFF) for index in range(COUNT) if random_.random() < 0.9}
        table = array(random_, COUNT, SIZE, in_use)
        data = bytearray(table.data)
        data[0x29] = random_.choice((0, 1, 1, 1, 1, 0x80))
        marks = [random_.getrandbits(16) << 16 | index for index in range(COUNT)]
        random_.shuffle(marks)
        lists = []
        while marks:
            length = random_.randint(1, 5)
            lists.append(marks[:length])
            marks = marks[length:]
        for marks in lists:
            for at_list, mark in enumerate(marks):
                at = 0x4C + (mark & 0xFFFF) * SIZE
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
        if lists and random_.random() < 0.6:
            struct.pack_into("<I", globals_, 0x3800, lists.pop()[0])
        places = random_.sample(range(0xE00), len(lists))
        for place, marks in zip(places, lists):
            if random_.random() < 0.8:
                struct.pack_into("<I", globals_, place * 4, marks[0])

        t.check(
            "decals_dispose_from_old_structure_bsp",
            globals={
                "decal_data": Buffer(bytes(data), pointers=table.pointers),
                "decal_globals": Buffer(bytes(globals_)),
            },
            externals={"unknown_0023aa70": lambda harness, arguments: 0},
        )
