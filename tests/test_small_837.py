# three functions of the way to the main menu: a new particle location, the
# effect locations of an effect deleted, and some vocalization records
# deleted

import random
import struct

from difftest import Buffer

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
LOCATION = 0x34
EFFECT = 0x90
EFFECT_LOCATION = 0x3C
RECORD = 0x54


def run(t) -> None:
    random_ = random.Random(837)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    def chain(data: bytearray, first: int, size: int, count: int, at: int, used: set) -> int:
        free = [entry for entry in range(count) if entry not in used]
        order = random_.sample(free, random_.randrange(min(3, len(free)) + 1))
        used.update(order)
        for place, entry in enumerate(order):
            struct.pack_into("<I", data, first + entry * size + at, salted(order[place + 1]) if place + 1 < len(order) else NONE)
        return salted(order[0]) if order else NONE

    for n in range(400):
        locations = bytearray(random_.randbytes(0x50 + 4 * LOCATION))
        struct.pack_into("<I", locations, 0x44, 0x50)
        made = random_.choice((NONE, salted(random_.randrange(4))))
        t.check(
            "particle_location_new",
            globals={
                "particle_location_data": Buffer(bytes(locations), pointers=(0x44,)),
                "unknown_00468788": Buffer(random_.randbytes(12)),
                "unknown_004ba034": random_.getrandbits(32),
                "global_random_seed_reference": Buffer(random_.randbytes(8)),
            },
            externals={"datum_new": lambda harness, arguments, made=made: (made, tuple(arguments))},
        )

        # effects (a data array of 0x90 bytes) with slots at 0x7C, and their effect locations
        effects = bytearray(random_.randbytes(0x50 + 3 * EFFECT))
        struct.pack_into("<iiI", effects, 0x24, EFFECT, 0, 0)
        struct.pack_into("<i", effects, 0x38, random_.choice((2, 3)))
        struct.pack_into("<I", effects, 0x44, 0x50)
        places = bytearray(random_.randbytes(0x50 + 8 * EFFECT_LOCATION))
        struct.pack_into("<I", places, 0x44, 0x50)
        used: set = set()
        salts = []
        for effect in range(3):
            at = 0x50 + effect * EFFECT
            salt = random_.choice((0, 0x1234, 0x5678))
            salts.append(salt)
            struct.pack_into("<HI", effects, at, salt, 0, )
            struct.pack_into("<I", effects, at + 4, random_.randrange(2))
            for slot in range(3):
                struct.pack_into("<I", effects, at + 0x7C + slot * 4, chain(places, 0x50, EFFECT_LOCATION, 8, 4, used))
        tags = bytearray(random_.randbytes(0x20 + 2 * 0x10))
        for tag in range(2):
            struct.pack_into("<I", tags, tag * 0x10 + 8, 0x20 + tag * 0x10)
            struct.pack_into("<i", tags, 0x20 + tag * 0x10 + 0x0C, random_.choice((0, 1, 3)))
        effect = random_.randrange(3)
        t.check(
            "effect_unknown_001773a0",
            random_.choice((NONE, salts[effect] << 16 | effect, 0x1111 << 16 | effect)),
            globals={
                "effect_data": Buffer(bytes(effects), pointers=(0x44,)),
                "effect_location_data": Buffer(bytes(places), pointers=(0x44,)),
                "global_tag_instances": Buffer(bytes(tags), pointers=(8, 0x18)),
            },
            externals={"datum_delete": lambda harness, arguments: (0, tuple(arguments))},
        )

        # records chained from 0x7D0 of the owner; units (objects) that may point back at 0x1C past their short at 0x342
        records = bytearray(random_.randbytes(0x50 + 5 * RECORD))
        struct.pack_into("<I", records, 0x44, 0x50)
        owner = bytearray(random_.randbytes(0x7D8))
        order = random_.sample(range(5), random_.randrange(6))
        table = some_objects(random_)
        objects = bytearray(table.data) + bytearray(random_.randbytes(0x400))
        ids = [salted(record) for record in range(5)]
        for index in range(COUNT):
            at = FIRST + index * OBJECT
            struct.pack_into("<h", objects, at + 0x342, random_.choice((0, 0x40)))
            offset = struct.unpack_from("<h", objects, at + 0x342)[0]
            struct.pack_into("<I", objects, at + offset + 0x1C, random_.choice(ids + [NONE]))
        for place, record in enumerate(order):
            at = 0x50 + record * RECORD
            struct.pack_into("<I", records, at + 0x50, ids[order[place + 1]] if place + 1 < len(order) else NONE)
            struct.pack_into("<I", records, at + 4, random_.choice((NONE, salted(random_.randrange(COUNT)))))
            records[at + 0x48] = random_.choice((0, 1))
        struct.pack_into("<Ih", owner, 0x7D0, ids[order[0]] if order else NONE, len(order))
        t.check(
            "vocalization_records_unknown_0020fe20",
            Buffer(bytes(owner)),
            globals={
                "vocalization_record_data": Buffer(bytes(records), pointers=(0x44,)),
                "object_header_data": Buffer(bytes(objects), pointers=table.pointers),
            },
            externals={"datum_delete": lambda harness, arguments: (0, tuple(arguments))},
        )
