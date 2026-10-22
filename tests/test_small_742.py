# six small functions of the network session and its life cycle

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
SOMETHING = 0x108
MANAGER = 0x40
SESSION_STATE = 0x741C


def time_options(random_: random.Random) -> dict:
    """The time that the network keeps, or not, and the console's."""
    tick = random_.getrandbits(32)
    return {
        "globals": {
            "unknown_00510548": bytes((random_.choice((0, 0, 1, 0x80)),)),
            "unknown_0051054c": random_.getrandbits(32),
        },
        "externals": {"GetTickCount": lambda harness, arguments: tick},
    }


def run(t) -> None:
    random_ = random.Random(742)

    for n in range(300):
        t.check(
            "network_session_unknown_0006de50",
            Buffer(random_.randbytes(0x50)),
            *(random_.getrandbits(32) for index in range(7)),
        )

        session = bytearray(random_.randbytes(0x4988))
        struct.pack_into("<I", session, 0x4984, random_.choice((0, NONE, 0x80000000, random_.getrandbits(32))))
        t.check("network_session_time_since_start", Buffer(bytes(session)), **time_options(random_))

        t.check(
            "network_life_cycle_unknown_0006f090",
            Buffer(random_.randbytes(SOMETHING)),
            Buffer(random_.randbytes(0x20)),
        )

        # something, what it has at 8 (at 0x108) and its session (at 0x148)
        data = bytearray(random_.randbytes(SOMETHING + MANAGER + SESSION_STATE + 4))
        struct.pack_into("<I", data, 8, SOMETHING)
        struct.pack_into("<I", data, SOMETHING + 0x30, SOMETHING + MANAGER)
        struct.pack_into("<I", data, SOMETHING, random_.choice((0, 1, 4, 9, 2, 3, 5, 8, 10, NONE)))
        struct.pack_into("<I", data, 0x104, random_.choice((0, 0, 0, 0xE, 0xF, 0x10, random_.getrandbits(32))))
        session_at = SOMETHING + MANAGER
        struct.pack_into(
            "<I", data, session_at + SESSION_STATE, random_.choice((0, 1, 2, 3, 4, 5, 8, 9, 10, NONE, 0x80000000))
        )
        if random_.random() < 0.5:
            data[session_at + 0x72D8 : session_at + 0x72DC] = data[session_at + 0x50 : session_at + 0x54]
        t.check("network_life_cycle_unknown_0006f1a0", Buffer(bytes(data), pointers=(8, SOMETHING + 0x30)))

        for name in ("network_life_cycle_unknown_00070c50", "network_life_cycle_unknown_00070d20"):
            t.check(
                name,
                random_.choice((0, 1, 0x100, 0x101, 0xFF, random_.getrandbits(32))),
                Buffer(random_.randbytes(0xA98)),
                **time_options(random_),
            )
