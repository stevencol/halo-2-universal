# two functions of the way to the main menu (guided by halo2-decompiled): an
# effect that goes to an event, and the location of an effect

import random
import struct

from difftest import Buffer

from test_object_types_more import after

NONE = 0xFFFFFFFF
EFFECTS = 4
EFFECT = 0x100


def run(t) -> None:
    random_ = random.Random(878)

    for n in range(400):
        # the effects as a data array, of two tags with three events each
        table = bytearray(random_.randbytes(0x4C + EFFECTS * EFFECT))
        struct.pack_into("<i", table, 0x24, EFFECT)
        struct.pack_into("<i", table, 0x38, random_.choice((EFFECTS, 2)))
        struct.pack_into("<I", table, 0x44, 0x4C)
        salts = [random_.choice((0, 1, 0x55)) for _ in range(EFFECTS)]
        for effect in range(EFFECTS):
            struct.pack_into("<hBB", table, 0x4C + effect * EFFECT, salts[effect], random_.getrandbits(8), 0)
            struct.pack_into("<I", table, 0x4C + effect * EFFECT + 4, random_.randrange(2))
        effects = Buffer(bytes(table), pointers=(0x44,))
        events = 0x20 + 2 * 0x20
        tags = bytearray(random_.randbytes(events + 2 * 3 * 0x38))
        for tag in range(2):
            struct.pack_into("<I", tags, tag * 16 + 8, 0x20 + tag * 0x20)
            struct.pack_into("<iI", tags, 0x20 + tag * 0x20 + 0x14, random_.choice((0, 1, 3)), events + tag * 3 * 0x38)
        for event in range(6):
            lower = random_.choice((0.0, 1.0, 2.5))
            struct.pack_into("<2f", tags, events + event * 0x38 + 0x08, lower, random_.choice((lower, lower, 3.0, 0.5)))
        tags = Buffer(bytes(tags), pointers=(8, 0x18, 0x20 + 0x18, 0x40 + 0x18))
        seed = Buffer(random_.randbytes(4))
        place = after(effects, tags)
        effect = random_.randrange(EFFECTS)
        t.check(
            "effect_unknown_001782a0",
            random_.choice((NONE, (salts[effect] & 0xFFFF) << 16 | effect, (salts[effect] ^ 1) << 16 | effect)),
            random_.choice((-1, 0, 1, 2, 3)),
            globals={"effect_data": effects, "global_tag_instances": tags},
            extra=[seed],
            externals={
                "effect_definition_get_random_seed": lambda harness, arguments, place=place: (place, tuple(arguments)),
                "effect_unknown_0017add0": lambda harness, arguments: (0, tuple(arguments)),
            },
        )

        # the location of an effect: its locations and the leaves are the test's
        effect = bytearray(random_.randbytes(0x90))
        struct.pack_into("<I", effect, 4, random_.randrange(2))
        for slot in range(4):
            struct.pack_into("<I", effect, 0x7C + slot * 4, random_.choice((NONE, NONE, random_.getrandbits(32))))
        definitions = bytearray(random_.randbytes(0x40))
        for tag in range(2):
            struct.pack_into("<I", definitions, tag * 16 + 8, 0x20 + tag * 0x10)
            struct.pack_into("<i", definitions, 0x20 + tag * 0x10 + 0x0C, random_.choice((0, 1, 2, 4)))
        bsp = bytearray(random_.randbytes(0x40 + 4 * 8))
        struct.pack_into("<I", bsp, 0x30, 0x40)
        for leaf in range(4):
            struct.pack_into("<h", bsp, 0x40 + leaf * 8, random_.choice((-1, 0, 3)))
        effect = Buffer(bytes(effect))
        definitions = Buffer(bytes(definitions), pointers=(8, 0x18))
        bsp = Buffer(bytes(bsp), pointers=(0x30,))
        location = Buffer(random_.randbytes(0x40))
        place = after(effect, definitions, bsp)

        def next_location(harness, arguments, found=random_.choice((0, place, place))):
            # its first argument is not used, and clang does not pass it
            return found, (struct.unpack("<I", bytes(harness.mu.mem_read(arguments[1], 4)))[0], arguments[2] & 0xFFFF)

        t.check(
            "effect_unknown_001789f0",
            effect,
            globals={
                "global_tag_instances": definitions,
                "global_structure_bsp": bsp,
                "global_structure_bsp_index": struct.pack("<h", random_.choice((0, 1, -1))),
                "global_bsp3d": 0x80345670,
            },
            extra=[location],
            externals={
                "effect_location_unknown_0017ae50": next_location,
                "bsp3d_test_point": lambda harness, arguments, leaf=random_.choice((NONE, 0, 1, 3)): (
                    leaf,
                    (arguments[0], arguments[1], bytes(harness.mu.mem_read(arguments[2], 12))),
                ),
            },
        )
