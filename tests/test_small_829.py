# five small functions of the way to the main menu: three searches one in
# another, a particle location deleted, whether a unit is in some state, the
# element of a tag an object keeps, and the number of a player

import random
import struct

from difftest import Buffer

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
PLAYER = 0x21C


def run(t) -> None:
    random_ = random.Random(829)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    def block(keys: list[int], size: int, at: int) -> tuple[bytes, list[int]]:
        """A block at `at`: how many, a pointer to elements of `size` bytes after it; where its pointer is."""
        data = struct.pack("<iI", len(keys), at + 8)
        for key in keys:
            data += struct.pack("<i", key) + bytes(size - 4)
        return data, [at + 4]

    for n in range(400):
        # three levels: a block of two (0x14), each of whose first has a block of two (0x14), whose first has a block of two (0x34)
        data = bytearray(random_.randbytes(0x34))
        pointers = []
        top, place = block([1, 2], 0x14, 0x34)
        data[0x34:0x3C] = top[:8]
        # put the block itself at 0x34 of something (its count and pointer)
        data = bytearray(random_.randbytes(0x34)) + top
        second_at = len(data)
        middle, middle_pointer = block([3, 4], 0x14, second_at)
        third_at = second_at + len(middle)
        last, last_pointer = block([5, 6], 0x34, third_at)
        data += middle + last
        # the element with key 1 holds the second block (from its 0x04); that with key 3 the third
        struct.pack_into("<iI", data, 0x34 + 8 + 4, 2, second_at + 8)
        struct.pack_into("<iI", data, second_at + 8 + 4, 2, third_at + 8)
        pointers = [0x34 + 4, 0x34 + 8 + 8, second_at + 4, second_at + 8 + 8, third_at + 4]
        t.check(
            "havok_unknown_001db120",
            Buffer(bytes(data), pointers=tuple(pointers)),
            random_.choice((1, 1, 2, 9)),
            random_.choice((3, 3, 4, 9)),
            random_.choice((5, 6, 9)),
        )

        # locations, emitters and particles: the location's emitters are mocked away by datum_delete
        locations = bytearray(random_.randbytes(0x50 + 3 * 0x34))
        struct.pack_into("<I", locations, 0x44, 0x50)
        emitters = bytearray(random_.randbytes(0x50 + 4 * 0x4C))
        struct.pack_into("<I", emitters, 0x44, 0x50)
        particles = bytearray(random_.randbytes(0x50 + 2 * 0x40))
        struct.pack_into("<I", particles, 0x44, 0x50)
        order = random_.sample(range(4), random_.randrange(4))
        for place_, emitter in enumerate(order):
            at = 0x50 + emitter * 0x4C
            struct.pack_into("<I", emitters, at + 8, salted(order[place_ + 1]) if place_ + 1 < len(order) else NONE)
            struct.pack_into("<I", emitters, at + 4, NONE)
        location = random_.randrange(3)
        struct.pack_into("<I", locations, 0x50 + location * 0x34 + 4, salted(order[0]) if order else NONE)
        t.check(
            "particle_location_delete",
            salted(location),
            globals={
                "particle_location_data": Buffer(bytes(locations), pointers=(0x44,)),
                "particle_emitter_data": Buffer(bytes(emitters), pointers=(0x44,)),
                "particle_data": Buffer(bytes(particles), pointers=(0x44,)),
            },
            externals={"datum_delete": lambda harness, arguments: (0, tuple(arguments))},
        )

        table = some_objects(random_)
        objects = bytearray(table.data) + bytearray(random_.randbytes(0x80))
        for index in range(COUNT):
            at = FIRST + index * OBJECT
            struct.pack_into("<h", objects, at + 0x12A, random_.choice((0, 0x10, 0x100)))
            offset = struct.unpack_from("<h", objects, at + 0x12A)[0]
            struct.pack_into("<I", objects, at + offset + 0x7C, random_.choice((0x080000C4, 0x0400004A, 0x050000C3, 7)))
        told = random_.random() < 0.3
        t.check(
            "unit_unknown_000c70b0",
            salted(random_.randrange(COUNT)),
            globals={"object_header_data": Buffer(bytes(objects), pointers=table.pointers)},
            externals={"unit_unknown_000c7070": lambda harness, arguments, told=told: (int(told), tuple(arguments))},
        )

        table = some_objects(random_)
        objects = bytearray(table.data)
        for index in range(COUNT):
            struct.pack_into("<I", objects, FIRST + index * OBJECT, salted(0))
        tags = bytearray(random_.randbytes(0x10 + 0x40))
        struct.pack_into("<I", tags, 8, 0x10)
        found = random_.choice((NONE, 0, 3))
        t.check(
            "object_unknown_000ba540",
            salted(random_.randrange(COUNT)),
            random_.choice((0, 0x1234, 5)),
            globals={"object_header_data": Buffer(bytes(objects), pointers=table.pointers), "global_tag_instances": Buffer(bytes(tags), pointers=(8,))},
            externals={"tag_unknown_0016d180": lambda harness, arguments, found=found: (found, tuple(arguments))},
        )

        # players with numbers (0x24), and the players globals for them
        players = bytearray(random_.randbytes(0x50 + 4 * PLAYER))
        struct.pack_into("<I", players, 0x44, 0x50)
        globals_ = bytearray(random_.randbytes(0x30))
        struct.pack_into("<h", globals_, 0x0A, random_.randrange(5))
        numbers = random_.sample([0, 1, 2, 3, NONE, NONE, NONE, NONE], 4)
        for player in range(4):
            struct.pack_into("<I", players, 0x50 + player * PLAYER + 0x24, numbers[player])
        t.check(
            "player_unknown_0014f270",
            salted(random_.randrange(4)),
            random_.choice((NONE, 0, 1, 2, 3)),
            globals={"player_data": Buffer(bytes(players), pointers=(0x44,)), "players_globals": Buffer(bytes(globals_))},
        )
