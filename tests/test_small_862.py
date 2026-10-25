# four functions of the way to the main menu (guided by halo2-decompiled): the
# seventeen masks of points, how fast a channel of animations moves, a color
# as hue, saturation and value, and the datum of an identifier of a
# placement

import random
import struct

from difftest import Buffer, GlobalData, f32

from test_object_types_more import after

NONE = 0xFFFFFFFF
TYPES = 13
TYPE = 0x10


def run(t) -> None:
    random_ = random.Random(862)

    for n in range(400):
        if n < 40:
            points = struct.pack("<32h", *(random_.randrange(4) for _ in range(32)))
            t.check(
                "rasterizer_unknown_0001c1b0",
                globals={"unknown_0043f188": points, "unknown_00487288": random_.randbytes(17 * 0x80)},
            )

        # how fast a channel moves: what havok_unknown_001c6dc0 gives is the test's
        channel = bytearray(random_.randbytes(0x20))
        struct.pack_into("<H", channel, 6, random_.choice((0xFFFF, 0, 1)))
        struct.pack_into("<2f", channel, 0x18, random_.uniform(-2, 2), random_.choice((0.0, 2.5, -3.7, random_.uniform(-100, 100))))
        movement = struct.pack("<3f", *(random_.uniform(-5, 5) for _ in range(3)))
        movement_value = struct.pack("<f", random_.uniform(-5, 5))

        def move(harness, arguments, movement=movement, movement_value=movement_value):
            harness.mu.mem_write(arguments[1], movement)
            harness.mu.mem_write(arguments[2], movement_value)
            return 0, (arguments[0], arguments[3])

        t.check(
            "havok_unknown_001c6d00",
            Buffer(bytes(channel)),
            Buffer(random_.randbytes(12)),
            Buffer(random_.randbytes(4)),
            globals={"global_zero_vector3d": Buffer(bytes(12))},
            externals={"havok_unknown_001c6dc0": move},
        )

        # a color as hue, saturation and value
        choices = (0.0, 0.0, 0.5, 1.0, 0.25, random_.uniform(0, 1), float("nan"))
        rgb = [random_.choice(choices) for _ in range(3)]
        if random_.random() < 0.3:
            rgb[random_.randrange(3)] = rgb[random_.randrange(3)]
        t.check(
            "real_rgb_color_to_real_hsv_color",
            Buffer(struct.pack("<3f", *rgb)),
            Buffer(random_.randbytes(12)),
        )

        # the datum of an identifier: the scenario has two blocks of eight
        scenario = bytearray(random_.randbytes(0x30 + 2 * 8 * 0x38))
        for block in (0x10, 0x20):
            struct.pack_into("<iI", scenario, block, random_.choice((0, 1, 4, 8)), 0x30 + (block // 0x10 - 1) * 8 * 0x38)
        identifier = bytearray(random_.randbytes(8))
        identifier[6] = random_.choice((0, 1, 5, 12))
        identifier[7] = random_.choice((0, 1, 1, 2, 0xFF))
        for element in range(16):
            at = 0x30 + element * 0x38 + 0x28
            if random_.random() < 0.5:
                scenario[at:at + 8] = identifier
                if random_.random() < 0.3:
                    scenario[at + 4] ^= 1
                if random_.random() < 0.2:
                    scenario[at + random_.choice((0, 6, 7))] ^= 1
        scenario = Buffer(bytes(scenario), pointers=(0x14, 0x24))
        identifier = Buffer(bytes(identifier))
        index_out = random_.choice((None, Buffer(random_.randbytes(4))))
        buffers = (identifier, index_out, scenario) if index_out else (identifier, scenario)
        base = after(*buffers)
        types = bytearray(random_.randbytes(TYPES * TYPE))
        for index in range(TYPES):
            struct.pack_into(
                "<hhh", types, index * TYPE + 0x0A, random_.choice((0x10, 0x20, 0x20, -1)), 0, random_.choice((0x38, 0x38, 0x1C))
            )
        t.check(
            "object_unknown_000b7a40",
            identifier,
            index_out if index_out else 0,
            globals={
                "global_scenario": scenario,
                "object_type_definitions": struct.pack("<13I", *(base + index * TYPE for index in range(TYPES))),
            },
            extra=[Buffer(bytes(types))],
        )
