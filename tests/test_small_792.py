# seven small functions: whether frames are waited for, the bits and the
# memory of the things of game_unknown_00131ff0.c, seven flags, an entry of
# game_unknown_00134d90.c by a number of the scenario and a count of a tag

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
THING = 0x2AE0


def run(t) -> None:
    random_ = random.Random(792)

    def word(value: int) -> int:
        return value & 0xFFFFFFFF

    for n in range(400):
        settings = bytearray(random_.randbytes(12))
        struct.pack_into("<h", settings, 2, random_.choice((0, 0, 1, 2)))
        game = bytearray(random_.randbytes(0x1200))
        game[0x1120] = random_.choice((0, 1))
        time = bytearray(random_.randbytes(0x98))
        time[0] = random_.choice((0, 1))
        time[1] = random_.choice((0, 1, 0x80))
        t.check(
            "main_unknown_0012b3c0",
            globals={
                "display_settings": bytes(settings),
                "main_game_globals": Buffer(bytes(game)) if n % 6 else 0,
                "main_time_globals": bytes(time),
            },
        )

        t.check("game_unknown_00131ff0_test_bit", random_.randrange(512), Buffer(random_.randbytes(0x260)))

        t.check("game_unknown_00131ff0_flags", *(random_.choice((0, 1, 0x100, 0x80, random_.getrandbits(32))) for _ in range(7)))

        thing = bytearray(random_.randbytes(THING))
        struct.pack_into("<H", thing, 0xA5C, random_.choice((0, 1, 0x100, 0x1FE, 0x1FF, 0x200, random_.randrange(0x200))))
        t.check(
            "game_unknown_00131ff0_allocate_bits",
            word(random_.choice((-1, 0, 1, 31, 32, 33, 64, 1000, random_.randrange(0x4000)))),
            Buffer(bytes(thing)),
        )

        thing = bytearray(random_.randbytes(THING + 4 * 8))
        struct.pack_into("<H", thing, 0xA5C, random_.choice((0, 1, 3, 0x200)))
        for index in range(4):
            struct.pack_into("<I", thing, 0x0C + index * 4, THING + index * 8)
        t.check("game_unknown_00131ff0_clear", Buffer(bytes(thing), pointers=(0x0C, 0x10, 0x14, 0x18)))

        # the scenario's entries of 0x18 bytes at 0x3C0, and the 0x204 bytes
        count = random_.choice((0, 1, 4, 16))
        numbers = [random_.choice((0, 5, 7, 9)) for _ in range(16)]
        scenario = bytearray(random_.randbytes(0x3C8 + 16 * 0x18))
        struct.pack_into("<iI", scenario, 0x3C0, count, 0x3C8)
        for index in range(16):
            struct.pack_into("<i", scenario, 0x3C8 + index * 0x18, numbers[index])
        t.check(
            "game_unknown_00134d90_find",
            random_.choice((0, Buffer(random_.randbytes(4)))),
            random_.choice((0, 5, 7, 9, 11)),
            globals={
                "unknown_004e6740": Buffer(random_.randbytes(0x204)) if n % 7 else 0,
                "global_scenario": Buffer(bytes(scenario), pointers=(0x3C4,)) if n % 9 else 0,
            },
        )

        # a tag with elements of 0x0C at 0x38 and of 0x5C at 0x28
        tag = random_.randrange(4)
        tags = bytearray(random_.randbytes(0x40 + 0x50 + 4 * 0x0C + 4 * 0x5C))
        struct.pack_into("<I", tags, tag * 0x10 + 8, 0x40)
        at = 0x40
        tags[at + 4] = random_.choice((0, 4, 0xFB))
        struct.pack_into("<i", tags, at + 0x1C, random_.choice((0, 1, 3, 4)))
        struct.pack_into("<I", tags, at + 0x28, 0x90 + 4 * 0x0C)
        struct.pack_into("<I", tags, at + 0x38, 0x90)
        for which in range(4):
            tags[at + 0x3C + which] = random_.randrange(4)
        elements = bytes(random_.choice((0, 1, 3, 0xFF)) for _ in range(4))
        t.check(
            "tag_unknown_00137650",
            random_.getrandbits(16) << 16 | tag,
            random_.randrange(4),
            Buffer(elements),
            globals={"global_tag_instances": Buffer(bytes(tags), pointers=(tag * 0x10 + 8, at + 0x28, at + 0x38))},
        )
