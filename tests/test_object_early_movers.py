# source/objects/object_early_movers.c

import random
import struct

from difftest import PHYSICAL_ADDRESS, Buffer, Memory

from test_game_state import game_state_globals
from test_units import COUNT, OBJECT, some_objects

FIRST = 0x4C + COUNT * 12
SIZE = 0x1D88
BASE = PHYSICAL_ADDRESS + 0x60000


def run(t) -> None:
    random_ = random.Random(6031)

    for n in range(60):
        allocated = random_.choice((0, 0x40, 0x1288))
        t.check(
            "object_early_movers_initialize",
            globals={
                "game_state_globals": game_state_globals(random_, base=BASE, cpu_allocation_size=allocated),
                "object_early_movers_globals": random_.getrandbits(32),
            },
            memory={BASE + allocated: Memory(random_.randbytes(SIZE + 0x10))},
        )
        t.check("object_early_movers_dispose", globals={"object_early_movers_globals": random_.getrandbits(32)})
        t.check("object_early_movers_initialize_for_new_map", globals={"object_early_movers_globals": Buffer(random_.randbytes(SIZE + 0x10))})
        t.check("object_early_movers_dispose_from_old_map", globals={"object_early_movers_globals": Buffer(random_.randbytes(SIZE + 0x10))})

    for n in range(1500):
        # the objects, with the tags after them: each object starts with its tag, and a tag has where what it is is
        table = some_objects(random_)
        data = bytearray(table.data)
        tags = bytearray(random_.randbytes(4 * 16))
        definitions = bytearray(random_.randbytes(4 * 8))
        tags_at = len(data)
        definitions_at = tags_at + len(tags)
        pointers = list(table.pointers)
        for index in range(4):
            struct.pack_into("<I", tags, index * 16 + 8, definitions_at + index * 8)
            pointers.append(tags_at + index * 16 + 8)
            definitions[index * 8 + 2] = random_.choice((0, 0x40, 0x40, 0xBF, 0xFF))
        for index in range(COUNT):
            struct.pack_into("<I", data, FIRST + index * OBJECT, random_.getrandbits(16) << 16 | random_.randrange(4))
        everything = Buffer(bytes(data) + bytes(tags) + bytes(definitions), pointers=tuple(pointers))
        movers = bytearray(random_.randbytes(SIZE))
        struct.pack_into("<i", movers, 0x1D80, random_.choice((0, 1, 31, 31, 32, 33)))

        # the tags are in the same buffer as the objects: the global is given where they are in it
        from difftest import SCRATCH_ADDRESS

        t.check(
            "object_early_movers_add",
            random_.getrandbits(16) << 16 | random_.randrange(COUNT),
            globals={
                "object_header_data": everything,
                "global_tag_instances": SCRATCH_ADDRESS + tags_at,
                "object_early_movers_globals": Buffer(bytes(movers)),
            },
        )
