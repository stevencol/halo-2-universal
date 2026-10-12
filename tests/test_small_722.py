# twelve more small functions of actors

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
ACTORS, ACTOR = 3, 0x888
PROPS, PROP = 3, 0x3C


def run(t) -> None:
    random_ = random.Random(722)

    for n in range(200):
        def actors() -> Buffer:
            data = bytearray(random_.randbytes(0x4C + ACTORS * ACTOR))
            struct.pack_into("<I", data, 0x44, 0x4C)
            for index in range(ACTORS):
                at = 0x4C + index * ACTOR
                data[at + 0x22A] = random_.choice((0, 1, 1, 0xFF))
                data[at + 0x223] = random_.choice((0, 0, 1))
                for offset in (0x338, 0x344, 0x26C):
                    named = random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(PROPS)))
                    struct.pack_into("<I", data, at + offset, named)
                struct.pack_into("<h", data, at + 0x31C, random_.choice((-1, 0, 5)))
                struct.pack_into("<h", data, at + 0x504, random_.choice((2, 2, 0, 3)))
            return Buffer(bytes(data), pointers=(0x44,))

        def props() -> Buffer:
            data = bytearray(random_.randbytes(0x4C + PROPS * PROP))
            struct.pack_into("<I", data, 0x44, 0x4C)
            for index in range(PROPS):
                at = 0x4C + index * PROP
                struct.pack_into("<h", data, at + 0x18, random_.choice((1, 1, 0, 2)))
                data[at + 0x26] = random_.choice((0, 1, 2, 3, 0x7F, 0x80, 0xFF))
            return Buffer(bytes(data), pointers=(0x44,))

        actor_index = random_.getrandbits(16) << 16 | random_.randrange(ACTORS)
        answers = lambda: {"unknown_0046fbe4": random_.getrandbits(32), "unknown_0046fbe8": random_.getrandbits(32)}

        for name in ("actor_unknown_001aeb60", "actor_unknown_001bca60", "actor_unknown_001c10a0"):
            t.check(name, actor_index, globals={"actor_data": actors(), "prop_ref_data": props()})
        for name, arguments in (
            ("actor_unknown_001b3540", 2),
            ("actor_unknown_001bce80", 2),
            ("actor_unknown_001b8370", 3),
            ("actor_unknown_001bcc50", 2),
        ):
            t.check(
                name,
                actor_index,
                *(random_.getrandbits(32) for _ in range(arguments - 1)),
                globals={"actor_data": actors()},
            )
        t.check(
            "actor_unknown_001bd1e0",
            actor_index,
            random_.getrandbits(32),
            globals={"actor_data": actors(), "global_forward3d": Buffer(random_.randbytes(12))},
        )

        for name in ("actor_unknown_001b56b0", "actor_unknown_001bc580", "actor_unknown_001b1f30"):
            thing = bytearray(random_.randbytes(0x18))
            for offset in (0xC, 0x10):
                struct.pack_into("<I", thing, offset, random_.choice((NONE, random_.getrandbits(32))))
            thing[0x12] = random_.choice((0, 1, 0xFF, thing[0x12]))
            t.check(
                name,
                actor_index,
                Buffer(bytes(thing)),
                random_.getrandbits(32),
                globals={"actor_data": actors(), **answers()},
            )

        t.check(
            "actor_set_unknown6D0",
            actor_index,
            random_.getrandbits(24) << 8 | random_.choice((0, 0, 1, 0xFF)),
            globals={"actor_data": actors()},
        )
