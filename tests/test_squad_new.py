# a squad of the scenario gets its squad of the game

import random
import struct

from difftest import Buffer

from test_data import array

NONE = 0xFFFFFFFF
TAGS, TAG_SIZE = 8, 0x100
SCENARIO, PALETTE = 0x200, 6


def tags(random_) -> Buffer:
    """Some tags, each after the table; a tag comes from a later one, or from none."""
    data = bytearray(random_.randbytes(TAGS * 16 + TAGS * TAG_SIZE + 0x40))
    pointers = []
    for index in range(TAGS):
        at = TAGS * 16 + index * TAG_SIZE
        struct.pack_into("<I", data, index * 16 + 8, at)
        pointers.append(index * 16 + 8)

        def one(low: int = 0) -> int:
            return NONE if low >= TAGS or random_.random() < 0.4 else random_.getrandbits(16) << 16 | random_.randrange(low, TAGS)

        struct.pack_into("<I", data, at + 8, one(index + 1))
        struct.pack_into("<I", data, at + 0x10, one())
        struct.pack_into("<I", data, at + 0x18, one())
        # the block at 0x34: its first thing is in the last bytes of the tag
        struct.pack_into("<iI", data, at + 0x34, random_.choice((0, 0, 1, 3, -1)), at + 0xE0)
        pointers.append(at + 0x38)
        struct.pack_into("<h", data, at + 0xE4, random_.choice((0, 6, 0x13, 0x14, -1, random_.randrange(0x14))))
        data[at + 0xC0] = random_.choice((0, 0, 1, 5, 0xFF))
    return Buffer(bytes(data), pointers=tuple(pointers))


def run(t) -> None:
    random_ = random.Random(674)

    for n in range(300):
        t.check(
            "character_definition_get_unknown34",
            random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(TAGS), random_.randrange(TAGS))),
            globals={"global_tag_instances": tags(random_)},
        )

    for n in range(400):
        scenario = bytearray(random_.randbytes(SCENARIO + PALETTE * 8))
        struct.pack_into("<iI", scenario, 0x178, random_.choice((PALETTE, PALETTE, 0, 2)), SCENARIO)
        for index in range(PALETTE):
            struct.pack_into(
                "<I",
                scenario,
                SCENARIO + index * 8 + 4,
                random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(TAGS), random_.randrange(TAGS))),
            )
        squad = bytearray(random_.randbytes(0x74))
        struct.pack_into("<I", squad, 0x20, random_.choice((0, 0x100, 0x200, 0x300, random_.getrandbits(32))))
        squad[0x24] = random_.choice((0, 0, 0, 2, 0xFF))
        struct.pack_into("<h", squad, 0x36, random_.choice((-1, 0, PALETTE - 1, PALETTE, random_.randrange(PALETTE))))

        in_use = {index: random_.randint(1, 0x7FFF) for index in random_.sample(range(0x14F), random_.randint(0, 20))}
        t.check(
            "squad_new_from_scenario",
            Buffer(bytes(squad)),
            globals={
                "squad_data": array(random_, 0x14F, 0x98, in_use),
                "global_scenario": Buffer(bytes(scenario), pointers=(0x17C,)),
                "global_tag_instances": tags(random_),
            },
        )
