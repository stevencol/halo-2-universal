# source/main/levels.c: what the globals tag says of a level; and a function of ai/ai.c

import random
import struct

from difftest import Buffer, Mock, mock_address

GLOBALS = 0x180
LEVELS = 0x18


def run(t) -> None:
    random_ = random.Random(618)

    for n in range(1500):
        # the globals tag, what it says of the levels, and the two blocks of that
        campaign = random_.choice((0, 1, 3))
        multiplayer = random_.choice((0, 1, 3))
        levels_at = GLOBALS
        campaign_at = levels_at + LEVELS
        multiplayer_at = campaign_at + campaign * 0xB50
        data = bytearray(random_.randbytes(multiplayer_at + multiplayer * 0xC64))
        has = random_.choice((0, 1, 1, 1, 1))
        struct.pack_into("<2I", data, 0x178, has, levels_at)
        struct.pack_into("<2I", data, levels_at + 8, campaign, campaign_at)
        struct.pack_into("<2I", data, levels_at + 0x10, multiplayer, multiplayer_at)
        numbers = [random_.choice((1, 3, 5, 7)) for _ in range(6)]
        for index in range(campaign):
            struct.pack_into("<i", data, campaign_at + index * 0xB50 + 4, numbers[index])
        for index in range(multiplayer):
            struct.pack_into("<i", data, multiplayer_at + index * 0xC64, numbers[3 + index])
        tag = lambda: {"global_game_globals": Buffer(bytes(data), pointers=(0x17C, levels_at + 0xC, levels_at + 0x14))}
        number = random_.choice((1, 3, 5, 7, 9))
        t.check("levels_get_campaign_level", number, globals=tag())
        t.check("levels_get_multiplayer_level", number, globals=tag())

    for n in range(200):
        t.check("ai_unknown_001c7fe0", globals={"unknown_004f55cc": Buffer(random_.randbytes(0x48))})


def run_619(t) -> None:
    random_ = random.Random(619)

    def a_table():
        # a table of data, and after it what its memory came from, with its two functions; or one that nothing gave
        if random_.choice((False, True, True, True)) is False:
            return 0
        data = bytearray(random_.randbytes(0x4C + 12))
        given = random_.choice((False, True, True))
        struct.pack_into("<I", data, 0x30, 0x4C if given else 0)
        struct.pack_into("<3I", data, 0x4C, 0x50, mock_address(0), mock_address(1))
        return Buffer(bytes(data), pointers=(0x30, 0x4C) if given else (0x4C,))

    for n in range(400):
        t.check(
            "levels_dispose",
            globals={"multiplayer_level_data": a_table(), "campaign_level_data": a_table()},
            mocks=[
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
            ],
        )


_run_618 = run


def run(t) -> None:
    _run_618(t)
    run_619(t)
