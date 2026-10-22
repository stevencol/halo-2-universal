# three functions of the teams that the matchmaking makes even: trying to swap
# a group of one team for one of another, a pass over all the pairs of teams,
# and passes until no swap is better

import random
import struct

from difftest import Buffer
from test_small_750 import GROUP, TEAM, TOTAL, group, players

TEAMS = 4 + 4 * TEAM
IMPROVEMENT = TEAMS + 4 + TEAMS


def team(random_: random.Random, values: tuple, identifiers: set) -> bytes:
    """A team of 1 to 5 groups and up to 16 players whose groups and group of
    all the players agree; the identifiers of players and groups are not those
    of another team."""
    groups = []
    total = []
    for _ in range(random_.choice((1, 2, 2, 3, 3, 5))):
        size = min(random_.choice((1, 1, 2, 3, 4)), 16 - len(total))
        members = players(random_, size, values, identifiers)
        total += members
        identifier = random_.getrandbits(32)
        while identifier in identifiers:
            identifier = random_.getrandbits(32)
        identifiers.add(identifier)
        bytes_ = bytearray(group(random_, members, identifier))
        # the byte that two groups have to share to be swapped
        bytes_[0] = random_.choice((0, 0, 0, 1))
        groups.append(bytes(bytes_))
    data = bytearray(random_.randbytes(TEAM))
    struct.pack_into("<ii", data, 0, len(groups), random_.randrange(-2, 9))
    for index, bytes_ in enumerate(groups):
        data[8 + index * GROUP : 8 + (index + 1) * GROUP] = bytes_
    random_.shuffle(total)
    data[TOTAL : TOTAL + GROUP] = group(random_, total, random_.getrandbits(32))
    return bytes(data)


def teams(random_: random.Random, count: int, values: tuple) -> bytes:
    identifiers: set = set()
    data = bytearray(random_.randbytes(TEAMS))
    struct.pack_into("<i", data, 0, count)
    for index in range(4):
        data[4 + index * TEAM : 4 + (index + 1) * TEAM] = team(random_, values, identifiers)
    return bytes(data)


def run(t) -> None:
    random_ = random.Random(752)

    for n in range(200):
        values = random_.choice(((0, 1, 2, 3), (5, 5, 6), (-3, 0, 7, 100), (1, 10, 100, 1000)))
        count = random_.choice((2, 2, 3, 4))
        improved = random_.choice((0, 0, 1))
        improvement = teams(random_, count, values) + bytes((improved,)) + random_.randbytes(3) + teams(random_, count, values)

        first = random_.randrange(count - 1)
        second = random_.randrange(first + 1, count)
        t.check("network_team_balance_improvement_try_swaps", first, second, Buffer(improvement))

        t.check("network_team_balance_improvement_pass", Buffer(improvement))

        t.check(
            "network_team_balance_improve",
            Buffer(random_.randbytes(IMPROVEMENT)),
            Buffer(teams(random_, random_.choice((0, 1, 2, 2, 3, 4)), values)),
        )
