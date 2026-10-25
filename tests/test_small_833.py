# three small functions of the way to the main menu: how far a unit has
# counted with a weapon, the two tasks of the network life cycle starting,
# and a value of a table at a real

import random
import struct

from difftest import Buffer, f32

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12


def run(t) -> None:
    random_ = random.Random(833)

    for n in range(400):
        table = some_objects(random_)
        objects = bytearray(table.data)
        weapons = [0] + [random_.getrandbits(16) << 16 | index for index in range(1, 3)]
        for index in range(COUNT):
            at = FIRST + index * OBJECT
            objects[at + 0x1FA] = random_.choice((0, 1, 5, 9, 20))
            objects[at + 0x1FB] = random_.choice((0, 2, 9, 12))
            objects[at + 0x212] = random_.choice((0xFF, 0, 1, 2))
            for slot in range(4):
                struct.pack_into("<I", objects, at + 0x218 + slot * 4, random_.choice(weapons + [NONE]))
        time = bytearray(random_.randbytes(0x24))
        ticks = random_.choice((30, 60))
        struct.pack_into("<hf", time, 2, ticks, 1 / ticks)
        t.check(
            "unit_unknown_000d1490",
            random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(COUNT))),
            random_.choice(weapons + [NONE]),
            globals={"object_header_data": Buffer(bytes(objects), pointers=table.pointers), "game_time_globals": Buffer(bytes(time))},
        )

        t.check(
            "network_life_cycle_unknown_00072c80",
            random_.getrandbits(32),
            random_.getrandbits(32),
            globals={
                "unknown_0047708c": random_.randbytes(0xA0),
                "unknown_0047712c": random_.randbytes(0xA0),
                "unknown_00477088": bytes((random_.randrange(2),)),
            },
        )

        count = random_.choice((1, 2, 4, 9))
        t.check(
            "sound_unknown_00185b30",
            count | random_.getrandbits(16) << 16,
            Buffer(struct.pack(f"<{count}f", *(random_.uniform(-2, 2) for _ in range(count)))),
            f32(random_.choice((0.0, 1.0, -1.0, 0.5, -0.25, 1.5, random_.uniform(-1, 1)))),
        )
