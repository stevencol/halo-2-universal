# three functions of the way to the main menu (guided by halo2-decompiled):
# a local player as it starts, the titles on the screen and their fade, and
# the next pair of an iterator over the animations of a mode

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(849)

    def block(keys: list[int], size: int, at: int) -> bytes:
        data = struct.pack("<iI", len(keys), at + 8)
        for key in keys:
            data += struct.pack("<i", key) + bytes(size - 4)
        return data

    for n in range(400):
        t.check(
            "unknown_0016f4b0",
            Buffer(random_.randbytes(0x358)),
            globals={
                "global_forward3d": Buffer(random_.randbytes(12)),
                "global_up3d": Buffer(random_.randbytes(12)),
                "unknown_0054e854": random_.randbytes(4),
                "global_zero_vector3d": Buffer(random_.randbytes(12)),
                "unknown_00468788": Buffer(random_.randbytes(12)),
                "global_structure_bsp_index": random_.randbytes(2),
            },
        )

        data = bytearray(random_.randbytes(0x20))
        struct.pack_into("<fB", data, 0, random_.choice((0.0, 0.02, 0.5, 0.99, 1.0, -0.1)), random_.choice((0, 1)))
        for title in range(4):
            struct.pack_into("<hh", data, 8 + title * 4, random_.choice((-1, 0, 1, 2, 5)), random_.choice((0, 10, 29, 59, 200)))
        struct.pack_into("<if", data, 0x18, random_.choice((0, 1)), random_.choice((0.0, 0.02, 1.0)))
        scenario = bytearray(random_.randbytes(0x200 + 3 * 0x24))
        struct.pack_into("<iI", scenario, 0x1F0, random_.choice((0, 2, 3)), 0x200)
        for title in range(3):
            struct.pack_into("<ff", scenario, 0x200 + title * 0x24 + 0x1C, random_.choice((0.5, 1.0)), random_.choice((0.5, 1.0, 2.0)))
        time = bytearray(random_.randbytes(0x24))
        struct.pack_into("<hf", time, 2, 30, 1 / 30)
        t.check(
            "unknown_0013c680",
            globals={
                "unknown_00510c50": Buffer(bytes(data)),
                "global_scenario": Buffer(bytes(scenario), pointers=(0x1F4,)),
                "game_time_globals": Buffer(bytes(time)),
            },
        )

        # a graph whose block at 0x34 has modes (0x14 bytes), each with a block of classes at 0x04, each with pairs at 0x0C/0x10
        modes = sorted(random_.sample((1, 2, 0x030000D9), random_.randrange(1, 4)))
        data = bytearray(random_.randbytes(0x34)) + bytearray(8)
        pointers = [0x38]
        mode_at = len(data)
        data += bytes(len(modes) * 0x14)
        struct.pack_into("<iI", data, 0x34, len(modes), mode_at)
        for place, mode in enumerate(modes):
            classes = sorted(random_.sample((5, 6, 0x030000D9), random_.randrange(0, 3)))
            classes_at = len(data)
            data += bytes(len(classes) * 0x14)
            struct.pack_into("<iiI", data, mode_at + place * 0x14, mode, len(classes), classes_at)
            pointers.append(mode_at + place * 0x14 + 8)
            for which, weapon_class in enumerate(classes):
                pairs = random_.randrange(0, 3)
                pairs_at = len(data)
                data += random_.randbytes(pairs * 8)
                struct.pack_into("<i", data, classes_at + which * 0x14, weapon_class)
                struct.pack_into("<iI", data, classes_at + which * 0x14 + 0x0C, pairs, pairs_at)
                pointers.append(classes_at + which * 0x14 + 0x10)
        iterator = bytearray(random_.randbytes(0x14))
        struct.pack_into("<hhii", iterator, 8, random_.choice((-1, 0, 1)), random_.choice((0, 1, 2, 3, 4)), random_.choice((1, 2, 9)), random_.choice((5, 6, 9)))
        t.check("havok_unknown_001dcfa0", Buffer(bytes(iterator)), Buffer(bytes(data), pointers=tuple(pointers)))
