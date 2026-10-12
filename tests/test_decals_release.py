# the decals of a place let go of the cache

import random
import struct

from difftest import Buffer

from test_data import array

NONE = 0xFFFFFFFF
COUNT, SIZE = 24, 0x40


def run(t) -> None:
    random_ = random.Random(669)

    for n in range(300):
        # the marks, each in one list at most; the chain of each goes on only to later marks
        in_use = {index: random_.randint(1, 0x7FFF) for index in range(COUNT) if random_.random() < 0.9}
        table = array(random_, COUNT, SIZE, in_use)
        data = bytearray(table.data)
        data[0x29] = random_.choice((0, 1, 1, 1, 1, 0x80))
        marks = [random_.getrandbits(16) << 16 | index for index in range(COUNT)]
        salted = {mark & 0xFFFF: mark for mark in marks}
        random_.shuffle(marks)
        lists = []
        while marks:
            length = random_.randint(1, 6)
            lists.append(marks[:length])
            marks = marks[length:]
        for marks in lists:
            for at_list, mark in enumerate(marks):
                index = mark & 0xFFFF
                at = 0x4C + index * SIZE
                data[at + 2] = random_.choice((0, 2, 2, 3, 0xFD, 0xFF))
                later = NONE if index + 1 >= COUNT or random_.random() < 0.5 else salted[random_.randrange(index + 1, COUNT)]
                struct.pack_into(
                    "<4I",
                    data,
                    at + 0x30,
                    marks[at_list - 1] if at_list else NONE,
                    marks[at_list + 1] if at_list + 1 < len(marks) else NONE,
                    random_.choice((mark, mark, mark, mark ^ 0x10000, random_.getrandbits(32))),
                    later,
                )

        place = random_.choice((0xFFFF, 0, 0x1FF, random_.randrange(0x200), random_.randrange(0x200)))
        globals_ = bytearray(random_.randbytes(0x3C2C + 0x10))
        globals_[0 : 0xE00 * 4] = b"\xff" * (0xE00 * 4)
        struct.pack_into("<I", globals_, 0x3800, NONE)
        if lists and random_.random() < 0.6:
            struct.pack_into("<I", globals_, 0x3800, lists.pop()[0])
        # most of the lists are of the place that is asked for
        slots = [(list_ << 9) + (place if place != 0xFFFF else 5) for list_ in range(7)]
        random_.shuffle(slots)
        for marks in lists:
            slot = slots.pop() if slots and random_.random() < 0.8 else None
            if slot is not None:
                struct.pack_into("<I", globals_, slot * 4, marks[0])

        t.check(
            "decals_release_place",
            random_.getrandbits(16) << 16 | place,
            globals={
                "decal_data": Buffer(bytes(data), pointers=table.pointers),
                "decal_globals": Buffer(bytes(globals_)),
                "unknown_00509448": random_.getrandbits(32),
            },
            externals={"lruv_block_delete": lambda harness, arguments: 0},
        )
