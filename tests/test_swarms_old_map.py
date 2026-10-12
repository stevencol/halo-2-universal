# the swarms, when a map goes

import random
import struct

from difftest import Buffer

OBJECT_SIZE = 0x200


def run(t) -> None:
    random_ = random.Random(651)

    for n in range(200):
        # some objects, one after the other
        count = random_.choice((0, 1, 2, 7))
        data = bytearray(random_.randbytes(max(count, 1) * OBJECT_SIZE))
        for index in range(count):
            at = index * OBJECT_SIZE
            struct.pack_into("<I", data, at + 0x134, random_.choice((0, 0, 0, 1, random_.getrandbits(32))))
            struct.pack_into("<h", data, at + 0x13A, random_.choice((0x140, 0x1E0, random_.randrange(0x140, 0x1F0))))
        objects = Buffer(bytes(data))

        def next_(harness, arguments, objects=objects, count=count):
            # the place of the last one is kept in the iterator, as the game does;
            # the byte at 5 is nothing that the function sets
            iterator = bytearray(harness.mu.mem_read(arguments[0], 0x10))
            iterator[5] = 0
            at = struct.unpack_from("<i", iterator, 8)[0] + 1
            harness.mu.mem_write(arguments[0] + 8, struct.pack("<i", at))
            return (objects.address + at * OBJECT_SIZE if at < count else 0), (bytes(iterator),)

        t.check(
            "swarms_dispose_from_old_map",
            globals={"swarm_data": Buffer(random_.randbytes(0x4C + 0x10))},
            extra=[objects],
            externals={"object_iterator_next": next_},
        )
