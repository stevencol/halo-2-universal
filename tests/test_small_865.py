# three functions of the way to the main menu (guided by halo2-decompiled):
# whether a unit is in some state of its animations, the placement of an
# object made ready, and whether an effect is to be made

import random
import struct

from difftest import Buffer

from test_object_types_more import after
from test_small_859 import BIG, big_objects
from test_units import COUNT

NONE = 0xFFFFFFFF
STATES = (
    0x0400004A, 0x0400076C, 0x0400076D, 0x050000C3, 0x05000768, 0x05000769, 0x0500076A, 0x0500076B,
    0x0600008C, 0x0700076E, 0x080000C4, 0x0800076F, 0x0A000767, 0x0C000043, 0x0D00002B, 0x0D000042,
    0x0E00002A, 0x0E00003B, 0x0E0000C3, 0x0F00003A,
)
OTHERS = (0, 0x0400004B, 0x0400076B, 0x0400076E, 0x05000767, 0x0500076C, 0x0C000044, 0x0D00002C, 0x0F00003B, 0x7FFFFFFF)


def run(t) -> None:
    random_ = random.Random(865)

    for n in range(400):
        units, pointers = big_objects(random_)
        index = random_.randrange(COUNT)
        at = 0x4C + COUNT * 12 + index * BIG
        struct.pack_into("<I", units, at + 0x14, random_.choice((NONE, 1, 2)))
        struct.pack_into("<h", units, at + 0x1FC, random_.choice((-1, 0, 3)))
        struct.pack_into("<h", units, at + 0x12A, 0x200)
        struct.pack_into("<I", units, at + 0x200 + 0x7C, random_.choice(STATES + OTHERS + OTHERS))
        t.check(
            "unit_unknown_00111490",
            random_.getrandbits(16) << 16 | index,
            globals={"object_header_data": Buffer(bytes(units), pointers=tuple(pointers))},
            externals={
                "unit_unknown_000c7070": lambda harness, arguments, said=random_.choice((0, 0, 1)): (said, tuple(arguments)),
                "unit_unknown_00111590": lambda harness, arguments, said=random_.choice((0, 1, 1)): (said, tuple(arguments)),
            },
        )

        # the placement of an object: the object it comes from is the test's
        placement = Buffer(random_.randbytes(0xC4))
        owner = random_.choice((None, Buffer(random_.randbytes(12))))
        forward = Buffer(random_.randbytes(12))
        up = Buffer(random_.randbytes(12))
        arguments = (placement, owner) if owner else (placement,)
        place = after(*arguments, forward, up)
        object_ = bytearray(random_.randbytes(0x140))
        object_[0xAA] = random_.choice((0, 1, 2, 3, 33))
        t.check(
            "object_unknown_000b7930",
            placement,
            random_.getrandbits(32),
            random_.getrandbits(32),
            owner if owner else 0,
            globals={"global_forward3d": forward, "global_up3d": up},
            extra=[Buffer(bytes(object_))],
            externals={
                "object_try_and_get_and_verify_type": lambda harness, arguments, found=random_.choice((0, place)): (
                    found,
                    tuple(arguments),
                )
            },
        )

        # whether an effect is to be made: where it comes from is after its parameters
        parameters = bytearray(random_.randbytes(0x90))
        struct.pack_into("<I", parameters, 0x04, random_.choice((NONE, 0, 1, 0x12340001)))
        has_source = random_.random() < 0.9
        struct.pack_into("<I", parameters, 0x5C, 0x60 if has_source else 0)
        struct.pack_into("<3f", parameters, 0x60 + 0x08, *(random_.uniform(-20, 20) for _ in range(3)))
        struct.pack_into("<h", parameters, 0x60 + 0x20, random_.choice((-1, 0, 5, 31, 32, 100, 511)))
        tags = bytearray(random_.randbytes(0x20 + 2 * 0x30))
        for tag in range(2):
            struct.pack_into("<I", tags, tag * 16 + 8, 0x20 + tag * 0x30)
            struct.pack_into(
                "<2f", tags, 0x20 + tag * 0x30 + 0x28, random_.choice((0.0, 1.0, 5.0, 10.0)), random_.choice((0.0, 5.0, 20.0, 60.0))
            )
        tags = Buffer(bytes(tags), pointers=(8, 0x18))
        parameters = Buffer(bytes(parameters), pointers=(0x5C,) if has_source else ())
        flags = random_.choice((None, Buffer(random_.randbytes(8))))
        points = Buffer(struct.pack("<12f", *(random_.uniform(-20, 20) for _ in range(12))))
        buffers = (parameters, tags, flags) if flags else (parameters, tags)
        base = after(*buffers)
        found = [random_.choice((0, base + local * 12)) for local in range(4)]
        t.check(
            "effect_unknown_00176210",
            parameters,
            globals={
                "global_tag_instances": tags,
                "unknown_00510c50": flags if flags else 0,
                "unknown_004c56c0": random_.randbytes(64),
            },
            extra=[points],
            externals={
                "game_unknown_0016efb0_get_point": lambda harness, arguments, found=found: (found[arguments[0] & 3], tuple(arguments)),
            },
        )
