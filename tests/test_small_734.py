# eighteen small functions: bits, tags that take from another, and lists of pairs

import math
import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
TAGS, TAG = 5, 0xD0
OFFSETS = (0x3C, 0x44, 0x54, 0x5C, 0xB4, 0xBC, 0xC4)


def as_bits(value: float) -> int:
    return struct.unpack("<I", struct.pack("<f", value))[0]


def run(t) -> None:
    random_ = random.Random(734)

    for n in range(300):
        # the tags: each takes from one with a bigger number, or from none
        salts = [random_.getrandbits(16) for _ in range(TAGS)]
        tags = bytearray(random_.randbytes(TAGS * 16 + TAGS * TAG))
        pointers = []
        for index in range(TAGS):
            at = TAGS * 16 + index * TAG
            struct.pack_into("<I", tags, index * 16 + 8, at)
            pointers.append(index * 16 + 8)
            above = NONE if index + 1 >= TAGS or random_.random() < 0.3 else salts[index + 1] << 16 | random_.randrange(index + 1, TAGS)
            struct.pack_into("<I", tags, at + 8, above)
            for offset in OFFSETS:
                struct.pack_into("<i", tags, at + offset, random_.choice((0, 0, 0, 1, 3, -1)))
        for offset in OFFSETS:
            t.check(
                "tag_get_inherited_unknown%X" % offset,
                random_.choice((NONE, salts[0] << 16, salts[0] << 16, random_.getrandbits(16) << 16 | random_.randrange(TAGS))),
                globals={"global_tag_instances": Buffer(bytes(tags), pointers=tuple(pointers))},
            )

        # something with pairs of shorts after it, and another block
        count = random_.choice((0, 1, 4, 8))
        pairs = b"".join(struct.pack("<2h", random_.choice((0, 1, 2, 3, -1)), random_.choice((-5, 0, 3, 9, 100))) for _ in range(8))
        thing = bytearray(random_.randbytes(0x60))
        struct.pack_into("<2I", thing, 0x4C, count, 0x60)
        struct.pack_into("<2I", thing, 0x54, random_.choice((0, 1, 2, 0xFFFFFFFF)), 0x60 + 12)
        a_thing = lambda: Buffer(bytes(thing) + pairs, pointers=(0x50, 0x58))
        key = lambda: random_.choice((0, 1, 2, 3, NONE, 0x10001))
        t.check("unknown4C_find_pair", a_thing(), key())
        t.check("unknown4C_find_pair_above", a_thing(), key(), random_.choice((0xFFFFFFF0, 0, 3, 8, 50, 200)))
        t.check("unknown4C_find_pair0", a_thing())
        t.check("unknown4C_find_pair1", a_thing())
        t.check("unknown54_get_first", a_thing())

        bit = random_.getrandbits(24) << 8 | random_.randrange(32)
        value = random_.getrandbits(24) << 8 | random_.choice((0, 1, 0xFF))
        t.check("flags_set_bit", Buffer(random_.randbytes(4)), bit, value)
        t.check("word_flags_set_bit", Buffer(random_.randbytes(4)), random_.getrandbits(24) << 8 | random_.randrange(16), value)
        vector = lambda: Buffer(random_.choice((bytes(32), random_.randbytes(32), bytes(28) + random_.randbytes(4), random_.randbytes(4) + bytes(28))))
        t.check("bit_vector232_not", vector())
        t.check("bit_vector256_clear_from", random_.choice((1, 31, 32, 33, 100, 224, 255, 256)), vector())
        t.check("bit_vector256_is_empty", vector())

        turn = lambda: {
            "unknown_00547634": struct.pack("<f", 2.0 * math.pi),
            "unknown_00547638": struct.pack("<f", math.pi),
        }
        angle = random_.choice((0.0, math.pi, -math.pi, random_.uniform(-7.0, 7.0), float("nan")))
        other = random_.choice((0.0, math.pi, -math.pi, random_.uniform(-7.0, 7.0)))
        t.check("angle_difference", as_bits(angle), as_bits(other), globals=turn())
