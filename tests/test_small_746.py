# six small functions of a table of players kept in two lists

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
PLAYERS, PLAYER = 350, 0x68


def an_index(random_: random.Random) -> int:
    return random_.choice((random_.randrange(PLAYERS), random_.randrange(PLAYERS), random_.randrange(4), NONE & 0xFFFF))


def table(random_: random.Random, few: list[int]) -> bytes:
    data = bytearray(random_.randbytes(PLAYERS * PLAYER))
    for index in range(PLAYERS):
        for offset in (0x58, 0x5A, 0x5C, 0x5E):
            value = random_.choice(few + [0xFFFF, 0xFFFF])
            struct.pack_into("<H", data, index * PLAYER + offset, value)
    return bytes(data)


def lists(random_: random.Random, index: int, few: list[int]) -> dict:
    def end() -> int:
        return random_.choice((index, NONE, NONE, *few))

    return {
        "unknown_004cf98c": table(random_, few),
        "unknown_004cf97c": end(),
        "unknown_004cf980": end(),
        "unknown_004cf984": end(),
        "unknown_004cf988": end(),
    }


def run(t) -> None:
    random_ = random.Random(746)

    for n in range(300):
        tick = random_.getrandbits(32)
        t.check(
            "network_recent_players_unknown_0007f260",
            globals={
                "unknown_004cf8e0": bytes((random_.choice((0, 1, 1, 0x80)),)),
                "unknown_004cf8e4": random_.getrandbits(32),
                "unknown_004cf95d": bytes((random_.choice((0, 1)),)),
                "unknown_004cf960": random_.getrandbits(32),
                "unknown_00510548": bytes((random_.choice((0, 0, 1, 0x80)),)),
                "unknown_0051054c": random_.getrandbits(32),
            },
            externals={
                "network_observer_reset_unknown4e08": lambda harness, arguments: (0, tuple(arguments)),
                "GetTickCount": lambda harness, arguments, tick=tick: tick,
            },
        )

        unknown = bytearray(random_.randbytes(0x80))
        unknown[0x7F] = random_.choice((0xFF, 0xFF, 0, 1, 0x80))
        t.check(
            "network_recent_player_make",
            Buffer(bytes(unknown)),
            Buffer(random_.randbytes(0x58)),
            Buffer(random_.randbytes(0xC)),
            random_.choice((Buffer(random_.randbytes(0x58)), Buffer(random_.randbytes(0x58)), 0)),
        )

        for name in (
            "network_recent_player_remove_first",
            "network_recent_player_remove_second",
            "network_recent_player_move_to_first",
            "network_recent_player_move_to_second",
        ):
            index = random_.randrange(PLAYERS)
            few = [random_.randrange(PLAYERS) for count in range(4)] + [index]
            t.check(name, index, globals=lists(random_, index, few))
