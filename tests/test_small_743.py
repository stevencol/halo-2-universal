# four small functions of the life cycle of the network game

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
MANAGER = 0x60
SESSION_SIZE = 0x78B8
STATE = 0x741C
UNKNOWN48 = 0x48
UNKNOWN7420 = 0x7420
UNKNOWN4980 = 0x4980
PEER_COUNT = 0x54


def with_session(random_: random.Random, size: int, at: int, state: int) -> tuple[bytearray, int, tuple]:
    """Something of `size` bytes, what it has at 8 after it, and a session
    that that has at `at` after that."""
    data = bytearray(random_.randbytes(size + MANAGER + SESSION_SIZE))
    session = size + MANAGER
    struct.pack_into("<I", data, 8, size)
    struct.pack_into("<I", data, size + at, session)
    struct.pack_into("<I", data, session + STATE, state)
    struct.pack_into("<i", data, session + PEER_COUNT, random_.randrange(17))
    return data, session, (8, size + at)


def time_options(random_: random.Random) -> dict:
    tick = random_.getrandbits(32)
    seconds = random_.getrandbits(31)
    return {
        "globals": {
            "unknown_00510548": bytes((random_.choice((0, 0, 1, 0x80)),)),
            "unknown_0051054c": random_.getrandbits(32),
        },
        "externals": {
            "GetTickCount": lambda harness, arguments: tick,
            "time": lambda harness, arguments: seconds,
        },
    }


def run(t) -> None:
    random_ = random.Random(743)

    for n in range(300):
        for name, size in (("network_life_cycle_unknown_0006e310", 0x20), ("network_life_cycle_unknown_0006fe20", 0x108)):
            state = random_.choice((0, 1, 2, 3, 4, 5, 6, 7, 7, 8, 8, 9, 10))
            data, session, pointers = with_session(random_, size, 0x38, state)
            data[0xC] = random_.choice((0, 0, 0, 1, 0x80))
            # the states in which leaving does more than set a byte: the
            # byte of 0x48 says not to
            data[session + UNKNOWN48] = 1 if state in (1, 3, 5, 9, 10) else random_.choice((0, 0, 1))
            data[session + UNKNOWN7420] = random_.choice((0, 0, 1))
            t.check(
                name,
                Buffer(bytes(data), pointers=pointers),
                random_.getrandbits(32),
                random_.getrandbits(32),
                random_.getrandbits(32),
            )

        data, session, pointers = with_session(random_, 0xA90, 0x30, random_.choice((0, 3, 5)))
        struct.pack_into("<I", data, session + UNKNOWN4980, random_.choice((7, 7, 0, 2, 5, NONE)))
        t.check("network_life_cycle_unknown_00070c00", Buffer(bytes(data), pointers=pointers), **time_options(random_))

        state = random_.choice((0, 1, 2, 3, 4, 5, 6, 7, 7, 8, 8, 9, 10, NONE))
        data, session, pointers = with_session(random_, 0x980, 0x38, state)
        data[session + UNKNOWN7420] = random_.choice((0, 0, 1))
        struct.pack_into("<I", data, session + UNKNOWN4980, random_.choice((0, 1, 2, 3, 4, 5, 6, 7, NONE)))
        t.check("network_life_cycle_unknown_00071f80", Buffer(bytes(data), pointers=pointers))
