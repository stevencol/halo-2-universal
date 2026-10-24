# eight small functions: the breakable surfaces (a surface of a collision,
# their bits and a number of a surface), two of the entries of
# unknown_004ed284 and an element of the block at 0x150 of the game globals

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
PLAYER = 0x21C


def run(t) -> None:
    random_ = random.Random(798)

    def word(value: int) -> int:
        return value & 0xFFFFFFFF

    def collision(data: bytearray, at: int, pointers: list) -> None:
        """a collision at an offset: as many surfaces (8 bytes) at 0x28 as at its list at 0x2C"""
        struct.pack_into("<iI", data, at + 0x28, random_.choice((0, 1, 4)), at + 0x40)
        pointers.append(at + 0x2C)
        for surface in range(4):
            data[at + 0x40 + surface * 8 + 4] = random_.choice((0, 8, 0xF7, 0xFF))
            data[at + 0x40 + surface * 8 + 5] = random_.choice((0, 1, 3))

    def bsp_index() -> bytes:
        return struct.pack("<h", random_.choice((0, 1)))

    for n in range(400):
        level = bytearray(random_.randbytes(0x80))
        pointers = []
        collision(level, 0, pointers)
        bsp = bytearray(random_.randbytes(0x160 + 2 * 0x58 + 2 * 0xC8))
        bsp_pointers = [0x13C, 0x144]
        struct.pack_into("<IiI", bsp, 0x13C, 0x160 + 2 * 0x58 - 0x70, random_.choice((0, 1, 2, -1)), 0x160)
        for element in range(2):
            struct.pack_into("<h", bsp, 0x160 + element * 0x58 + 0x34, random_.randrange(2))
        for element in range(2):
            collision(bsp, 0x160 + 2 * 0x58 + element * 0xC8 - 0x70 + 0x70, bsp_pointers)
        t.check(
            "breakable_surfaces_unknown_00183f50",
            random_.choice((NONE, NONE, 0, 1, 2, word(-2))),
            word(random_.choice((-1, 0, 1, 3, 4))),
            random_.choice((0, 1, 3, 0x103)),
            globals={
                "global_collision_bsp": Buffer(bytes(level), pointers=tuple(pointers)) if n % 7 else 0,
                "global_structure_bsp": Buffer(bytes(bsp), pointers=tuple(bsp_pointers)),
            },
        )

        surfaces = Buffer(random_.randbytes(0x4204))
        t.check(
            "breakable_surfaces_get_bits",
            random_.choice((NONE, 0, 1, 7)),
            globals={"unknown_004ed280": surfaces, "global_structure_bsp_index": bsp_index()},
        )
        t.check(
            "breakable_surfaces_test",
            random_.choice((NONE, 0, 31, 32, 200, random_.randrange(256))),
            random_.choice((NONE, 0, 3, 8)),
            globals={"unknown_004ed280": Buffer(random_.randbytes(0x4204)), "global_structure_bsp_index": bsp_index()},
        )
        t.check(
            "breakable_surfaces_unknown_00184400",
            random_.choice((NONE, 0, 7, 255)),
            random_.choice((NONE, 0, 5, 1023)),
            globals={
                "unknown_004ea960": random_.randbytes(0x100),
                "unknown_004eaa60": random_.randbytes(0x2000),
                "unknown_004eca60": random_.randbytes(32),
            },
        )

        # players: a data array with salts
        players = bytearray(random_.randbytes(0x50 + 4 * PLAYER))
        struct.pack_into("<i", players, 0x24, PLAYER)
        struct.pack_into("<i", players, 0x38, random_.choice((0, 2, 4)))
        struct.pack_into("<I", players, 0x44, 0x50)
        salts = [random_.choice((0, 0x1234, 0x5678)) for _ in range(4)]
        for player in range(4):
            struct.pack_into("<H", players, 0x50 + player * PLAYER, salts[player])
            struct.pack_into("<I", players, 0x50 + player * PLAYER + 0x2C, random_.choice((NONE, 3)))
        entries = bytearray(random_.randbytes(4 * 0x94))
        for entry in range(4):
            player = random_.randrange(4)
            struct.pack_into("<I", entries, entry * 0x94 + 0x60, random_.choice((NONE, salts[player] << 16 | player, 0x1234 << 16 | player)))
        t.check(
            "sound_unknown_00187450",
            random_.randrange(4),
            globals={"unknown_004ed284": Buffer(bytes(entries)), "player_data": Buffer(bytes(players), pointers=(0x44,))},
        )
        t.check("sound_unknown_00187a40", random_.choice((NONE, 0, 3)), globals={"unknown_004ed284": Buffer(bytes(entries))})

        tag = bytearray(random_.randbytes(0x160 + 4 * 0xB4))
        struct.pack_into("<iI", tag, 0x150, random_.choice((0, 1, 4, -1)), 0x160)
        for element in range(4):
            struct.pack_into("<I", tag, 0x160 + element * 0xB4, random_.choice((1, 2, 3)))
        t.check("sound_unknown_001885f0", Buffer(bytes(tag), pointers=(0x154,)), random_.choice((1, 2, 3, 4)), Buffer(random_.randbytes(2)))
        t.check(
            "sound_unknown_00188690",
            random_.getrandbits(16) << 16 | word(random_.choice((-1, 0, 1, 3, 4, -2))) & 0xFFFF,
            globals={"global_game_globals": Buffer(bytes(tag), pointers=(0x154,))},
        )
