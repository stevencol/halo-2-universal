# two functions of the way to the main menu (guided by halo2-decompiled): the
# matrix of a havok component from that of its body, and the matrices of the
# nodes of a render model

import random
import struct

from difftest import Buffer

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF


def floats(random_, count: int) -> bytes:
    return struct.pack(f"<{count}f", *(random_.uniform(-2, 2) for _ in range(count)))


def run(t) -> None:
    random_ = random.Random(877)

    for n in range(400):
        table = some_objects(random_)
        objects = bytearray(table.data)
        index = random_.randrange(COUNT)
        objects[0x4C + index * 12 + 3] = random_.choice((0, 0, 1, 32))
        struct.pack_into("<I", objects, 0x4C + COUNT * 12 + index * OBJECT, 0)
        tags = bytearray(random_.randbytes(0x30 + 3 * 0x50 + 2 * 0x60))
        for tag in range(3):
            struct.pack_into("<I", tags, tag * 16 + 8, 0x30 + tag * 0x50)
        struct.pack_into("<I", tags, 0x30 + 0x38, 1)
        struct.pack_into("<I", tags, 0x30 + 0x50 + 0x04, 2)
        struct.pack_into("<I", tags, 0x30 + 2 * 0x50 + 0x4C, 0x30 + 3 * 0x50)
        tags[0x30 + 3 * 0x50:] = floats(random_, 2 * 0x60 // 4)
        component = bytearray(random_.randbytes(0x80))
        struct.pack_into("<I", component, 0x04, random_.getrandbits(32) & ~0x200 | random_.choice((0, 0x200)))
        struct.pack_into("<I", component, 0x08, random_.getrandbits(16) << 16 | index)
        component[0x18] = random_.choice((0, 1, 2, 0xFF, 0x80))
        component[0x19] = random_.choice((0, 1, 0xFF))
        struct.pack_into("<i", component, 0x74, random_.choice((0, 1, 3)))

        def offset(harness, arguments, value=random_.randbytes(4), found=random_.choice((0, 1))):
            harness.mu.mem_write(arguments[1], value)
            return found, (arguments[0],)

        t.check(
            "havok_unknown_001d43d0",
            Buffer(bytes(component)),
            Buffer(floats(random_, 13)),
            Buffer(random_.randbytes(0x34)),
            globals={
                "object_header_data": Buffer(bytes(objects), pointers=table.pointers),
                "global_tag_instances": Buffer(bytes(tags), pointers=(8, 0x18, 0x28, 0x30 + 2 * 0x50 + 0x4C)),
            },
            externals={"object_unknown_000e0d70": offset},
        )

        # a render model with a tree of nodes: each a child of an earlier one
        count = random_.choice((1, 2, 4, 6))
        children = {node: [] for node in range(count)}
        parents = [NONE] + [random_.randrange(node) for node in range(1, count)]
        for node in range(1, count):
            children[parents[node]].append(node)
        model = bytearray(random_.randbytes(0x50 + count * 0x60))
        struct.pack_into("<iI", model, 0x48, count, 0x50)
        for node in range(count):
            first = children[node][0] if children[node] else -1
            siblings = children[parents[node]] if node else [0]
            place = siblings.index(node)
            following = siblings[place + 1] if place + 1 < len(siblings) else -1
            struct.pack_into("<hhh", model, 0x50 + node * 0x60 + 0x04, parents[node] if node else -1, first, following)
        t.check(
            "unknown_0016d9c0",
            Buffer(floats(random_, 3)),
            Buffer(floats(random_, 3)),
            Buffer(floats(random_, 3)),
            Buffer(bytes(model), pointers=(0x4C,)),
            Buffer(random_.randbytes(count * 0x34)),
            Buffer(floats(random_, count * 8)),
        )
