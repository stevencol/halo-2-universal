# four functions of the way to the main menu (guided by halo2-decompiled): the
# blend screen of an animation, the animation of a channel and whether it
# has a bit, and a bitmap of a group wanted at a stage

import random
import struct

from difftest import Buffer, f32

from test_object_types_more import after
from test_small_856 import ANIMATION, GRAPH, some_graph

NONE = 0xFFFFFFFF
BLENDS = GRAPH + 4 * 0x1C


def graph_with_blends(random_, at: int) -> tuple[bytearray, tuple[int, ...]]:
    """A graph of some_graph, with four blend screens after it and an animation of each, or none."""
    graph, pointers = some_graph(random_, at)
    graph += random_.randbytes(4 * 0x1C)
    struct.pack_into("<I", graph, 0x28, at + GRAPH)
    for index in range(4):
        animation = 0xC0 + index * ANIMATION
        graph[animation + 0x12] = random_.choice((0xFF, 0, 1, 2, 3))
        graph[animation + 0x18] = random_.getrandbits(8)
    return graph, pointers + (at + 0x28,)


def graphs(random_) -> Buffer:
    """The tags: two graphs, each inheriting from tags 0 and 1."""
    tags = bytearray(random_.randbytes(0x20))
    pointers = []
    for index in range(2):
        struct.pack_into("<I", tags, index * 16 + 8, 0x20 + index * BLENDS)
        pointers.append(index * 16 + 8)
        more, more_pointers = graph_with_blends(random_, 0x20 + index * BLENDS)
        tags += more
        pointers += more_pointers
    return Buffer(bytes(tags), pointers=tuple(pointers))


def run(t) -> None:
    random_ = random.Random(858)
    told = {"havok_unknown_001dd9d0": lambda harness, arguments: (0, tuple(arguments))}

    for n in range(400):
        graph, pointers = graph_with_blends(random_, 0)
        tags = graphs(random_)
        t.check(
            "havok_unknown_001daf30",
            Buffer(bytes(graph), pointers=pointers),
            random_.choice((0xFFFF, 0, 1, 2, 3)) << 16 | random_.choice((0xFFFF, 0xFFFF, 0, 1)),
            globals={"global_tag_instances": tags},
            externals=told,
        )

        channel = struct.pack(
            "<IHH",
            random_.choice((0, 1)) | random_.getrandbits(16) << 16,
            random_.choice((0xFFFF, 0xFFFF, 0, 1)),
            random_.choice((0xFFFF, 0, 1, 2, 3)),
        ) + random_.randbytes(8)
        t.check("havok_unknown_001c6440", Buffer(channel), globals={"global_tag_instances": tags}, externals=told)
        if random_.random() < 0.2:
            channel = struct.pack("<I", NONE) + channel[4:]
        t.check("havok_unknown_001c7300", Buffer(channel), globals={"global_tag_instances": tags}, externals=told)

        # a bitmap of a group wanted at a stage: tag 3 has no group
        groups = bytearray(random_.randbytes(4 * 16 + 3 * 0x4C))
        for index in range(4):
            struct.pack_into("<I", groups, index * 16 + 8, 0x40 + index * 0x4C if index < 3 else 0)
        for index in range(3):
            struct.pack_into("<iI", groups, 0x40 + index * 0x4C + 0x44, random_.choice((0, 1, 3, -1)), 0)
        groups = Buffer(bytes(groups), pointers=tuple(index * 16 + 8 for index in range(3)) + tuple(0x40 + index * 0x4C + 0x48 for index in range(3)))
        # the game looks at the group of the tag before it sees there is none: the default one only can have none
        tag = random_.choice((NONE, 0, 1, 2, 0x12340001))
        defaults = bytearray(random_.randbytes(0x68))
        struct.pack_into("<I", defaults, 0x64, random_.choice((NONE, 0, 1) if tag == NONE else (NONE, 0, 1, 3)))
        defaults = Buffer(bytes(defaults))
        bitmap = Buffer(random_.randbytes(0x20))
        found = random_.choice((0, after(groups, defaults), after(groups, defaults)))
        t.check(
            "rasterizer_unknown_000143c0",
            tag,
            random_.choice((-1, 0, 1, 2, 5)),
            random_.randrange(4),
            f32(random_.choice((0.0, 1.0, -0.5))),
            globals={
                "global_tag_instances": groups,
                "unknown_00485a80": defaults,
                "unknown_0051f3c8": random_.randbytes(0x20),
                "unknown_00485af4": random_.randbytes(0x20),
            },
            extra=[bitmap],
            externals={
                "bitmap_group_get_bitmap": lambda harness, arguments, found=found: (found, (arguments[0], arguments[1] & 0xFFFF)),
                "shell_bitmap_get_texture_blocking": lambda harness, arguments, texture=random_.getrandbits(32): (texture, (arguments[0], arguments[1])),
            },
        )
