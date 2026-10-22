# six small functions of what the interface makes of the network game

import random

from difftest import Buffer

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(760)

    def says(*values):
        return lambda harness, arguments, said=random_.choice(values): (said, tuple(arguments))

    for n in range(600):
        t.check(
            "user_interface_networking_unknown_00199d7c",
            externals={
                "network_life_cycle_unknown_000594f0": says(0, 1, 2, 3, 3, 3, 4, 5, NONE),
                "network_life_cycle_unknown_00059570": says(0, 1, 2, 3, 4, 5, 6, NONE),
            },
        )
        t.check(
            "user_interface_networking_unknown_00199e6d",
            random_.getrandbits(32),
            externals={"network_life_cycle_unknown_00063e90": says(0, 0, 1, 2, NONE, 0x80000000)},
        )
        unknown = Buffer(bytes((random_.choice((0, 1, 2, 0x80, 0xFF)),)))
        t.check(
            "user_interface_networking_get_unknown49A1",
            extra=[unknown],
            externals={
                "network_life_cycle_session_get_unknown49A1": lambda harness, arguments, there=random_.choice((0, 1, 1)): (
                    unknown.address if there else 0,
                    (),
                ),
            },
        )
        t.check(
            "user_interface_networking_get_unknown498C",
            externals={"network_life_cycle_session_get_unknown498C": says(0, 1, 2, 3, NONE, 0x80000000)},
        )
        t.check(
            "user_interface_networking_unknown49B0_is_2",
            externals={"network_life_cycle_session_get_unknown49B0": says(0, 1, 2, 2, 3, 0x102, NONE)},
        )
        t.check(
            "user_interface_networking_unknown18_is_2",
            externals={"network_life_cycle_session_get_unknown18": says(0, 1, 2, 2, 3, 0x102, NONE)},
        )
