# five functions of the way to the main menu (guided by halo2-decompiled):
# whether a datum of the scenario is made now, the value of a ramp of the
# scenario (two ways), the animation of a name and how a channel moves

import random
import struct

from difftest import Buffer, f32

from test_small_856 import ANIMATION, GRAPH, some_graph

NONE = 0xFFFFFFFF
RAMPS = 4
NAMES = (1, 2, 3, 4, 5, 6)


def named_graph(random_, at: int) -> tuple[bytearray, tuple[int, ...]]:
    """A graph of some_graph with some animations (at 0x2C), each with a name; one it inherits from can be no tag."""
    graph, pointers = some_graph(random_, at)
    struct.pack_into("<i", graph, 0x2C, random_.choice((0, 1, 2, 4)))
    for index in range(4):
        struct.pack_into("<I", graph, 0xC0 + index * ANIMATION, random_.choice(NAMES))
    for index in range(2):
        if random_.random() < 0.15:
            struct.pack_into("<I", graph, 0xC0 + 4 * ANIMATION + index * 0x20 + 4, NONE)
    return graph, pointers


def ramps(random_) -> tuple[Buffer, Buffer]:
    """What unknown_004e6740 keeps of the ramps, and a scenario with them and their functions."""
    states = bytearray(random_.randbytes(4 + RAMPS * 32))
    for index in range(RAMPS):
        struct.pack_into("<f", states, 4 + index * 32, random_.uniform(-1, 2))
        struct.pack_into("<f", states, 4 + index * 32 + 0x18, random_.choice((0.0, random_.uniform(-1, 1))))
    scenario = bytearray(random_.randbytes(0x3D0 + RAMPS * 24 + RAMPS * 12))
    count = random_.choice((RAMPS, RAMPS, 3, 0))
    struct.pack_into("<iI", scenario, 0x3C0, count, 0x3D0)
    pointers = [0x3C4]
    for index in range(RAMPS):
        ramp = 0x3D0 + index * 24
        data = 0x3D0 + RAMPS * 24 + index * 12
        struct.pack_into("<iI", scenario, ramp + 0x0C, random_.choice((1, 1, 0)), data)
        pointers.append(ramp + 0x10)
        struct.pack_into("<h", scenario, ramp + 0x16, random_.choice((-1, -1, 0, 1, 2, 3, 5)))
        scenario[data + 1] = random_.choice((0, 0, 0x10, 0x0F))
        struct.pack_into("<2f", scenario, data + 4, random_.uniform(-2, 2), random_.uniform(-2, 2))
    return Buffer(bytes(states)), Buffer(bytes(scenario), pointers=tuple(pointers))


def run(t) -> None:
    random_ = random.Random(861)

    for n in range(400):
        # whether a datum of the scenario is made now
        datum = bytearray(random_.randbytes(0x34))
        datum[4] = random_.choice((0, 1, 2, 3))
        datum[0x30] = random_.choice((0, 1, 2, 2))
        struct.pack_into("<hh", datum, 0x2C, random_.choice((-1, 0, 1, 2)), 0)
        struct.pack_into("<h", datum, 2, random_.choice((-1, -1, 0)))
        t.check(
            "unit_unknown_000d5910",
            Buffer(bytes(datum)),
            random_.choice((0, 1, 2, 0x101)),
            globals={"global_structure_bsp_index": struct.pack("<h", random_.choice((0, 1, 2, 5, -1)))},
            externals={"unit_unknown_000d59f0": lambda harness, arguments, made=random_.choice((0, 1)): (made, tuple(arguments))},
        )

        # the value of a ramp: the functions of tags give their values in turn
        states, scenario = ramps(random_)
        values = [f32(random_.choice((0.0, 0.5, 1.0, -0.5, 1.5, random_.uniform(-2, 2)))) for _ in range(4)]

        def function(harness, arguments, values=values):
            told = sum(1 for name, _ in harness._calls if name == "unknown_0013b390")
            return values[told % len(values)], tuple(arguments)

        options = {
            "globals": {"unknown_004e6740": states, "global_scenario": scenario},
            "externals": {"unknown_0013b390": function},
        }
        t.check("game_unknown_00134fe0", random_.randrange(RAMPS), f32(random_.uniform(-1, 2)), **options)
        t.check("game_unknown_00134fc0", random_.randrange(RAMPS), **options)

        # the animation of a name: the graphs it inherits from are in the tags' buffer
        graph, pointers = named_graph(random_, 0)
        tags = bytearray(random_.randbytes(0x20))
        tag_pointers = []
        for index in range(2):
            struct.pack_into("<I", tags, index * 16 + 8, 0x20 + index * GRAPH)
            tag_pointers.append(index * 16 + 8)
            more, more_pointers = named_graph(random_, 0x20 + index * GRAPH)
            tags += more
            tag_pointers += more_pointers
        tags = Buffer(bytes(tags), pointers=tuple(tag_pointers))
        t.check(
            "havok_unknown_001dd0b0",
            Buffer(bytes(graph), pointers=pointers) if random_.random() < 0.95 else 0,
            Buffer(random_.randbytes(4)),
            random_.choice(NAMES + (7,)),
            globals={"global_tag_instances": tags},
            externals={"havok_unknown_001dd9d0": lambda harness, arguments: (0, tuple(arguments))},
        )

        # how a channel moves: what is wanted of its animation, and what it gives, are the test's
        channel = struct.pack("<IHH", random_.randrange(2), random_.getrandbits(16), random_.choice((0xFFFF, 0, 1, 2)))
        wanted = random_.randbytes(12)

        def want(harness, arguments, wanted=wanted):
            harness.mu.mem_write(arguments[0], wanted)
            return 0, (arguments[1], arguments[2])

        t.check(
            "havok_unknown_001c6dc0",
            Buffer(channel),
            Buffer(random_.randbytes(12)),
            Buffer(random_.randbytes(4)),
            random_.choice((0, 1, 7, 0x12340005)),
            globals={"global_tag_instances": Buffer(random_.randbytes(2 * 16))},
            externals={
                "havok_unknown_001ddb40": want,
                "unit_unknown_0020aa70": lambda harness, arguments: (
                    0,
                    (arguments[0], arguments[1], bytes(harness.mu.mem_read(arguments[2], 12)), arguments[3] & 0xFFFF),
                ),
            },
        )
