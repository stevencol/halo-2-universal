# eight small functions: the cluster of a point of the scenario in the game
# globals, a long of a tag, sums of two things of Havok, entries of three
# blocks by the players and whether a list is empty

import random
import struct

from difftest import Buffer, f32

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
PLAYER = 0x21C


def run(t) -> None:
    random_ = random.Random(793)

    def word(value: int) -> int:
        return value & 0xFFFFFFFF

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    def players() -> Buffer:
        data = bytearray(random_.randbytes(0x50 + 4 * PLAYER))
        struct.pack_into("<I", data, 0x44, 0x50)
        for player in range(4):
            struct.pack_into("<hhI", data, 0x50 + player * PLAYER + 0x28, random_.choice((-1, 0, 1, 3)), 0, salted(random_.randrange(COUNT)))
        return Buffer(bytes(data), pointers=(0x44,))

    for n in range(400):
        cluster = random_.choice((-1, 0, 7))

        def located(harness, arguments, cluster=cluster):
            # the location: its cluster at 4
            harness.mu.mem_write(arguments[0] + 4, struct.pack("<h", cluster))
            return 0, (harness.mu.mem_read(arguments[1], 12),)

        scenario = bytearray(random_.randbytes(0x1F0 + 4 * 0x40))
        struct.pack_into("<I", scenario, 0x1EC, 0x1F0)
        t.check(
            "main_game_unknown_00138a40",
            random_.getrandbits(16) << 16 | random_.choice((0xFFFF, 0, 1, 3)),
            globals={"global_scenario": Buffer(bytes(scenario), pointers=(0x1EC,)), "main_game_globals": Buffer(random_.randbytes(0x1200))},
            externals={"scenario_location_from_point": located},
        )

        # something naming a tag at 0x0C, which names another at 0x24
        tags = bytearray(random_.randbytes(0x40 + 0x80 + 0x10 + 4 * 0x0C))
        struct.pack_into("<I", tags, 0 * 0x10 + 8, 0x40)
        struct.pack_into("<I", tags, 0x40 + 0x24, 0x80)
        struct.pack_into("<II", tags, 0x80 + 0x08, 0xC0 + 0x10, 0)
        struct.pack_into("<I", tags, 0x80 + 0x60, 0xC0)
        struct.pack_into("<h", tags, 0xC0, random_.choice((-1, 0, 1, 3)))
        data = bytearray(random_.randbytes(0x10))
        struct.pack_into("<I", data, 0x0C, random_.choice((NONE, random_.getrandbits(16) << 16)))
        t.check(
            "tag_unknown_0013da30",
            random_.choice((0, Buffer(bytes(data)), Buffer(bytes(data)))),
            globals={"global_tag_instances": Buffer(bytes(tags), pointers=(8, 0x40 + 0x24, 0x80 + 0x08, 0x80 + 0x60))},
        )

        def thing():
            if random_.random() < 0.2:
                return 0
            count = random_.choice((0, 1, 3, 8, -1))
            data = bytearray(random_.randbytes(0x24 + 8 * 8))
            struct.pack_into("<iIi", data, 0x18, 8 * 8, 0x24, count)
            return Buffer(bytes(data), pointers=(0x1C,))

        t.check("havok_unknown_00147090", globals={"unknown_0047989c": thing(), "unknown_004798a0": thing()})

        player = salted(random_.randrange(4))
        block = bytearray(random_.randbytes(4 * 0x94 + 0x40))
        t.check("player_unknown_001520f0", player, globals={"player_data": players(), "unknown_004ed284": Buffer(bytes(block))})

        table = some_objects(random_)
        t.check(
            "player_unknown_00152240",
            player,
            random_.getrandbits(16) << 16 | random_.choice((0, 0, 1, 0xFFFF)),
            globals={"player_data": players(), "object_header_data": Buffer(table.data + random_.randbytes(0x100), pointers=table.pointers)},
            externals={"unknown_000b58c0": lambda harness, arguments: (0, tuple(arguments))},
        )

        t.check("unknown_00510c5c_clear_entry", random_.choice((NONE, 0, 1, 3)), globals={"unknown_00510c5c": Buffer(random_.randbytes(0x30 + 4 * 0xA0))})
        t.check("player_unknown_00153cd0", player, globals={"player_data": players(), "unknown_00502120": Buffer(random_.randbytes(4 * 0x88))})

        values = struct.pack("<16I", *(random_.choice((0, 0, 0, 1, NONE)) for _ in range(16)))
        t.check("unknown_004e9148_empty", globals={"unknown_00510c64": random_.choice((0, 1, 4, 16, -1)), "unknown_004e9148": values})
