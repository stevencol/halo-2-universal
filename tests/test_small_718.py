# nine small functions of several files

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
PLAYERS, PLAYER = 4, 0x21C
THREADS, THREAD = 3, 0x418


def table(random_, count: int, size: int) -> bytearray:
    """A table whose things are right after its head."""
    data = bytearray(random_.randbytes(0x4C + count * size))
    struct.pack_into("<I", data, 0x44, 0x4C)
    return data


def run(t) -> None:
    random_ = random.Random(718)

    for n in range(300):
        # the scenario: a block at 0x348 and things at 0x244, both after it
        count = random_.choice((0, 1, 3))
        scenario = bytearray(random_.randbytes(0x400 + 0x400))
        struct.pack_into("<2I", scenario, 0x348, count, 0x400)
        struct.pack_into("<I", scenario, 0x244, 0x400)
        a_scenario = lambda: Buffer(bytes(scenario), pointers=(0x34C, 0x244))
        index = random_.choice((0xFFFFFFFF, 0x80000000, 0, 1, 2, 3, 4))
        t.check(
            "scenario_get_unknown348",
            index,
            globals={"global_scenario": 0 if n % 7 == 0 else a_scenario()},
        )
        thing = bytearray(random_.randbytes(0x30))
        struct.pack_into("<h", thing, 0x2A, random_.choice((-1, -1, 0, 1, 5)))
        t.check("unknown_get_scenario_unknown244", Buffer(bytes(thing)), globals={"global_scenario": a_scenario()})

        # the players
        player_index = random_.getrandbits(16) << 16 | random_.randrange(PLAYERS)
        players = lambda: {"player_data": Buffer(bytes(table(random_, PLAYERS, PLAYER)), pointers=(0x44,))}
        t.check("player_clear_unknown1B8", player_index, globals=players())
        t.check("unknown_set_first_player", player_index, Buffer(random_.randbytes(0x20)), globals=players())
        t.check("unknown_set_second_player", player_index, Buffer(random_.randbytes(0x20)), globals=players())

        # the threads: each is in a frame that names the one before
        threads = table(random_, THREADS, THREAD)
        pointers = [0x44]
        for thread in range(THREADS):
            at = 0x4C + thread * THREAD
            frame = at + 0x20 + random_.randrange(0x40) * 4
            struct.pack_into("<I", threads, at + 0x10, frame)
            pointers.append(at + 0x10)
            if random_.random() < 0.7:
                struct.pack_into("<I", threads, frame, at + 0x20 + random_.randrange(0x40) * 4)
                if frame not in pointers:
                    pointers.append(frame)
        t.check(
            "hs_thread_pop_frame",
            random_.getrandbits(16) << 16 | random_.randrange(THREADS),
            globals={"hs_thread_data": Buffer(bytes(threads), pointers=tuple(pointers))},
        )

        # the globals tag and the tables of the levels
        tag = bytearray(random_.randbytes(0x1A0))
        struct.pack_into("<2I", tag, 0x178, random_.choice((0, 1, 2)), 0x180)
        levels = lambda: {
            "global_game_globals": Buffer(bytes(tag), pointers=(0x17C,) if random_.random() < 0.8 else ()),
            "campaign_level_data": Buffer(random_.randbytes(0x4C)),
            "multiplayer_level_data": Buffer(random_.randbytes(0x4C)),
        }
        if random_.random() < 0.2:
            struct.pack_into("<I", tag, 0x17C, 0)
        t.check("levels_get_campaign_level_data", globals=levels())
        t.check("levels_get_multiplayer_level_data", globals=levels())

        unknown = bytearray(random_.randbytes(0x24))
        unknown[5] = random_.choice((0, 0, 1, 0xFF))
        t.check(
            "main_unknown_000335c0",
            globals={
                "unknown_00510c50": 0 if n % 5 == 0 else Buffer(bytes(unknown)),
                "unknown_00509415": random_.choice((0, 1, 0x100, 0xFF, 0x7700)),
            },
        )
