# six small functions of what the interface asks of the network game and of
# its one of the list of games

import random
import struct

from difftest import Buffer
from test_network_hoppers import hopper

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(761)

    def says(*values):
        return lambda harness, arguments, said=random_.choice(values): (said, tuple(arguments))

    for n in range(600):
        t.check(
            "user_interface_networking_unknown_00199df9",
            random_.choice((0, 1)),
            random_.choice((0, 1)),
            externals={
                "user_interface_networking_unknown_00199e2e": says(0),
                "network_life_cycle_unknown_000591e0": says(0, 1),
                "network_life_cycle_unknown_000591f0": says(0, 1),
                "network_life_cycle_unknown_00059890": says(0, 1),
            },
        )

        one = bytearray(hopper(random_))
        struct.pack_into("<I", one, 0, random_.choice((0, 1, 2, 3, 4, 5, 5, 6, NONE)))
        found = Buffer(bytes(one))
        index = random_.choice((NONE, 0, 3, 15))
        externals = {
            "network_life_cycle_session_get_unknown49C8": lambda harness, arguments, index=index: (index, ()),
            "network_hoppers_get": lambda harness, arguments, there=random_.choice((0, 1, 1)): (found.address if there else 0, tuple(arguments)),
        }
        for name, predicate in (
            ("user_interface_networking_hopper_unknown_00193470", "network_hopper_unknown_00193470"),
            ("user_interface_networking_hopper_unknown_00193490", "network_hopper_unknown_00193490"),
            ("user_interface_networking_hopper_unknown_001934b0", "network_hopper_unknown_001934b0"),
        ):
            t.check(
                name,
                extra=[found],
                externals={
                    **externals,
                    predicate: lambda harness, arguments, said=random_.choice((0, 1, 0x100)): (said, (arguments[0] == found.address,)),
                },
            )
        t.check("user_interface_networking_hopper_is_type5", extra=[found], externals=externals)

        t.check(
            "user_interface_networking_unknown_0019a250",
            globals={"unknown_004d8ba0": bytes((random_.choice((0, 1, 1, 1)),))},
            externals={"network_life_cycle_get_state": says(*range(11), NONE, 0x80000000)},
        )
