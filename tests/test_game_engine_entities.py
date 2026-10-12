# the two entities of the simulation that the game engine keeps

import random
import struct

from difftest import Buffer

KINDS = 8
NONE = 0xFFFFFFFF
NOTHING = lambda harness, arguments: 0


def run(t) -> None:
    random_ = random.Random(707)

    for n in range(300):
        entities = [NONE, random_.getrandbits(32), random_.getrandbits(32)]
        globals_ = bytearray(random_.randbytes(0xC9C))
        kind = random_.randrange(KINDS)
        struct.pack_into("<I", globals_, 0xC14, kind)
        struct.pack_into("<2I", globals_, 0x24, random_.choice(entities), random_.choice(entities))
        kinds = bytearray(KINDS * 4)
        if random_.random() < 0.7:
            struct.pack_into("<I", kinds, kind * 4, random_.getrandbits(32) | 1)

        def options() -> dict:
            return {
                "globals": {"unknown_004e9ae8": Buffer(bytes(globals_)), "unknown_0055e4d0": bytes(kinds)},
                "externals": {"unknown_001e98e0": NOTHING, "unknown_000b58c0": NOTHING},
            }

        def one() -> Buffer:
            other = random_.choice(entities)
            if random_.random() < 0.3:
                other = (struct.unpack_from("<I", globals_, 0x28)[0] & 0x3FF) | (random_.getrandbits(22) << 10)
            return Buffer(struct.pack("<I", other) + random_.randbytes(4))

        t.check("game_engine_entity28_set", one(), *(random_.getrandbits(32) for _ in range(5)), **options())
        t.check("game_engine_entity28_clear", one(), **options())
        t.check("game_engine_entity28_update", one(), **options())
        t.check("game_engine_unknown_0009bb70", *(random_.getrandbits(32) for _ in range(4)), **options())
        t.check("game_engine_entity24_clear", one(), **options())
        t.check("game_engine_unknown_000a7810", random_.getrandbits(32), **options())
