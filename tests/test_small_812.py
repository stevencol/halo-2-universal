# four medium functions of the way to the main menu: an object put in and
# taken out of a list with a time, an entry of the scenario that something
# names, something of 0x84 bytes made nothing, and a speed and a distance
# through three phases

import random
import struct

from difftest import Buffer, f32

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12


def run(t) -> None:
    random_ = random.Random(812)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    for n in range(400):
        # objects in the list (first at 0x08 of unknown_004de2f4, next at 0x1C) have the bit 14
        table = some_objects(random_)
        objects = bytearray(table.data)
        ids = [salted(index) for index in range(COUNT)]
        members = random_.sample(range(COUNT), random_.choice((0, 1, 2, 3)))
        for index in range(COUNT):
            at = FIRST + index * OBJECT
            flags = random_.getrandbits(32) & ~0xC000
            flags |= 0x4000 if index in members else random_.choice((0, 0x8000))
            struct.pack_into("<I", objects, at + 4, flags)
            struct.pack_into("<i", objects, at + 0x20, random_.choice((0, 100, 1000)))
            objects[0x4C + index * 12 + 2] = random_.choice((0, 1))
        for position, index in enumerate(members):
            following = ids[members[position + 1]] if position + 1 < len(members) else NONE
            struct.pack_into("<I", objects, FIRST + index * OBJECT + 0x1C, following)
        list_ = bytearray(random_.randbytes(0x0C))
        struct.pack_into("<I", list_, 8, ids[members[0]] if members else NONE)
        time = bytearray(random_.randbytes(0x24))
        struct.pack_into("<I", time, 8, random_.choice((0, 50, 900)))
        add = random_.choice((0, 1))
        candidates = members if (not add and members) else list(range(COUNT))
        index = random_.choice(candidates)
        t.check(
            "object_unknown_000bb950",
            ids[index],
            add,
            random_.choice((0, 10, 200)),
            globals={
                "object_header_data": Buffer(bytes(objects), pointers=table.pointers),
                "unknown_004de2f4": Buffer(bytes(list_)),
                "game_time_globals": Buffer(bytes(time)),
            },
        )

        # the scenario's entries (8 bytes) at 0x08 and the bsp's elements (0x44) at 0x214
        count = random_.choice((0, 1, 3, 4))
        scenario = bytearray(random_.randbytes(0x220 + 4 * 8 + 2 * 0x44))
        struct.pack_into("<iI", scenario, 0x08, count, 0x220)
        struct.pack_into("<I", scenario, 0x214, 0x220 + 4 * 8)
        for entry in range(4):
            struct.pack_into("<I", scenario, 0x220 + entry * 8 + 4, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(2))))
        for element in range(2):
            at = 0x220 + 4 * 8 + element * 0x44
            scenario[at + 0x3C] = random_.choice((0, 1))
            struct.pack_into("<h", scenario, at + 0x40, random_.choice((-1, 0, 2, 5, -3)))
        tags = bytearray(random_.randbytes(0x40))
        for tag in range(2):
            struct.pack_into("<I", tags, tag * 0x10 + 8, 0x20 + tag * 0x10)
        tags += random_.randbytes(0x20)
        data = bytearray(random_.randbytes(0x70))
        data[0x6C] = random_.choice((0xFF, 0xFF, 0, 2, 5, 0x80))
        data[0x6E] = random_.choice((0xFF, 0, 1, 3, 0x80))
        t.check(
            "scenario_unknown_00130f60",
            Buffer(bytes(data)),
            Buffer(random_.randbytes(4)),
            globals={
                "global_scenario": Buffer(bytes(scenario), pointers=(0x0C, 0x214)),
                "global_structure_bsp_index": struct.pack("<h", random_.randrange(2)),
                "global_tag_instances": Buffer(bytes(tags), pointers=(8, 0x18)),
            },
        )

        t.check("havok_unknown_001cafc0", Buffer(random_.randbytes(0x84)))

        phases = bytearray(random_.randbytes(0x20))
        phases[0] = random_.choice((0, 0, 0, 1))
        struct.pack_into("<4I", phases, 0x0C, f32(random_.uniform(-2, 2)), f32(random_.choice((0.0, 0.5, 1.0, -1.0))),
                         f32(random_.choice((0.0, 0.5, 2.0))), f32(random_.uniform(-2, 2)))
        struct.pack_into("<I", phases, 0x1C, f32(random_.choice((0.0, 0.5, 3.0))))
        t.check(
            "unit_unknown_000c7750",
            Buffer(bytes(phases)),
            Buffer(random_.randbytes(4)),
            f32(random_.choice((0.0, 0.25, 1.0, 3.0, 10.0, -1.0))),
            f32(random_.uniform(-3, 3)),
            f32(random_.uniform(-3, 3)),
            Buffer(random_.randbytes(4)),
        )
