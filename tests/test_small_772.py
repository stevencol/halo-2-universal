# five small functions: noting when the entry of this console is marked
# among the sixteen of 0x0051e8d4, setting a byte of the marked one, and
# three that the interface asks of the network game

import random

from difftest import Buffer, GlobalRange

NONE = 0xFFFFFFFF
ENTRIES, ENTRY = 16, 0xB


def run(t) -> None:
    random_ = random.Random(772)

    def says(*values):
        return lambda harness, arguments, said=random_.choice(values): (said, tuple(harness.normalize(a) for a in arguments))

    for n in range(600):
        # the entries are at 0xDC24 of unknown_00510cb0: only that the
        # pointer is to it is compared
        ranges = {"unknown_00510cb0": GlobalRange(0xDC24), "unknown_004cf7cc": GlobalRange(6)}
        flags = {
            "unknown_00511020": bytes((random_.choice((0, 1, 0x80)),)),
            "unknown_00511024": random_.getrandbits(32),
            "unknown_00511028": bytes((random_.choice((0, 1, 0x80)),)),
            "unknown_0051102c": random_.getrandbits(32),
        }
        index = (NONE, 0, 1, 7, 15, random_.randrange(16))
        t.check(
            "game_unknown_001991f0_mark_own",
            globals={**ranges, **flags},
            externals={
                "game_unknown_001991f0_find_marked": says(*index, NONE, NONE),
                "game_unknown_001991f0_find": says(*index),
                "game_unknown_001991f0_mark": says(0),
                "time": says(0, 1, 0x5F000000, random_.getrandbits(32)),
            },
        )
        t.check(
            "game_unknown_001991f0_unknown_00199460",
            globals={**ranges, **flags, "unknown_0051e8d4": random_.randbytes(ENTRIES * ENTRY)},
            externals={
                "game_unknown_001991f0_find_marked": says(*index),
                "time": says(0, 1, 0x5F000000, random_.getrandbits(32)),
            },
        )

        t.check(
            "user_interface_networking_unknown_001999b3",
            externals={
                "unknown_0019989d": says(0, 1, 2, 6, NONE, random_.getrandbits(32)),
                "user_interface_networking_unknown_0019999e": says(0, 1, 0x100),
            },
        )

        session = Buffer(random_.randbytes(0x10))
        identifier = Buffer(random_.randbytes(0xC))

        def session_of(harness, arguments, there=random_.choice((0, 1, 1, 1)), what=random_.choice((session, None))):
            if what is not None:
                harness.mu.mem_write(arguments[0], what.address.to_bytes(4, "little"))
            return there, ()

        def find(harness, arguments, said=random_.choice((NONE, 0, 3, 15))):
            return said, (arguments[0] == session.address, arguments[0] == 0, arguments[1] == identifier.address)

        for name, getter in (
            ("user_interface_networking_find_state_player", "network_life_cycle_get_state_session"),
            ("user_interface_networking_find_session_player", "network_life_cycle_get_session"),
        ):
            t.check(
                name,
                random_.choice((identifier, identifier, 0)),
                extra=[session],
                externals={getter: session_of, "user_interface_networking_find_player": find},
            )
