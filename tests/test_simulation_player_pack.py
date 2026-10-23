# source/networking/simulation_player.c: what a player does is made small for a message

import random
import struct

from difftest import Buffer

UNIT = 0x244


def run(t) -> None:
    random_ = random.Random(589)

    for n in range(3000):
        # the table of the objects: its entries, of 12 bytes, and the units after them
        headers = bytearray(random_.randbytes(0x4C + 4 * 12))
        struct.pack_into("<I", headers, 0x44, 0x4C)
        pointers = [0x44]
        units = bytearray()
        for index in range(4):
            struct.pack_into("<I", headers, 0x4C + index * 12 + 8, len(headers) + len(units))
            pointers.append(0x4C + index * 12 + 8)
            unit = bytearray(random_.randbytes(UNIT))
            unit[0x212] = random_.choice((0, 1, 0xFF))
            unit[0x213] = random_.choice((0, 1, 0xFF))
            units += unit
        action = bytearray(random_.randbytes(0x5C))
        struct.pack_into("<I", action, 0, random_.choice((0, 1, 2, 3, 0x800, 0x4000, 0x4803, random_.getrandbits(32))))
        has_unit = random_.choice((False, True, True))
        unit_index = (random_.getrandbits(16) << 16 | random_.randrange(4)) if has_unit else 0xFFFFFFFF
        if has_unit and random_.choice((False, True)):
            action[0x1E:0x20] = units[(unit_index & 0xFFFF) * UNIT + 0x210 : (unit_index & 0xFFFF) * UNIT + 0x212]
        if not has_unit and random_.choice((False, True)):
            action[0x1E:0x20] = b"\xff\xff"
        action[0x20] = random_.choice((0, 1, 0xFF))
        action[0x21] = random_.choice((0, 1, 0xFF))
        struct.pack_into("<h", action, 0x24, random_.choice((-1, -1, 3, 0x7F)))

        def rest(harness, arguments):
            return 0, (arguments[0], harness.normalize(arguments[1]), bytes(harness.mu.mem_read(arguments[2], 0x34)))

        t.check(
            "simulation_player_action_pack",
            Buffer(random_.randbytes(0x34)),
            Buffer(bytes(action)),
            unit_index,
            globals={"object_header_data": Buffer(bytes(headers) + bytes(units), pointers=tuple(pointers))},
            externals={
                "game_unknown_001a1c8e_object_fast_enough": lambda harness, arguments, value=random_.getrandbits(32): (value, (arguments[0],)),
                "unknown_00082d20": rest,
            },
        )
