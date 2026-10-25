# five functions of the way to the main menu (guided by halo2-decompiled): the
# palette entry of a datum of the scenario, a bitmap of a group wanted at a
# stage, an animation of a graph and its data, and the value of a function
# of a tag

import random
import struct

from difftest import Buffer, f32

from test_object_types_more import after

NONE = 0xFFFFFFFF
TYPES = 13
TYPE = 0x10
ANIMATION = 0x6C
GRAPH = 0xC0 + 4 * ANIMATION + 2 * 0x20 + 4 * 0x14


def some_graph(random_, at: int) -> tuple[bytearray, tuple[int, ...]]:
    """A graph at an offset of a buffer: four animations, two graphs it inherits from (tags 0 and 1) and four resources."""
    graph = bytearray(random_.randbytes(GRAPH))
    animations = 0xC0
    inherited = animations + 4 * ANIMATION
    resources = inherited + 2 * 0x20
    struct.pack_into("<I", graph, 0x30, at + animations)
    struct.pack_into("<i", graph, 0x4C, 2)
    struct.pack_into("<I", graph, 0x50, at + inherited)
    struct.pack_into("<I", graph, 0xB0, at + resources)
    for index in range(2):
        struct.pack_into("<I", graph, inherited + index * 0x20 + 4, random_.choice((0, 1)) | random_.getrandbits(16) << 16)
    for index in range(4):
        animation = animations + index * ANIMATION
        graph[animation + 0x16] = random_.choice((0, 0x10, 0x11, 0xEF))
        struct.pack_into("<i", graph, animation + 0x24, random_.randrange(4))
        struct.pack_into("<i", graph, animation + 0x28, random_.randrange(-0x100, 0x100))
    return graph, (at + 0x30, at + 0x50, at + 0xB0)


def run(t) -> None:
    random_ = random.Random(856)

    for n in range(400):
        # the palette entry of a datum: the scenario has two blocks
        datum = bytearray(random_.randbytes(0x30))
        struct.pack_into("<h", datum, 0, random_.choice((-1, 0, 1, 2, 5, -2)))
        datum[0x2E] = random_.choice((0, 1, 5, 12, 12, 13, 0xFF, 0x80))
        scenario = bytearray(random_.randbytes(0x40))
        for block in (0x10, 0x20):
            struct.pack_into("<iI", scenario, block, random_.choice((0, 1, 3, 6)), 0x30)
        datum = Buffer(bytes(datum))
        scenario = Buffer(bytes(scenario), pointers=(0x14, 0x24))
        base = after(datum, scenario)
        types = bytearray(random_.randbytes(TYPES * TYPE))
        for index in range(TYPES):
            struct.pack_into("<h", types, index * TYPE + 0x0C, random_.choice((0x10, 0x20, 0x20, -1)))
        t.check(
            "object_unknown_000d58a0",
            datum,
            scenario,
            globals={"object_type_definitions": struct.pack("<13I", *(base + index * TYPE for index in range(TYPES)))},
            extra=[Buffer(bytes(types))],
        )

        # a bitmap of a group wanted at a stage
        tags = bytearray(random_.randbytes(4 * 16 + 4 * 0x48))
        for index in range(4):
            struct.pack_into("<I", tags, index * 16 + 8, 0x40 + index * 0x48)
            struct.pack_into("<i", tags, 0x40 + index * 0x48 + 0x44, random_.choice((0, 1, 3, 7, -1)))
        tags = Buffer(bytes(tags), pointers=tuple(index * 16 + 8 for index in range(4)))
        bitmap = Buffer(random_.randbytes(0x20))
        place = after(tags)
        found = random_.choice((0, place, place))
        loaded = random_.choice((0, 0x1234))
        t.check(
            "rasterizer_unknown_00014480",
            random_.choice((NONE, 0, 1, 2, 3, 0x12340002)),
            random_.randrange(4),
            random_.choice((0, 1, 5, -3, 0x7FFF, 0x12348003)),
            globals={
                "global_tag_instances": tags,
                "unknown_0051f3c8": random_.randbytes(0x20),
                "unknown_00485af4": random_.randbytes(0x20),
            },
            extra=[bitmap],
            externals={
                "bitmap_group_get_bitmap": lambda harness, arguments, found=found: (found, (arguments[0], arguments[1] & 0xFFFF)),
                "shell_bitmap_get_texture": lambda harness, arguments, loaded=loaded: (loaded, (arguments[0], arguments[1])),
                "shell_bitmap_get_texture_blocking": lambda harness, arguments, texture=random_.getrandbits(32): (texture, (arguments[0], arguments[1])),
            },
        )

        # an animation of a graph: the graphs it inherits from are in the tags' buffer
        graph, pointers = some_graph(random_, 0)
        inherited = bytearray(random_.randbytes(0x20))
        for index in range(2):
            struct.pack_into("<I", inherited, index * 16 + 8, 0x20 + index * GRAPH)
        inherited_pointers = [index * 16 + 8 for index in range(2)]
        for index in range(2):
            more, more_pointers = some_graph(random_, 0x20 + index * GRAPH)
            inherited += more
            inherited_pointers += more_pointers
        tags = Buffer(bytes(inherited), pointers=tuple(inherited_pointers))
        id_ = random_.choice((0xFFFF, 0, 1, 2, 3)) << 16 | random_.choice((0xFFFF, 0xFFFF, 0, 1))
        t.check(
            "havok_unknown_001daea0",
            Buffer(bytes(graph), pointers=pointers),
            id_,
            globals={"global_tag_instances": tags},
            externals={"havok_unknown_001dd9d0": lambda harness, arguments: (0, tuple(arguments))},
        )
        t.check(
            "havok_unknown_001dd7c0",
            Buffer(bytes(graph), pointers=pointers),
            random_.choice((0, 1, 2, 3)) << 16 | random_.choice((0xFFFF, 0xFFFF, 0, 1)),
            globals={"global_tag_instances": tags},
            externals={
                "animation_cache_get_data": lambda harness, arguments, data=random_.getrandbits(32): (data, tuple(arguments)),
            },
        )

        # the value of a function of a tag
        function = bytearray(random_.randbytes(0x14))
        struct.pack_into("<i", function, 0, random_.choice((0, 1, 5, -1)))
        has_data = random_.random() < 0.85
        struct.pack_into("<I", function, 4, 8 if has_data else 0)
        function[9] = random_.choice((0, 0x10, 0x0F, 0xF0))
        struct.pack_into("<2f", function, 0x0C, random_.uniform(-5, 5), random_.uniform(-5, 5))
        value = f32(random_.choice((0.0, 0.25, 1.0, -0.5, 1.5, float("nan"), random_.uniform(-2, 2))))
        t.check(
            "unknown_0013bb90",
            Buffer(bytes(function), pointers=(4,) if has_data else ()),
            f32(random_.uniform(-1, 2)),
            f32(random_.uniform(0, 3)),
            externals={"unknown_0013b390": lambda harness, arguments, value=value: (value, tuple(arguments))},
        )
