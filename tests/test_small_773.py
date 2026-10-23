# five small functions: a number of the results if there is something to
# read, something of a level, clearing and ending the task of something, the
# name of a game, and memory of the pool of the interface

import random

from difftest import Buffer

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(773)

    def says(*values):
        return lambda harness, arguments, said=random_.choice(values): (said, tuple(arguments))

    for n in range(600):
        number = lambda: random_.choice((NONE, 0, 1, 3, 15, random_.getrandbits(32)))
        t.check(
            "game_results_get_statistic_if_readable",
            number(),
            number(),
            number(),
            globals={
                "unknown_00510cb0": bytes((random_.choice((0, 1, 1, 0x80)),)),
                "unknown_00510cb1": bytes((random_.choice((0, 1, 1, 0x80)),)),
            },
            externals={"game_results_get_statistic": says(NONE, 0, 7, 0x7FFF)},
        )

        level = Buffer(random_.randbytes(0x10))

        def found(harness, arguments, there=random_.choice((0, 1, 1))):
            return (level.address if there else 0), tuple(arguments)

        t.check(
            "levels_unknown_0019c970",
            random_.choice((NONE, NONE, 0, 1, 3, 0x80000000)),
            random_.choice((NONE, 0, 1, 5, 30, random_.getrandbits(32))),
            extra=[level],
            externals={"levels_get_multiplayer_level": found, "unknown_0019c270": found},
        )

        data = Buffer(random_.randbytes(0x624))
        t.check(
            "online_unknown_001a2f70_clear_and_end_task",
            data,
            externals={"online_unknown_001a2f70_end_task": lambda harness, arguments: (0, (arguments[0] == data.address,))},
        )

        name = Buffer(random_.randbytes(0x40))
        t.check(
            "online_unknown_001a2f70_get_title_name",
            name,
            random_.choice((0, 0, 1, 0x4D530064, random_.getrandbits(32))),
            random_.choice((0, 1, 0x20, 0x80)),
            externals={
                "online_title_get_name": lambda harness, arguments: (0, (arguments[0], arguments[1] == name.address, arguments[2])),
            },
        )

        t.check(
            "user_interface_memory_allocate_simple",
            random_.choice((0, 1, 0x100, 0x10000, random_.getrandbits(32))),
            externals={"user_interface_memory_allocate": says(0, 0x12345678)},
        )
