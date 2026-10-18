# ten small functions of the network game and of the voice

import random
import struct

from difftest import Buffer, GlobalData

NONE = 0xFFFFFFFF
PROFILES = 4
PROFILE = 0xD0

GETTERS = (
    "network_life_cycle_session_get_unknown49A4",
    "network_life_cycle_session_get_unknown498C",
    "network_life_cycle_session_get_unknown49A1",
    "network_life_cycle_session_get_unknown49AC",
    "network_life_cycle_session_get_unknown49B0",
    "network_life_cycle_session_get_unknown18",
)


def run(t) -> None:
    random_ = random.Random(740)

    for n in range(300):
        for name in GETTERS:
            session = bytearray(random_.randbytes(0x7420))
            struct.pack_into("<I", session, 0x741C, random_.choice((0, 1, 2, 3, 4, 8, 9, NONE, 0x80000000)))
            session[0x49A8] = random_.choice((0, 1, 1, 0xFF))
            t.check(
                name,
                globals={
                    "unknown_00527330": bytes((random_.choice((0, 1, 1, 1)),)),
                    "network_life_cycle_session": Buffer(bytes(session)),
                },
            )

        t.check(
            "network_life_cycle_unknown_004cd8ca_set",
            random_.getrandbits(32),
            Buffer(random_.randbytes(0x20)),
            globals={
                "unknown_004cd8ca": bytes((random_.choice((0, 1, 0xFF)),)),
                "unknown_004cd8cc": random_.randbytes(0x20),
                "unknown_004cd8ec": random_.getrandbits(32),
                "unknown_004cd8f0": random_.getrandbits(32),
                "unknown_004cd8f4": random_.getrandbits(32),
                "unknown_004cf968": random_.getrandbits(32),
                "unknown_004cf96c": random_.getrandbits(32),
            },
        )

        t.check(
            "network_life_cycle_unknown_004cd8ca_get",
            Buffer(random_.randbytes(0x20)),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
            globals={
                "unknown_004cd8ca": bytes((random_.choice((0, 1, 1, 0xFF)),)),
                "unknown_004cd8cc": random_.randbytes(0x20),
                "unknown_004cd8ec": random_.getrandbits(32),
                "unknown_004cd8f0": random_.getrandbits(32),
                "unknown_004cd8f4": random_.getrandbits(32),
            },
        )

        t.check(
            "game_voice_unknown_00063dc0",
            random_.randrange(3),
            globals={
                "unknown_004cd904": GlobalData(random_.randbytes(PROFILES * PROFILE)),
                "unknown_004cdc50": random_.randbytes(12),
            },
        )

        profiles = bytearray(random_.randbytes(PROFILES * PROFILE))
        for index in range(PROFILES):
            profiles[index * PROFILE] = random_.choice((0, 0, 1, 0xFF))
        t.check(
            "game_voice_unknown_004cd904_get_identifier",
            random_.randrange(PROFILES),
            Buffer(random_.randbytes(12)),
            globals={"unknown_004cd904": GlobalData(bytes(profiles))},
        )
