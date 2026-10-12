# the objects of the type 7 take their two shorts from a list of the structure bsp

import random
import struct

from difftest import Buffer

from test_units import COUNT, OBJECT, some_objects

FIRST = 0x4C + COUNT * 12
LIST, ELEMENTS = 0x300, 0x310


def run(t) -> None:
    random_ = random.Random(659)

    for n in range(300):
        # the objects: some are known by the same as others
        table = some_objects(random_)
        data = bytearray(table.data)
        known = []
        for index in range(COUNT):
            one = random_.choice(known) if known and random_.random() < 0.2 else bytes(
                random_.randbytes(6) + bytes((random_.choice((0, 1, 7)), random_.choice((0, 0, 1))))
            )
            known.append(one)
            data[FIRST + index * OBJECT + 0xA4 : FIRST + index * OBJECT + 0xAC] = one
        objects = Buffer(bytes(data), pointers=table.pointers)

        # the list: things known by what the objects are, or nearly, or by anything
        count = random_.choice((0, -1, 1, 2, 5, 8))
        bsp = bytearray(random_.randbytes(ELEMENTS + max(count, 1) * 0xC))
        struct.pack_into("<I", bsp, 0x220, LIST)
        struct.pack_into("<iI", bsp, LIST, count, ELEMENTS)
        for index in range(max(count, 0)):
            one = bytearray(random_.choice(known))
            if random_.random() < 0.4:
                one[random_.randrange(8)] ^= 1 << random_.randrange(8)
            bsp[ELEMENTS + index * 0xC : ELEMENTS + index * 0xC + 8] = one

        some = random_.sample(range(COUNT), random_.choice((0, 1, 3, COUNT, COUNT)))
        salts = [random_.getrandbits(16) << 16 for _ in range(COUNT)]

        def next_(harness, arguments, some=some, salts=salts):
            # the objects of this case, one after the other: which one is next is kept in the iterator
            at = struct.unpack("<h", harness.mu.mem_read(arguments[0] + 6, 2))[0]
            harness.mu.mem_write(arguments[0] + 6, struct.pack("<h", at + 1))
            if at >= len(some):
                return 0, ()
            harness.mu.mem_write(arguments[0] + 8, struct.pack("<I", salts[at] | some[at]))
            table_at = struct.unpack("<I", harness.mu.mem_read(harness.global_addresses["object_header_data"], 4))[0]
            entries = struct.unpack("<I", harness.mu.mem_read(table_at + 0x44, 4))[0]
            return struct.unpack("<I", harness.mu.mem_read(entries + some[at] * 12 + 8, 4))[0], ()

        t.check(
            "game_unknown_00210db0_unknown_00210f00",
            globals={
                "object_header_data": objects,
                "global_structure_bsp": Buffer(bytes(bsp), pointers=(0x220, LIST + 4)),
            },
            externals={"object_iterator_next": next_},
        )
