# four small functions of the list of games of the service: the numbers for
# each number of players, its file, and one of it by its number at 8

import random
import struct

from difftest import Buffer, GlobalData
from test_network_hoppers import COUNT, SIZE, hopper
from test_small_757 import blob

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(758)

    for n in range(400):
        minimum = random_.choice((-2, 0, 1, 2, 3, 8, 15, 16, 17, random_.randint(-2, 20)))
        maximum = random_.choice((minimum, minimum + 1, minimum + 5, 1, 4, 16, 20, random_.randint(-2, 20)))
        scale = random_.choice((0, 1, 100, 1000, 0x7FFF, -50, random_.getrandbits(16)))
        t.check(
            "network_hopper_make_players_values",
            scale & 0xFFFFFFFF,
            minimum & 0xFFFFFFFF,
            maximum & 0xFFFFFFFF,
            Buffer(random_.randbytes(16 * 4)),
        )

        team_size = random_.choice((0, 1, 2, 3, 4, 8, -1, random_.randint(-2, 9)))
        minimum_teams = random_.choice((0, 1, 2, 3, 4, random_.randint(-1, 8)))
        maximum_teams = random_.choice((minimum_teams, minimum_teams + 1, minimum_teams + 3, 2, 4, 8, random_.randint(-1, 16)))
        t.check(
            "network_hopper_make_team_values",
            Buffer(random_.randbytes(16 * 4)),
            team_size & 0xFFFFFFFF,
            maximum_teams & 0xFFFFFFFF,
            minimum_teams & 0xFFFFFFFF,
            scale & 0xFFFFFFFF,
        )

        file = bytearray(random_.randbytes(0x110))
        t.check("network_hoppers_get_file", Buffer(bytes(file)))

        table = bytearray()
        numbers = []
        for _ in range(COUNT):
            one = hopper(random_)
            struct.pack_into("<I", one, 8, random_.choice((1, 2, 5, 100, random_.getrandbits(32))))
            numbers.append(struct.unpack_from("<I", one, 8)[0])
            table += one
        t.check(
            "network_hoppers_find_unknown8",
            random_.choice(numbers + [random_.getrandbits(32)]),
            Buffer(random_.randbytes(4)),
            globals={"unknown_0047d8f4": blob(random_), "network_hoppers": GlobalData(bytes(table))},
            externals={
                "network_hopper_valid": lambda harness, arguments, so=random_.choice((0, 1, 1, 1)): (
                    so,
                    ((arguments[0] - harness.global_addresses["network_hoppers"]) // SIZE,),
                ),
            },
        )
