# a new light

import random
import struct

from difftest import Buffer

from test_data import array

SIZE = 0x110
NONE = 0xFFFFFFFF
TAGS, DEFINITION_SIZE = 6, 0xB0


def run(t) -> None:
    random_ = random.Random(640)

    for n in range(400):
        # the tags, and after them the definition of the light
        definition_index = random_.randrange(TAGS)
        tags = bytearray(random_.randbytes(TAGS * 16 + DEFINITION_SIZE))
        struct.pack_into("<I", tags, definition_index * 16 + 8, TAGS * 16)
        definition = TAGS * 16
        tags[definition] = random_.choice((0, 1, 1, random_.randrange(0x100)))
        struct.pack_into("<I", tags, definition + 0x94, random_.choice((NONE, NONE, random_.getrandbits(32))))
        struct.pack_into("<h", tags, definition + 0xA4, random_.choice((0, 1, 2, 2, 3)))

        game = bytearray(random_.randbytes(0x1200))
        struct.pack_into("<i", game, 8, random_.choice((0, 1, 2, 3, 4, 5, 6)))

        maximum_count = random_.choice((0x15E, 40, 4, 1))
        count = random_.choice((0, maximum_count, random_.randint(0, maximum_count)))
        in_use = {index: random_.randint(-0x8000, 0x7FFF) or 1 for index in random_.sample(range(maximum_count), count)}

        t.check(
            "light_new",
            definition_index | random_.getrandbits(16) << 16,
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            globals={
                "global_tag_instances": Buffer(bytes(tags), pointers=(definition_index * 16 + 8,)),
                "main_game_globals": Buffer(bytes(game)),
                "light_data": array(random_, maximum_count, SIZE, in_use),
                "unknown_004e0308": random_.getrandbits(32),
            },
            externals={"unknown_000c0c80": lambda harness, arguments: 0},
        )
