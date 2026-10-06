# source/input/input_abstraction.c

import random
import struct

from difftest import Buffer
from test_async import calls_so_far

NONE = 0xFFFFFFFF
GLOBALS_SIZE = 0x1BC
INPUT_GLOBALS_SIZE = 0x1B0


def run(t) -> None:
    random_ = random.Random(258)

    # ---------- how a controller is set when nothing else says

    for _ in range(20):
        t.check("input_abstraction_preferences_set_defaults", Buffer(random_.randbytes(0x1C)))

    # ---------- all starts, and ends

    for _ in range(60):
        gamepads = bytearray(random_.randbytes(INPUT_GLOBALS_SIZE))
        for index in range(4):
            struct.pack_into("<I", gamepads, 0x14 + index * 4, random_.choice((0, 0, 1, 0x100, random_.getrandbits(32))))
        t.check(
            "input_abstraction_initialize",
            globals={"input_abstraction_globals": random_.randbytes(GLOBALS_SIZE), "input_globals": bytes(gamepads)},
            externals={"GetTickCount": lambda harness, arguments, time=random_.getrandbits(32): time},
        )
    for _ in range(10):
        t.check("input_abstraction_dispose", globals={"input_abstraction_globals": random_.randbytes(GLOBALS_SIZE)})

    # ---------- the gamepads that were plugged in and unplugged

    for n in range(800):
        state = bytearray(random_.randbytes(GLOBALS_SIZE))
        now = random_.getrandbits(32)
        started = (now - random_.choice((0, 1999, 2000, 2001, 100000, 5))) & 0xFFFFFFFF
        struct.pack_into("<I", state, 0x1B0, started)
        state[0x1B8] = random_.choice((1, 1, 1, 1, 0, 0x80))
        first = random_.choice((0, 0, (now + 7 - random_.choice((0, 1999, 2000, 2001, 100000))) & 0xFFFFFFFF))
        times = (now, now + 7, now + 20)

        def clock_(harness, arguments, times=times):
            return times[calls_so_far(harness, "GetTickCount") % 3]

        t.check(
            "input_abstraction_handle_gamepad_changes",
            random_.choice((0, 0, 1, 8, 0x1000, 0x8000, 0x1001, 0xFFF000, 0x1000000, random_.getrandbits(32))),
            globals={
                "input_abstraction_globals": bytes(state),
                "unknown_0055e77c": struct.pack("<I", first),
                "unknown_004e9188": bytes([random_.choice((1, 1, 1, 0, 0x80))]),
                "unknown_004e9190": struct.pack("<I", random_.choice((0, 0, 0x20, 0x100, 0x400, 0x500, 0x200, 0xFFFFFAFF, random_.getrandbits(32)))),
            },
            externals={"GetTickCount": clock_, "unknown_001565a0": lambda harness, arguments: 0},
        )

    # ---------- the seat of the unit of the player of a controller

    PLAYERS = 4
    OBJECTS = 8
    TAGS = 16
    for n in range(900):
        players = bytearray(random_.randbytes(0x2C))
        for index in range(4):
            struct.pack_into("<I", players, 0x1C + index * 4, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(PLAYERS), random_.getrandbits(16) << 16 | random_.randrange(PLAYERS))))
        player_data = bytearray(random_.randbytes(0x4C + PLAYERS * 0x21C))
        struct.pack_into("<I", player_data, 0x44, 0x4C)
        for index in range(PLAYERS):
            struct.pack_into("<I", player_data, 0x4C + index * 0x21C + 0x2C, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(OBJECTS), random_.getrandbits(16) << 16 | random_.randrange(OBJECTS), random_.getrandbits(16) << 16 | random_.randrange(OBJECTS))))
        headers = bytearray(random_.randbytes(0x4C + OBJECTS * 0xC))
        struct.pack_into("<I", headers, 0x44, 0x4C)
        pointers = [0x44]
        for index in range(OBJECTS):
            struct.pack_into("<I", headers, 0x4C + index * 0xC + 8, len(headers))
            pointers.append(0x4C + index * 0xC + 8)
            thing = bytearray(random_.randbytes(0x200))
            struct.pack_into("<I", thing, 0, random_.getrandbits(16) << 16 | random_.randrange(TAGS))
            struct.pack_into("<I", thing, 0x14, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(OBJECTS), random_.getrandbits(16) << 16 | random_.randrange(OBJECTS), random_.getrandbits(16) << 16 | random_.randrange(OBJECTS))))
            struct.pack_into("<h", thing, 0x1FC, random_.choice((-1, 0, 1, 2, 3, 3)))
            headers += thing
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        tag_pointers = []
        for index in range(TAGS):
            struct.pack_into("<I", tags, index * 0x10 + 8, len(tags))
            tag_pointers.append(index * 0x10 + 8)
            definition = bytearray(random_.randbytes(0x1F4))
            struct.pack_into("<I", definition, 0x1CC, len(tags) + 0x1F4)
            tag_pointers.append(len(tags) + 0x1CC)
            struct.pack_into("<h", definition, 0x1F0, random_.choice((3, 3, 5, 5, 0, 4, 6, -1, 0x103)))
            tags += definition
            for seat in range(4):
                piece = bytearray(random_.randbytes(0xB0))
                struct.pack_into("<I", piece, 0, random_.choice((0, 4, 4, 0xFFFFFFFB, random_.getrandbits(32))))
                tags += piece
        t.check(
            "input_abstraction_controller_unknown_00218510",
            random_.choice((NONE, 0, 1, 2, 3, 0, 1, 2, 3)),
            globals={
                "players_globals": Buffer(bytes(players)),
                "player_data": Buffer(bytes(player_data), pointers=(0x44,)),
                "object_header_data": Buffer(bytes(headers), pointers=tuple(pointers)),
                "global_tag_instances": Buffer(bytes(tags), pointers=tuple(tag_pointers)),
            },
        )
