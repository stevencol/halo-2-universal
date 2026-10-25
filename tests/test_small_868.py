# two functions of the way to the main menu (guided by halo2-decompiled): the
# objects of the datums of the scenario by their places, and a unit that
# stops zooming

import random
import struct

from difftest import Buffer

from test_object_types_more import after
from test_small_859 import BIG, big_objects
from test_units import COUNT

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(868)

    for n in range(400):
        # the objects that the iterators give, and their datums, are the test's
        counts = bytearray(random_.randbytes(0xAC))
        struct.pack_into("<i", counts, 0x50, random_.choice((0, 1, 3, 5)))
        struct.pack_into("<i", counts, 0xA8, random_.choice((0, 2, 4)))
        maximum = random_.choice((4, 8, 16))
        counts = Buffer(bytes(counts))
        objects = Buffer(random_.randbytes(maximum * 4))
        bsp = bytearray(random_.randbytes(0x144))
        struct.pack_into("<h", bsp, 0x140, random_.choice((0, 1, 3, 9, -1)))
        bsp = Buffer(bytes(bsp))
        things = Buffer(random_.randbytes(8 * 0xB0))
        base = after(counts, objects, bsp)
        given = {
            mask: [(base + random_.randrange(8) * 0xB0, random_.getrandbits(32)) for _ in range(random_.choice((0, 1, 3, 5)))]
            for mask in (0x40, 0x80)
        }
        datums = [(random_.choice((0, 1, 1)), random_.choice((0, 1, 2, 3, 7, 15, 20))) for _ in range(16)]

        def next_object(harness, arguments, given=given):
            # what the iterator was made with: its types, its flags and where it starts
            mask, flags, start = struct.unpack("<IBxh", bytes(harness.mu.mem_read(arguments[0], 8)))
            told = sum(1 for name, record in harness._calls if name == "object_iterator_next" and record[0] == mask)
            objects_ = given.get(mask, [])
            if told >= len(objects_):
                return 0, (mask, flags, start)
            address, index = objects_[told]
            harness.mu.mem_write(arguments[0] + 8, struct.pack("<I", index))
            return address, (mask, flags, start)

        def datum(harness, arguments, datums=datums):
            told = sum(1 for name, _ in harness._calls if name == "object_unknown_000b7a40")
            found, index = datums[told % len(datums)]
            if found:
                harness.mu.mem_write(arguments[1], struct.pack("<I", index))
            return (base if found else 0), (arguments[0],)

        t.check(
            "object_unknown_002960e0",
            counts,
            objects,
            maximum,
            globals={"global_structure_bsp": bsp},
            extra=[things],
            externals={"object_iterator_next": next_object, "object_unknown_000b7a40": datum},
        )

        # a unit that stops zooming
        units, pointers = big_objects(random_)
        index = random_.randrange(COUNT)
        at = 0x4C + COUNT * 12 + index * BIG
        units[at + 0x212] = random_.choice((0xFF, 0, 1))
        for slot in range(2):
            weapon = random_.choice((NONE, random_.randrange(COUNT)))
            struct.pack_into("<I", units, at + 0x218 + slot * 4, weapon)
            if weapon != NONE:
                struct.pack_into("<I", units, 0x4C + COUNT * 12 + weapon * BIG, random_.randrange(2))
        units[at + 0x240] = random_.choice((0xFF, 0, 3))
        units = Buffer(bytes(units), pointers=tuple(pointers))
        players = bytearray(random_.randbytes(0x48 + 4 * 0x21C))
        struct.pack_into("<I", players, 0x44, 0x48)
        for player in range(4):
            struct.pack_into("<h", players, 0x48 + player * 0x21C + 0x28, random_.choice((-1, 0, 1, 3)))
        players = Buffer(bytes(players), pointers=(0x44,))
        tags = bytearray(random_.randbytes(0x20 + 2 * 0x280))
        for tag in range(2):
            struct.pack_into("<I", tags, tag * 16 + 8, 0x20 + tag * 0x280)
            struct.pack_into("<I", tags, 0x20 + tag * 0x280 + 0x278, random_.choice((NONE, 0x1234, 0x5678)))
        tags = Buffer(bytes(tags), pointers=(8, 0x18))
        entries = Buffer(random_.randbytes(4 * 0x94 + 0x14 + 0x90))
        game = bytearray(random_.randbytes(0x140))
        struct.pack_into("<I", game, 0x134, 0x40)
        struct.pack_into("<I", game, 0x40 + 0xD0, random_.choice((NONE, 0x9999)))
        game = Buffer(bytes(game), pointers=(0x134,))
        unit = bytearray(random_.randbytes(0x140))
        struct.pack_into("<I", unit, 0x13C, random_.choice((NONE, 0, 1, 2, 3)))
        place = after(units, players, tags, entries, game)
        t.check(
            "unit_unknown_000c86e0",
            random_.getrandbits(16) << 16 | index,
            random_.choice((0, 1, 0x100)),
            globals={
                "object_header_data": units,
                "player_data": players,
                "global_tag_instances": tags,
                "unknown_004ed284": entries,
                "global_game_globals": game,
            },
            extra=[Buffer(bytes(unit))],
            externals={
                "object_try_and_get_and_verify_type": lambda harness, arguments, found=random_.choice((0, place, place)): (found, tuple(arguments)),
                "weapon_unknown_00100f70": lambda harness, arguments, zoomed=random_.choice((0, 1)): (zoomed, tuple(arguments)),
                "sound_unknown_001896c0": lambda harness, arguments: (0, tuple(arguments)),
            },
        )
