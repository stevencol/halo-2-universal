# five small functions of the way to the main menu: the bigger of two reals,
# a row of bits copied, a point for a tree of particle systems, a particle
# system out of an effect, and a time of an object

import random
import struct

from difftest import Buffer

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
SYSTEM = 0x54
EFFECT = 0x190


def run(t) -> None:
    random_ = random.Random(824)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    for n in range(400):
        # something whose thing (at 0x08) points to a definition at 0x48
        data = bytearray(random_.randbytes(0x70))
        struct.pack_into("<I", data, 0x08, 0x10)
        struct.pack_into("<I", data, 0x10 + 0x48, 0x5C)
        data[0x5C + 1] = random_.choice((0, 0x0F, 0x10, 0xF3))
        struct.pack_into("<ff", data, 0x5C + 4, random_.uniform(-2, 2), random_.uniform(-2, 2))
        t.check("havok_unknown_00224350", Buffer(bytes(data), pointers=(0x08, 0x10 + 0x48)))

        # bits: how many at 0x230, their rows at 0x250 (three rows of three longs)
        count = random_.choice((0, 1, 31, 32, 33, 70))
        data = bytearray(random_.randbytes(0x250 + 3 * 12))
        struct.pack_into("<iI", data, 0x224, random_.choice((0, 1, -1, 2)), 0x230)
        struct.pack_into("<i", data, 0x230, count)
        struct.pack_into("<I", data, 0x230 + 0x18, 0x250)
        t.check(
            "unknown_00249d10",
            Buffer(bytes(data), pointers=(0x228, 0x248)),
            Buffer(random_.randbytes(16)),
            random_.randrange(3) if count <= 32 else random_.randrange(2),
        )

        # particle systems: a tree under the one given, each with those under it at 0x40 and the next at 0x38
        systems = bytearray(random_.randbytes(0x50 + 6 * SYSTEM))
        struct.pack_into("<I", systems, 0x44, 0x50)
        root = bytearray(random_.randbytes(SYSTEM))
        struct.pack_into("<I", root, 0x40, NONE)
        ids = [system if system == 0 else salted(system) for system in range(6)]
        for system in range(6):
            struct.pack_into("<II", systems, 0x50 + system * SYSTEM + 0x38, NONE, NONE)
            struct.pack_into("<I", systems, 0x50 + system * SYSTEM + 0x40, NONE)
        for system in range(6):
            parent = random_.randrange(-1, system)
            at = 0x50 + parent * SYSTEM
            owner = root if parent < 0 else systems
            place = 0x40 if parent < 0 else at + 0x40
            struct.pack_into("<I", systems, 0x50 + system * SYSTEM + 0x38, struct.unpack_from("<I", owner, place)[0])
            struct.pack_into("<I", owner, place, ids[system])
        t.check(
            "particle_system_unknown_00175180",
            Buffer(bytes(root)),
            Buffer(random_.randbytes(8)),
            globals={"particle_system_data": Buffer(bytes(systems), pointers=(0x44,))},
        )

        # four particle systems in a list of an effect, one of them in its pairs
        systems = bytearray(random_.randbytes(0x50 + 4 * SYSTEM))
        struct.pack_into("<I", systems, 0x44, 0x50)
        salts = [random_.getrandbits(16) for _ in range(4)]
        ids = [salts[system] << 16 | system for system in range(4)]
        for system in range(4):
            at = 0x50 + system * SYSTEM
            struct.pack_into("<H", systems, at, salts[system])
            struct.pack_into("<II", systems, at + 0x38, ids[system + 1] if system < 3 else NONE, ids[system - 1] if system else NONE)
        effects = bytearray(random_.randbytes(0x50 + 2 * EFFECT))
        struct.pack_into("<I", effects, 0x44, 0x50)
        chosen = random_.randrange(4)
        for effect in range(2):
            struct.pack_into("<II", effects, 0x50 + effect * EFFECT + 0xFC, ids[0], ids[3])
            for pair in range(16):
                struct.pack_into("<I", effects, 0x50 + effect * EFFECT + 0x110 + pair * 8 + 4, random_.choice((ids[chosen], 1, 2)))
        t.check(
            "effect_unknown_0017af80",
            salted(random_.randrange(2)),
            ids[chosen],
            globals={"particle_system_data": Buffer(bytes(systems), pointers=(0x44,)), "effect_data": Buffer(bytes(effects), pointers=(0x44,))},
        )

        # objects are 0x250 bytes here, so the byte at 0x3AA is in the next one (or after the last)
        table = some_objects(random_)
        objects = bytearray(table.data) + bytearray(random_.randbytes(0x200))
        for index in range(COUNT):
            objects[FIRST + index * OBJECT + 0x3AA] = random_.choice((0, 1, 5, 0xFE))
        time = bytearray(random_.randbytes(0x24))
        struct.pack_into("<f", time, 4, random_.choice((1 / 30, 1 / 60)))
        t.check(
            "object_unknown_000e0d70",
            salted(random_.randrange(COUNT)),
            Buffer(random_.randbytes(4)),
            globals={"object_header_data": Buffer(bytes(objects), pointers=table.pointers), "game_time_globals": Buffer(bytes(time))},
        )
