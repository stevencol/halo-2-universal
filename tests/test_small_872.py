# two functions of the way to the main menu (guided by halo2-decompiled): the
# events of the animation of a channel, and the sections of the groups of a
# tag for the variants wanted

import random
import struct

from difftest import Buffer

from test_object_types_more import after

NONE = 0xFFFFFFFF
GROUPS = 5
VARIANTS = 3


def run(t) -> None:
    random_ = random.Random(872)

    for n in range(400):
        # the animation is the test's: frames, and events from 0x60
        channel = bytearray(random_.randbytes(0x20))
        struct.pack_into("<IHH", channel, 0, random_.randrange(2), random_.getrandbits(16), random_.choice((0xFFFF, 0, 1)))
        frames = random_.choice((0, 5, 10, 30))
        struct.pack_into(
            "<2f",
            channel,
            0x18,
            random_.choice((0.0, 1.0, -1.0, 0.5)),
            random_.choice((0.0, 3.0, 5.0, float(frames), float(frames + 1), 2.5, 40.0, -1.0)),
        )
        events = random_.choice((0, 1, 4, 5, 9))
        animation = bytearray(random_.randbytes(0x60 + 9 * 4))
        struct.pack_into("<h", animation, 0x14, frames)
        struct.pack_into("<iI", animation, 0x4C, events, 0x60)
        for event in range(9):
            struct.pack_into("<Bxh", animation, 0x60 + event * 4, random_.choice((0, 1, 5, 15, 16, 33)), random_.choice((0, 2, 3, 5, frames, frames + 1, 7)))
        tags = Buffer(random_.randbytes(2 * 16))
        channel = Buffer(bytes(channel))
        place = after(channel, tags)
        t.check(
            "havok_unknown_001c69b0",
            channel,
            globals={"global_tag_instances": tags},
            extra=[Buffer(bytes(animation), pointers=(0x50,))],
            externals={"havok_unknown_001daea0": lambda harness, arguments, place=place: (place, tuple(arguments))},
        )

        # a tag of groups with variants; whether each geometry is loaded is the test's
        count = random_.choice((0, 1, 3, GROUPS))
        # the definition is at 0x10 of the tags' buffer, and its pointers are offsets in that buffer
        groups = 0x30
        variants = groups + GROUPS * 0x10
        definition = bytearray(random_.randbytes(variants + GROUPS * VARIANTS * 0x10 + 8 * 0x5C))
        sections = variants + GROUPS * VARIANTS * 0x10
        struct.pack_into("<iI", definition, 0x1C, count, 0x10 + groups)
        struct.pack_into("<I", definition, 0x28, 0x10 + sections)
        pointers = [0x20, 0x28]
        for group in range(GROUPS):
            struct.pack_into("<iI", definition, groups + group * 0x10 + 0x08, random_.choice((0, 1, 2, VARIANTS)), 0x10 + variants + group * VARIANTS * 0x10)
            pointers.append(groups + group * 0x10 + 0x0C)
            for variant in range(VARIANTS):
                struct.pack_into("<6h", definition, variants + (group * VARIANTS + variant) * 0x10 + 0x04, *(random_.randrange(8) for _ in range(6)))
        tags = bytearray(random_.randbytes(0x10)) + definition
        struct.pack_into("<I", tags, 8, 0x10)
        tags = Buffer(bytes(tags), pointers=(8,) + tuple(0x10 + pointer for pointer in pointers))
        wanted = bytes(random_.choice((0xFF, 0, 1, 2, 3)) for _ in range(GROUPS))
        current = bytes(random_.choice((0xFF, 0, 1, 2)) for _ in range(GROUPS))
        answers = [random_.choice((0, 1)) for _ in range(2 * GROUPS)]

        def loaded(harness, arguments, answers=answers):
            told = sum(1 for name, _ in harness._calls if name == "geometry_cache_block_load")
            return answers[told % len(answers)], tuple(arguments)

        t.check(
            "unknown_0004c2b0",
            random_.getrandbits(16) << 16,
            Buffer(wanted),
            Buffer(current),
            random_.randrange(6),
            random_.choice((0, 1, 0x100)),
            Buffer(random_.randbytes(GROUPS)),
            Buffer(random_.randbytes(1)),
            globals={"global_tag_instances": tags},
            externals={"geometry_cache_block_load": loaded},
        )
