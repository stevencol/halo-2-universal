# five short functions: network_life_cycle.c, objects.c, game_voice.c and two of sound_manager.c

import random
import struct

from difftest import Buffer

from test_real_math import f32


def run(t) -> None:
    random_ = random.Random(581)

    for n in range(1500):
        # the thing, what it has at 0x8, and the two sessions of that
        data = bytearray(random_.randbytes(0x10 + 0x40 + 0x4A00 + 0x7420))
        manager, other, session = 0x10, 0x50, 0x50 + 0x4A00
        struct.pack_into("<I", data, 8, manager)
        struct.pack_into("<I", data, manager + 0x30, other)
        struct.pack_into("<I", data, manager + 0x38, session)
        struct.pack_into("<i", data, other + 0x4980, random_.choice((0, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, -1, random_.getrandbits(31))))
        struct.pack_into("<i", data, session + 0x741C, random_.choice((0, 2, 3, 5, 8, 9, -1)))
        data[session + 0x49E8] = random_.choice((0, 1, 0x80))
        t.check(
            "network_life_cycle_unknown_00070570",
            Buffer(bytes(data), pointers=(8, manager + 0x30, manager + 0x38)),
        )

        t.check(
            "game_voice_unknown_00162420",
            globals={"unknown_004e9ae8": Buffer(random_.randbytes(0x80))},
        )

        def starts(harness, arguments):
            source = bytes(harness.mu.mem_read(arguments[0], 0x58))
            return 0, (source[:0xC], source[0x44:], arguments[1])

        t.check(
            "sound_unknown_001896c0",
            random_.getrandbits(32),
            f32(random_.choice((0.0, 1.0, 0.5, random_.uniform(0, 4)))),
            externals={"unknown_00189fe0": starts},
        )
        t.check(
            "sound_unknown_00189760",
            Buffer(struct.pack("<IIfI", random_.getrandbits(32), random_.getrandbits(32), random_.uniform(0, 4), random_.getrandbits(32))),
            externals={
                "unknown_00189fe0": starts,
                "unknown_0018d5b0": lambda harness, arguments, value=random_.getrandbits(32): (value, (arguments[0],)),
            },
        )

    for n in range(200):
        # some objects, one after the other
        count = random_.choice((0, 1, 2, 7))
        objects = Buffer(random_.randbytes(max(count, 1) * 0x100))

        def next_(harness, arguments, objects=objects, count=count):
            # the place of the last one is kept in the iterator, as the game does
            iterator = bytes(harness.mu.mem_read(arguments[0], 0x10))
            at = struct.unpack_from("<i", iterator, 8)[0] + 1
            harness.mu.mem_write(arguments[0] + 8, struct.pack("<i", at))
            return (objects.address + at * 0x100 if at < count else 0), (iterator,)

        t.check(
            "objects_unknown_0003d380",
            random_.getrandbits(32),
            extra=[objects],
            externals={"object_iterator_next": next_},
        )
