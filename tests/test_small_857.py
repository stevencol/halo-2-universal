# four functions of the way to the main menu (guided by halo2-decompiled): the
# texture of a bitmap with two other flags, an object that lets go of what
# the simulation tells of it, and what the cluster partition keeps of one

import random
import struct

from difftest import Buffer, GlobalRange

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(857)

    for n in range(400):
        bitmap = bytearray(random_.randbytes(0x74))
        frame = random_.choice((5, 100, -3, 0x7FFFFFF0))
        struct.pack_into("<i", bitmap, 0x70, random_.choice((frame, frame - 1, frame + 1, frame + 1)))
        struct.pack_into("<I", bitmap, 0x50, random_.choice((0, 0x80123400, 0x80123400)))
        for name in ("shell_unknown_0001cfb0", "shell_unknown_0003bcb0"):
            t.check(
                name,
                Buffer(bytes(bitmap)),
                globals={"texture_cache_globals+52": frame & 0xFFFFFFFF},
                externals={
                    "texture_cache_bitmap_get_loaded": lambda harness, arguments, loaded=random_.choice((0, 0, 0x80555000)): loaded,
                    "texture_cache_bitmap_get_hardware_format": lambda harness, arguments, made=random_.choice((0, 0x80777000)): made,
                },
            )

        # the objects: one with a thing of the simulation or none
        table = some_objects(random_)
        objects = bytearray(table.data)
        first = 0x4C + COUNT * 12
        index = random_.randrange(COUNT)
        at = first + index * OBJECT
        struct.pack_into("<I", objects, at + 0xD4, random_.choice((NONE, random_.getrandbits(32))))
        objects[0x4C + index * 12 + 2] = random_.choice((0, 0x40, 0x41, 0xBF))
        struct.pack_into("<I", objects, at + 0x04, random_.getrandbits(32) & ~0x200 | random_.choice((0, 0x200, 0x200)))
        objects = Buffer(bytes(objects), pointers=table.pointers)
        object_index = random_.getrandbits(16) << 16 | index
        t.check(
            "object_unknown_000a7a60",
            object_index,
            globals={"object_header_data": objects},
            externals={
                "unknown_000b5920": lambda harness, arguments: (0, tuple(arguments)),
                "object_types_call_unknown54": lambda harness, arguments: (0, tuple(arguments)),
            },
        )

        record = random_.randbytes(0x14)

        def keep(harness, arguments, record=record):
            harness.mu.mem_write(arguments[1], record)
            return 0, (arguments[0],)

        t.check(
            "object_unknown_000b8b70",
            object_index,
            globals={"object_header_data": objects, "collideable_object_cluster_partition": GlobalRange(12)},
            externals={
                "object_unknown_000b88e0": keep,
                "cluster_partition_set_extra": lambda harness, arguments: (
                    0,
                    (arguments[0], harness.normalize(arguments[1]), bytes(harness.mu.mem_read(arguments[2], 0x14)), *arguments[3:]),
                ),
            },
        )
