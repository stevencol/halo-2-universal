# source/objects/objects.c: objects let go of the things that the simulation tells of them

import random
import struct

from difftest import Buffer

from test_units import COUNT, OBJECT, some_objects

FIRST = 0x4C + COUNT * 12


def run(t) -> None:
    random_ = random.Random(598)

    def told(harness, arguments):
        return 0, (arguments[0],)

    for n in range(1500):
        table = some_objects(random_)
        data = bytearray(table.data)
        for index in range(COUNT):
            struct.pack_into("<I", data, FIRST + index * OBJECT + 0xD4, random_.choice((0xFFFFFFFF, 0x1234, 0x1234)))
        objects = {"object_header_data": Buffer(bytes(data), pointers=table.pointers)}
        t.check(
            "object_detach_simulation_entity",
            random_.getrandbits(16) << 16 | random_.randrange(COUNT),
            globals=objects,
            externals={"unknown_00108e80": told},
        )

        some = random_.sample(range(COUNT), random_.choice((0, 1, 3, COUNT)))
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
            "objects_detach_simulation_entities",
            globals=objects,
            externals={"object_iterator_next": next_, "unknown_00108e80": told},
        )
