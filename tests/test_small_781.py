# five small functions: something of 0x60 bytes of the effects, the sixteen
# pairs of an effect, and the greater of two reals of the sound of a gamepad

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
EFFECTS, EFFECT = 4, 0x190
PAIRS = 16


def run(t) -> None:
    random_ = random.Random(781)

    def pairs(data: bytearray, at: int) -> None:
        free = random_.choice((0, 0, 1, 2, 16))
        for index in range(PAIRS):
            first = random_.choice((0, 0, 5, random_.getrandbits(32)))
            second = random_.choice((NONE, NONE, 0, 3))
            struct.pack_into("<II", data, at + index * 8, first, second)
        for index in random_.sample(range(PAIRS), free):
            struct.pack_into("<II", data, at + index * 8, 0, NONE)

    for n in range(600):
        t.check("effect_unknown_00175ee0_new", Buffer(random_.randbytes(0x64)))

        effect = bytearray(random_.randbytes(EFFECT))
        pairs(effect, 0x110)
        t.check("effect_clear_pairs", Buffer(bytes(effect)))

        # the effects: a data array of four
        table = bytearray(random_.randbytes(0x4C + EFFECTS * EFFECT))
        struct.pack_into("<I", table, 0x44, 0x4C)
        for index in range(EFFECTS):
            pairs(table, 0x4C + index * EFFECT + 0x110)
        globals_ = {"effect_data": Buffer(bytes(table), pointers=(0x44,))}
        effect_index = random_.getrandbits(16) << 16 | random_.randrange(EFFECTS)
        t.check("effect_has_free_pair", effect_index, globals=globals_)
        t.check("effect_add_pair", effect_index, random_.getrandbits(32), random_.getrandbits(32), globals=globals_)

        sound = bytearray(random_.randbytes(0x264))
        for gamepad in range(4):
            for offset in (0x74, 0x78):
                struct.pack_into("<f", sound, gamepad * 0x94 + offset, random_.choice((0.0, -0.0, 1.0, 2.0, -3.0, float("nan"), random_.uniform(-5, 5))))
        t.check("sound_unknown_00187420", random_.randrange(4), globals={"unknown_004ed284": Buffer(bytes(sound))})
