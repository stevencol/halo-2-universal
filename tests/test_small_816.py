# three medium functions of the way to the main menu: two lists of the
# engine filled with longs, what a unit is in the seat of its parent, and
# where the things of an object are

import random
import struct

from difftest import Buffer, GlobalData

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
SEAT = 0xB0
DEFINITION = 0x1D0
ENTRY = 0xD0C


def run(t) -> None:
    random_ = random.Random(816)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    def tags_with(definitions: list[bytearray], seats: int = 0) -> Buffer:
        """Two tags with their definitions after them (and, after those, seats for each)."""
        data = bytearray(random_.randbytes(0x20))
        pointers = []
        for tag in range(2):
            struct.pack_into("<I", data, tag * 0x10 + 8, 0x20 + tag * DEFINITION)
            pointers.append(tag * 0x10 + 8)
        for tag, definition in enumerate(definitions):
            if seats:
                struct.pack_into("<I", definition, 0x1CC, 0x20 + 2 * DEFINITION + tag * seats * SEAT)
                pointers.append(0x20 + tag * DEFINITION + 0x1CC)
            data += definition
        for _ in range(2 * seats):
            seat = bytearray(random_.randbytes(SEAT))
            struct.pack_into("<I", seat, 0, random_.getrandbits(32) & ~0x50 | random_.choice((0, 0x10, 0x40, 0x50)))
            data += seat
        return Buffer(bytes(data), pointers=tuple(pointers))

    for n in range(400):
        # the owner (0x20 bytes: lists at 0x0C to 0x18), four lists, and their arrays for six
        owner = bytearray(random_.randbytes(0x20 + 4 * 0x18 + 4 * 0x48))
        pointers = []
        for slot in range(4):
            struct.pack_into("<I", owner, 0x0C + slot * 4, 0x20 + slot * 0x18)
            pointers.append(0x0C + slot * 4)
            at = 0x20 + slot * 0x18
            arrays = 0x80 + slot * 0x48
            struct.pack_into("<iH", owner, at, random_.choice((0, 1, 3, 6)), random_.choice((0, 1, 2, 4, 7)))
            struct.pack_into("<IIII", owner, at + 8, arrays, arrays + 12, arrays + 36, arrays + 60)
            pointers += [at + 8, at + 12, at + 16, at + 20]
        first_count = random_.choice((-1, 0, 1, 3, 6))
        second_count = random_.choice((-1, 0, 1, 2, 6))
        t.check(
            "game_engine_unknown_00165bc0",
            first_count,
            Buffer(random_.randbytes(24)),
            Buffer(bytes(owner), pointers=tuple(pointers)),
            Buffer(random_.randbytes(24)),
            second_count,
            random_.choice((0, 1, 0x100)),
        )

        # a unit (an object of the table) whose parent is another object, with seats in its definition
        table = some_objects(random_)
        objects = bytearray(table.data)
        for index in range(COUNT):
            at = FIRST + index * OBJECT
            struct.pack_into("<I", objects, at, salted(random_.randrange(2)))
            struct.pack_into("<h", objects, at + 0x1FC, random_.randrange(3))
        found = random_.random() < 0.85
        unit = random_.randrange(COUNT)
        number = random_.choice((0x0400004A, 0x0400004A, 7))
        in_seat = random_.choice((0, 1))
        headers = Buffer(bytes(objects), pointers=table.pointers)
        t.check(
            "unit_unknown_00155810",
            salted(unit),
            Buffer(random_.randbytes(2)),
            globals={
                "object_header_data": headers,
                "global_tag_instances": tags_with([bytearray(random_.randbytes(DEFINITION)) for _ in range(2)], seats=3),
            },
            externals={
                "object_try_and_get_and_verify_type": lambda harness, arguments, headers=headers, at=FIRST + unit * OBJECT, found=found: (
                    headers.address + at if found else 0,
                    tuple(arguments),
                ),
                "object_unknown_0010f5f0": lambda harness, arguments, number=number: (number, tuple(arguments)),
                "unit_unknown_000c7070": lambda harness, arguments, in_seat=in_seat: (in_seat, tuple(arguments)),
            },
        )

        # four entries of a tag and numbers, and objects whose definitions lead to a tag by two more
        table = some_objects(random_)
        objects = bytearray(table.data)
        for index in range(COUNT):
            at = FIRST + index * OBJECT
            struct.pack_into("<I", objects, at, salted(random_.randrange(2)))
            struct.pack_into("<hh", objects, at + 0x114, random_.choice((0, 0x34, 0x100, -0x34)), random_.choice((0, 0x20, 0x118)))
        entries = bytearray(random_.randbytes(4 * ENTRY))
        for entry in range(4):
            struct.pack_into("<I", entries, entry * ENTRY, salted(random_.randrange(2)))
        definitions = []
        for tag in range(2):
            definition = bytearray(random_.randbytes(DEFINITION))
            struct.pack_into("<II", definition, 0x00, 0, salted(random_.randrange(2)))
            struct.pack_into("<I", definition, 0x38, salted(random_.randrange(2)))
            struct.pack_into("<I", definition, 0x48, random_.choice((0, 5, 0x40, 0x41, NONE)))
            definitions.append(definition)
        t.check(
            "object_unknown_0003d4f0",
            random_.choice((0, 1, 0x100)),
            salted(random_.randrange(COUNT)),
            random_.getrandbits(16) << 16 | random_.randrange(4),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4), places=(0,)),
            Buffer(random_.randbytes(4)),
            globals={
                "object_header_data": Buffer(bytes(objects), pointers=table.pointers),
                "global_tag_instances": tags_with(definitions),
                "unknown_004c1bd8": GlobalData(bytes(entries)),
            },
        )
