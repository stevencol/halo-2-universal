# six small functions of the teams that the matchmaking makes even: a group of
# players, and how far apart the teams are

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
GROUP = 0x90
TEAM = 0x998
TOTAL = 8 + 16 * GROUP
TEAMS = 4 + 4 * TEAM


def group(random_: random.Random, count: int, values: tuple) -> bytearray:
    """A group of 0x90 bytes with so many players, from the highest number."""
    data = bytearray(random_.randbytes(GROUP))
    skills = sorted((random_.choice(values) for _ in range(count)), reverse=True)
    total = 0
    for index, skill in enumerate(skills):
        struct.pack_into("<iI", data, 0x10 + index * 8, skill, random_.getrandbits(32))
        total += skill
    struct.pack_into("<iI", data, 8, count, total & 0xFFFFFFFF)
    return data


def teams(random_: random.Random, count: int, players: tuple, skills: tuple) -> bytes:
    """Up to four teams, with the count and the sum of each in its group of all."""
    data = bytearray(random_.randbytes(TEAMS))
    struct.pack_into("<i", data, 0, count)
    for index in range(4):
        struct.pack_into("<ii", data, 4 + index * TEAM + TOTAL + 8, random_.choice(players), random_.choice(skills))
    return bytes(data)


def run(t) -> None:
    random_ = random.Random(749)

    for n in range(300):
        t.check("network_team_balance_group_reset", Buffer(random_.randbytes(GROUP)))

        values = random_.choice(((0, 1, 2, 3), (5, 5, 6), (-3, 0, 7, 100), (-0x80000000, 0x7FFFFFFF, 1)))
        count = random_.choice((0, 1, 2, 5, 15, random_.randrange(16)))
        t.check(
            "network_team_balance_group_add_player",
            Buffer(bytes(group(random_, count, values))),
            Buffer(struct.pack("<iI", random_.choice(values), random_.getrandbits(32))),
        )

        count = random_.choice((0, 1, 2, 3, 4, 4, -1, -0x80000000))
        t.check("network_team_balance_reset", Buffer(random_.randbytes(TEAMS)), count & 0xFFFFFFFF)

        count = random_.choice((0, 1, 2, 3, 4, 4, 4, -1))
        players = random_.choice(((0, 1, 2), (3, 3, 4), (0, 8, 16)))
        skills = random_.choice(((0, 1, 2), (10, 10, 11), (-5, 0, 5, 1000), (-0x80000000, 0x7FFFFFFF)))
        t.check("network_team_balance_skill_difference", Buffer(teams(random_, count, players, skills)))
        t.check("network_team_balance_player_difference", Buffer(teams(random_, count, skills, players)))
        t.check("network_team_balance_smallest_team", Buffer(teams(random_, count, players, skills)))
