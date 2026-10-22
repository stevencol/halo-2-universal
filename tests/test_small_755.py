# six small functions of the network game and its session: setting two things
# of it, asking it for three and doing something with a profile

import random
import struct

from difftest import Buffer, GlobalData

NONE = 0xFFFFFFFF
SESSION = 0x7420
STATES = (0, 1, 2, 3, 4, 8, 9, NONE, 0x80000000)
PROFILES = 4
PROFILE = 0xD0


def session_data(random_: random.Random) -> bytearray:
    data = bytearray(random_.randbytes(SESSION))
    struct.pack_into("<I", data, 0x741C, random_.choice(STATES))
    if random_.random() < 0.5:
        data[0x72D8:0x72DC] = data[0x50:0x54]
    data[0x24] = random_.choice((0, 1, 1))
    return data


def says(random_: random.Random):
    return lambda harness, arguments, said=random_.choice((0, 1, 1, 0x100)): (said, (harness.normalize(arguments[0]), *arguments[1:]))


def run(t) -> None:
    random_ = random.Random(755)

    for n in range(300):
        # with the network game and its session as globals
        session = session_data(random_)
        struct.pack_into("<I", session, 0x4980, random_.choice((1, 1, 0, 2)))
        common = {
            "unknown_00527330": bytes((random_.choice((0, 1, 1, 1)),)),
            "unknown_00527334": random_.choice((3, 3, 2, 4, 0)),
            "network_life_cycle_session": Buffer(bytes(session)),
        }
        t.check(
            "network_life_cycle_set_unknown5dd0",
            random_.getrandbits(16),
            globals=common,
            externals={
                "network_session_leader_set_unknown5dd0": lambda harness, arguments, said=random_.choice((0, 1, 1)): (
                    said,
                    (harness.normalize(arguments[0]), arguments[1] & 0xFFFF),
                ),
                "network_session_unknown_0005bf10": lambda harness, arguments, said=random_.choice((0, 1, 1)): (
                    said,
                    (harness.normalize(arguments[0]), arguments[1] & 0xFF, *arguments[2:]),
                ),
            },
        )

        t.check(
            "network_life_cycle_set_unknown49c4",
            globals=common,
            externals={"network_session_leader_set_unknown49c4": says(random_)},
        )

        profiles = bytearray(random_.randbytes(PROFILES * PROFILE))
        for index in range(PROFILES):
            profiles[index * PROFILE] = random_.choice((0, 1, 1))
        t.check(
            "network_life_cycle_unknown_00064900",
            random_.randrange(PROFILES),
            random_.choice((0, 1)),
            random_.getrandbits(32),
            random_.getrandbits(32),
            globals={**common, "unknown_004cd904": GlobalData(bytes(profiles))},
            externals={
                "network_session_unknown_0005bf10": lambda harness, arguments, said=random_.choice((0, 1, 1, 0x100)): (
                    said,
                    (
                        harness.normalize(arguments[0]),
                        arguments[1] & 0xFF,
                        arguments[2],
                        arguments[3],
                        arguments[4] - harness.global_addresses["unknown_004cd904"],
                    ),
                ),
            },
        )

        # with the session that the state of the network game gives
        found = Buffer(bytes(session_data(random_)))

        def state_session(harness, arguments, there=random_.choice((0, 1, 1, 1))):
            if there:
                harness.mu.mem_write(arguments[0], struct.pack("<I", found.address))
            return there, ()

        t.check(
            "network_life_cycle_get_unknown4d08",
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
            extra=[found],
            externals={
                "network_life_cycle_get_state_session": state_session,
                "network_session_get_unknown4d08": lambda harness, arguments, said=random_.choice((0, 1, 1)): (
                    said,
                    (arguments[0] == found.address, *(harness.normalize(argument) for argument in arguments[1:])),
                ),
            },
        )
        t.check(
            "network_life_cycle_get_unknown5dd0",
            extra=[found],
            externals={"network_life_cycle_get_state_session": state_session},
        )
        t.check(
            "network_life_cycle_get_identifier_and_key",
            random_.choice((Buffer(random_.randbytes(8)), 0)),
            random_.choice((Buffer(random_.randbytes(0x10)), 0)),
            extra=[found],
            externals={"network_life_cycle_get_state_session": state_session},
        )
