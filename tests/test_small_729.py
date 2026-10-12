# seven functions of the command scripts and the swarms

import random
import struct

from difftest import Buffer, GlobalData

from test_small_709 import COUNT, OBJECT, objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
SCRIPTS, SCRIPT = 3, 0xD4
SWARMS, SWARM = 3, 0x34


def run(t) -> None:
    random_ = random.Random(729)

    for n in range(200):
        def table(count: int, size: int, fill=None) -> Buffer:
            data = bytearray(random_.randbytes(0x4C + count * size))
            struct.pack_into("<I", data, 0x44, 0x4C)
            for index in range(count):
                if fill:
                    fill(data, 0x4C + index * size, index)
            return Buffer(bytes(data), pointers=(0x44,))

        def scripts() -> dict:
            return {
                "command_script_data": table(SCRIPTS, SCRIPT),
                "unknown_00502410": random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(SCRIPTS), random_.getrandbits(16) << 16 | random_.randrange(SCRIPTS))),
            }

        low = lambda: random_.getrandbits(24) << 8 | random_.choice((0, 1, 1, 0xFF))
        for name in ("command_script_unknown_00276f80", "command_script_unknown_00277060"):
            t.check(name, low(), random_.getrandbits(32), globals=scripts())
        time = bytearray(random_.randbytes(0x24))
        struct.pack_into("<h", time, 2, random_.choice((30, 60)))
        seconds = random_.choice((0.0, 0.5, 1.0, 2.25, random_.uniform(0.0, 60.0)))
        t.check(
            "command_script_unknown_00277250",
            struct.unpack("<I", struct.pack("<f", seconds))[0],
            globals={**scripts(), "game_time_globals": Buffer(bytes(time))},
        )
        kind = lambda: random_.getrandbits(16) << 16 | random_.choice((0xFFFF, 0x8000, 0, 1, 5, 6, 7, 8))
        t.check(
            "command_script_unknown_002775d0",
            kind(),
            globals={**scripts(), "unknown_0046fca8": GlobalData(random_.randbytes(28))},
        )
        t.check(
            "command_script_unknown_00277620",
            kind(),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            globals=scripts(),
        )

        # the five things of the swarms
        number = random_.getrandbits(32)
        things = bytearray(random_.randbytes(5 * 0x50 + 0x10))
        for index in range(5):
            struct.pack_into("<I", things, index * 0x50, random_.choice((number, number, random_.getrandbits(32))))
            struct.pack_into("<I", things, index * 0x50 + 0x48, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(SWARMS))))
        t.check(
            "swarm_unknown_00290b90",
            number,
            globals={"unknown_005044c4": Buffer(bytes(things)), "swarm_data": table(SWARMS, SWARM)},
        )

        # the objects of a swarm: each names one with a bigger number, or none
        salts = [random_.getrandbits(16) for _ in range(COUNT)]
        an_object = lambda index: salts[index] << 16 | index

        def some_objects() -> Buffer:
            objects_ = objects(random_)
            data = bytearray(objects_.data)
            for one in range(COUNT):
                at = FIRST + one * OBJECT
                struct.pack_into("<I", data, at + 0x134, random_.choice((0, 0, 0, 0, 0x20)))
                struct.pack_into("<h", data, at + 0x13A, random_.choice((0x200, 0x300)))
                above = NONE if one + 1 >= COUNT or random_.random() < 0.3 else an_object(random_.randrange(one + 1, COUNT))
                for offset in (0x200, 0x300):
                    struct.pack_into("<I", data, at + offset + 0xC, above)
            return Buffer(bytes(data), pointers=objects_.pointers)

        def a_swarm(data, at, index) -> None:
            struct.pack_into("<I", data, at + 0x18, random_.choice((NONE, an_object(0), an_object(random_.randrange(COUNT)))))

        t.check(
            "swarm_unknown_00290bf0",
            random_.getrandbits(16) << 16 | random_.randrange(SWARMS),
            random_.getrandbits(32),
            globals={"swarm_data": table(SWARMS, SWARM, a_swarm), "object_header_data": some_objects()},
        )
