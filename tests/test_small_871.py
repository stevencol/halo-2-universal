# two functions of the way to the main menu (guided by halo2-decompiled): the
# variant of the model of a unit by a name, and the rectangle of a cell of a
# grid

import random
import struct

from difftest import Buffer

from test_small_859 import BIG, big_objects
from test_units import COUNT

NONE = 0xFFFFFFFF
VARIANTS = 5
NAMES = (1, 2, 3)


def run(t) -> None:
    random_ = random.Random(871)

    for n in range(400):
        units, pointers = big_objects(random_)
        index = random_.randrange(COUNT)
        at = 0x4C + COUNT * 12 + index * BIG
        struct.pack_into("<I", units, at, random_.randrange(2) | random_.getrandbits(16) << 16)
        struct.pack_into("<h", units, at + 0x342, 0x300)
        # the tags: two definitions, two models with their variants, and two tags they name
        model_size = 0xA4 + VARIANTS * 0x38
        places = [0x60, 0xA0, 0xE0, 0xE0 + model_size, 0xE0 + 2 * model_size, 0xE0 + 2 * model_size + 0x20]
        tags = bytearray(random_.randbytes(places[-1] + 0x20))
        tag_pointers = []
        for tag, place in enumerate(places):
            struct.pack_into("<I", tags, tag * 16 + 8, place)
            tag_pointers.append(tag * 16 + 8)
        for definition in range(2):
            struct.pack_into("<I", tags, places[definition] + 0x38, random_.choice((2, 3)))
        for model in (2, 3):
            base = places[model]
            struct.pack_into("<iI", tags, base + 0x50, random_.choice((0, 1, 3, VARIANTS)), base + 0xA4)
            tag_pointers.append(base + 0x54)
            struct.pack_into("<I", tags, base + 0x90, random_.choice((NONE, 4, 5)))
            for variant in range(VARIANTS):
                at_variant = base + 0xA4 + variant * 0x38
                struct.pack_into("<I", tags, at_variant, random_.choice(NAMES))
                struct.pack_into("<I", tags, at_variant + 0x2C, random_.choice((0, NONE, 7, 9)))
                struct.pack_into("<I", tags, at_variant + 0x34, random_.choice((NONE, 4, 5)))
        t.check(
            "unit_unknown_00114ec0",
            random_.getrandbits(16) << 16 | index,
            random_.choice(NAMES + (4,)),
            globals={
                "object_header_data": Buffer(bytes(units), pointers=tuple(pointers)),
                "global_tag_instances": Buffer(bytes(tags), pointers=tuple(tag_pointers)),
            },
        )

        clips = {}
        for offset in range(0x8A, 0x9A, 2):
            clips[f"rasterizer_clip_unknown_{0x485A00 + offset:08x}"] = struct.pack("<h", random_.randrange(-50, 700))
        grid = (random_.choice((1, 2, 3, 4, 8, -2)), random_.choice((1, 2, 3, 5, -3)))
        place = (random_.randrange(0, 5), random_.randrange(0, 5))
        span = (random_.randrange(0, 5), random_.randrange(0, 5))
        t.check(
            "rasterizer_unknown_0002f6b0",
            Buffer(struct.pack("<2h", *place)),
            Buffer(struct.pack("<2h", *grid)),
            Buffer(struct.pack("<2h", *span)),
            Buffer(random_.randbytes(8)),
            Buffer(random_.randbytes(8)),
            globals=clips,
        )
