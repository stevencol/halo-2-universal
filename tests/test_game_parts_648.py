# the part of the game with two things that share memory of the console

import random

THINGS = ("unknown_00547f88", "unknown_0054aa68")


def which(harness, address: int) -> object:
    """Which of the two things an address is, or the address."""
    for name in THINGS:
        if address == harness.global_addresses[name]:
            return name
    return address


def run(t) -> None:
    random_ = random.Random(648)

    def made(harness, arguments):
        # the six shorts are on the stack, or wherever source/ has them
        return 0, (bytes(harness.mu.mem_read(arguments[0], 12)), arguments[1], which(harness, arguments[2]))

    def closed(harness, arguments):
        return 0, (which(harness, arguments[0]),)

    for n in range(100):
        memory = random_.choice((0, 0x70000000, random_.getrandbits(28) << 4))
        t.check(
            "game_unknown_00131ff0_initialize",
            globals={"unknown_00510c44": random_.getrandbits(32)},
            externals={
                "VirtualAlloc": lambda harness, arguments, memory=memory: memory,
                "GetLastError": lambda harness, arguments: 8,
                "unknown_00134980": made,
            },
        )
        freed = random_.choice((0, 1, 1))
        t.check(
            "game_unknown_00131ff0_dispose",
            globals={"unknown_00510c44": memory},
            externals={
                "unknown_00134b20": closed,
                "VirtualFree": lambda harness, arguments, freed=freed: freed,
                "GetLastError": lambda harness, arguments: 5,
            },
        )
