# eighteen small functions of props, clumps and others

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
PROPS, PROP = 4, 0xC4
REFS, REF = 4, 0x3C
TRACKS, TRACK = 3, 0x124
CLUMPS, CLUMP = 3, 0x50
POINTS, POINT = 2, 0x484
SCRIPTS, SCRIPT = 3, 0xD4


def run(t) -> None:
    random_ = random.Random(728)

    for n in range(200):
        number = random_.getrandbits(32)
        ref_salts = [random_.getrandbits(16) for _ in range(REFS)]
        prop_salts = [random_.getrandbits(16) for _ in range(PROPS)]
        a_ref_index = lambda index: ref_salts[index] << 16 | index
        a_prop_index = lambda index: prop_salts[index] << 16 | index

        def later(index: int, count: int, whole) -> int:
            """One with a bigger number, or none: so that no list goes around."""
            if index + 1 >= count or random_.random() < 0.3:
                return NONE
            return whole(random_.randrange(index + 1, count))

        def table(count: int, size: int, fill=None) -> Buffer:
            data = bytearray(random_.randbytes(0x4C + count * size))
            struct.pack_into("<I", data, 0x44, 0x4C)
            for index in range(count):
                if fill:
                    fill(data, 0x4C + index * size, index)
            return Buffer(bytes(data), pointers=(0x44,))

        def a_ref(data, at, index) -> None:
            struct.pack_into("<I", data, at + 4, random_.choice((number, random_.getrandbits(32))))
            struct.pack_into("<I", data, at + 0x14, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(TRACKS))))
            struct.pack_into("<h", data, at + 0x1C, random_.choice((3, 3, 2)))
            struct.pack_into("<h", data, at + 0x24, random_.choice((0, 0, 1, 2, 3, 4)))
            struct.pack_into("<I", data, at + 0x2C, later(index, REFS, a_ref_index))
            struct.pack_into("<I", data, at + 0x34, later(index, REFS, a_ref_index))

        def a_track(data, at, index) -> None:
            struct.pack_into("<I", data, at + 0x70 + 0x50, random_.choice((NONE, random_.getrandbits(32))))
            data[at + 0x70 + 0x88] = random_.choice((0, 1, 0xFF))

        def a_prop(data, at, index) -> None:
            struct.pack_into("<h", data, at + 4, random_.choice((0, 0, 0, 1, -1)))
            struct.pack_into("<I", data, at + 8, random_.choice((number, random_.getrandbits(32))))
            struct.pack_into("<I", data, at + 0x14, later(index, PROPS, a_prop_index))
            struct.pack_into("<I", data, at + 0x18, random_.choice((NONE, a_ref_index(random_.randrange(REFS)))))

        def a_clump(data, at, index) -> None:
            struct.pack_into("<I", data, at + 0x14, random_.choice((NONE, a_prop_index(random_.randrange(PROPS)), a_prop_index(0))))
            struct.pack_into("<I", data, at + 0x28, random_.choice((0, 100, 1000, 5000)))

        refs = lambda: table(REFS, REF, a_ref)
        tracks = lambda: table(TRACKS, TRACK, a_track)
        props = lambda: table(PROPS, PROP, a_prop)
        clumps = lambda: table(CLUMPS, CLUMP, a_clump)
        ref_index = a_ref_index(random_.randrange(REFS))
        prop_index = a_prop_index(random_.randrange(PROPS))
        clump_index = random_.getrandbits(16) << 16 | random_.randrange(CLUMPS)

        def one_ref() -> Buffer:
            data = bytearray(random_.randbytes(REF))
            a_ref(data, 0, 0)
            return Buffer(bytes(data))

        t.check("prop_ref_unknown_0025cca0", ref_index, globals={"prop_ref_data": refs()})
        t.check("prop_ref_index_get_tracking_unknown70", ref_index, globals={"prop_ref_data": refs(), "tracking_data": tracks()})
        t.check("prop_ref_unknown_0025d9b0", ref_index, globals={"prop_ref_data": refs(), "tracking_data": tracks()})
        for name in ("prop_ref_get_tracking_unknown70", "prop_ref_unknown_0025da00", "prop_ref_unknown_0025d970"):
            t.check(name, one_ref(), globals={"tracking_data": tracks()})
        t.check("prop_unknown_0026add0", prop_index, globals={"prop_data": props(), "prop_ref_data": refs()})
        t.check("prop_unknown_0026b8a0", prop_index, number, globals={"prop_data": props(), "prop_ref_data": refs()})
        t.check("clump_find_prop", clump_index, number, globals={"clump_data": clumps(), "prop_data": props()})
        t.check("clump_unknown_00268810", clump_index, globals={"clump_data": clumps()})
        t.check(
            "clump_unknown_0026b630",
            clump_index,
            random_.getrandbits(32),
            random_.getrandbits(32),
            globals={"clump_data": clumps()},
        )
        time = bytearray(random_.randbytes(0x24))
        struct.pack_into("<f", time, 4, random_.choice((1.0 / 30.0, 1.0 / 60.0)))
        struct.pack_into("<I", time, 8, random_.choice((0, 900, 1000, 1001, 1901, 3000, 6000)))
        t.check(
            "clump_unknown_0026b660",
            clump_index,
            random_.getrandbits(32),
            random_.getrandbits(32),
            globals={"clump_data": clumps(), "game_time_globals": Buffer(bytes(time))},
        )
        thing = bytearray(random_.randbytes(0x18))
        struct.pack_into("<f", thing, 0x14, random_.choice((0.0, -0.0, 1.0, -1.0, 1e-30, float("nan"))))
        t.check("unknown_0026b750", random_.getrandbits(32), random_.getrandbits(32), Buffer(bytes(thing)))
        t.check(
            "clump_unknown_0026b770",
            random_.choice((NONE, clump_index, clump_index)),
            globals={"clump_data": clumps(), "game_time_globals": Buffer(bytes(time))},
        )
        t.check("clump_unknown_0026bda0", clump_index, Buffer(random_.randbytes(8)), globals={"clump_data": clumps()})
        t.check(
            "prop_ref_iterator_next",
            Buffer(struct.pack("<2I", random_.getrandbits(32), random_.choice((NONE, ref_index, ref_index)))),
            globals={"prop_ref_data": refs()},
        )

        def a_point(data, at, index) -> None:
            struct.pack_into("<h", data, at + 8, random_.choice((0, 5, 20, 32)))
            struct.pack_into("<I", data, at + 0x40, random_.choice((0, 0xFFFFFFFF, random_.getrandbits(32))))

        t.check(
            "dynamic_firing_point_get_point",
            random_.getrandbits(1) << 31 | random_.randrange(POINTS) << 16 | random_.choice((0xFFFF, 0x8000, 0, 1, 4, 5, 19, 31, 32)),
            globals={"dynamic_firing_point_data": table(POINTS, POINT, a_point)},
        )
        t.check(
            "command_script_unknown_00276990",
            random_.getrandbits(32),
            globals={
                "command_script_data": table(SCRIPTS, SCRIPT),
                "unknown_00502410": random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(SCRIPTS))),
            },
        )
