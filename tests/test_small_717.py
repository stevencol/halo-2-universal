# six small functions that go from a thing to its tag

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
TAGS, TAG_SIZE = 4, 0x40


def tags(random_) -> Buffer:
    """Some tags after their table; each names another at 4, and has things at 0x18 that are after them all."""
    data = bytearray(random_.randbytes(TAGS * 16 + TAGS * TAG_SIZE + 0x100))
    pointers = []
    for index in range(TAGS):
        at = TAGS * 16 + index * TAG_SIZE
        struct.pack_into("<I", data, index * 16 + 8, at)
        pointers.append(index * 16 + 8)
        struct.pack_into("<I", data, at, random_.choice((0, 4, random_.getrandbits(32))))
        struct.pack_into("<I", data, at + 4, random_.getrandbits(16) << 16 | random_.randrange(TAGS))
        struct.pack_into("<I", data, at + 0x18, TAGS * 16 + TAGS * TAG_SIZE)
        pointers.append(at + 0x18)
    return Buffer(bytes(data), pointers=tuple(pointers))


def run(t) -> None:
    random_ = random.Random(717)

    for n in range(200):
        tag_index = random_.getrandbits(16) << 16 | random_.randrange(TAGS)
        t.check(
            "effect_definition_get_random_seed",
            tag_index,
            globals={"global_tag_instances": tags(random_), "global_random_seed_reference": Buffer(random_.randbytes(8))},
        )
        t.check("tag_get_unknown4_tag", tag_index, globals={"global_tag_instances": tags(random_)})

        def thing() -> Buffer:
            return Buffer(random_.randbytes(0x68) + struct.pack("<I", tag_index) + random_.randbytes(4))

        t.check("unknown68_get_tag", thing(), globals={"global_tag_instances": tags(random_)})
        t.check("unknown68_get_tag_unknownC", thing(), globals={"global_tag_instances": tags(random_)})
        t.check("unknown68_get_tag_unknownC_or_zero", thing(), globals={"global_tag_instances": tags(random_)})
        t.check(
            "unknown68_get_tag_unknown18",
            random_.choice((NONE, 0, 1, 5)),
            thing(),
            globals={"global_tag_instances": tags(random_)},
        )
