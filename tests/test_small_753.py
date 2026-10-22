# three functions of the teams that the matchmaking makes even: putting so
# many players (each with its group and its number) in teams, how far apart
# the teams are in players, and whether some players fit in the teams

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF


def longs(values) -> bytes:
    return struct.pack(f"<{len(values)}I", *(value & 0xFFFFFFFF for value in values))


def players(random_: random.Random, count: int) -> list:
    """The group of each player: a few groups, from 0, with no gaps."""
    group_count = random_.randint(1, max(1, min(count, 6)))
    groups = [random_.randrange(group_count) for _ in range(count)]
    return groups


def run(t) -> None:
    random_ = random.Random(753)

    for n in range(150):
        count = random_.choice((0, 1, 2, 4, 6, 8, 12, 16, random_.randint(0, 16)))
        grouped = random_.choice((0, 1, 1))
        groups = players(random_, count) if grouped else list(range(count))
        values = random_.choice(((0, 1, 2, 3), (5, 5, 6), (1, 10, 100, 1000), (0,)))
        skills = [random_.choice(values) for _ in range(count)]
        team_count = random_.choice((1, 2, 2, 3, 4))
        maximum = random_.choice((1, 2, 4, 8, 16, 0, -1))
        minimum_group = random_.choice((0, 1, 2, 3))
        maximum_group = random_.choice((1, 2, 4, 16))
        mark = random_.choice((0, 1))
        t.check(
            "network_team_balance_assign",
            team_count,
            maximum & 0xFFFFFFFF,
            mark,
            minimum_group,
            maximum_group,
            grouped,
            count,
            Buffer(longs(groups + [random_.randrange(16) for _ in range(16 - count)])),
            Buffer(longs(skills + [random_.choice(values) for _ in range(16 - count)])),
            random_.choice((0, 1, 1)),
            Buffer(random_.randbytes(16 * 4)),
        )

        t.check(
            "network_team_balance_assign_difference",
            count,
            Buffer(longs(groups + [0] * (16 - count))),
            team_count,
            maximum & 0xFFFFFFFF,
            mark,
            minimum_group,
            maximum_group,
            grouped,
            Buffer(random_.randbytes(4)),
        )

        # groups of one and of two players, those of two marked (exactly as
        # many as the fewest that are marked), with swaps: the marked ones are
        # not swapped for the others
        sizes = [random_.choice((1, 2)) for _ in range(random_.randint(3, 8))]
        groups = [index for index, size in enumerate(sizes) for _ in range(size)][:16]
        count = len(groups)
        t.check(
            "network_team_balance_assign",
            random_.choice((2, 2, 3)),
            16,
            1,
            2,
            16,
            1,
            count,
            Buffer(longs(groups + [0] * (16 - count))),
            Buffer(longs([random_.choice((1, 10, 100, 1000)) for _ in range(16)])),
            1,
            Buffer(random_.randbytes(16 * 4)),
        )

        # the same with no marks and no swaps asked for, where swaps would
        # make the teams better
        sizes = [random_.choice((1, 2)) for _ in range(random_.randint(3, 8))]
        groups = [index for index, size in enumerate(sizes) for _ in range(size)][:16]
        count = len(groups)
        t.check(
            "network_team_balance_assign",
            random_.choice((2, 2, 3)),
            16,
            0,
            0,
            16,
            1,
            count,
            Buffer(longs(groups + [0] * (16 - count))),
            Buffer(longs([random_.choice((1, 10, 100, 1000)) for _ in range(16)])),
            0,
            Buffer(random_.randbytes(16 * 4)),
        )

        # players in groups, and others that go in a group of their own
        grouped_count = random_.randint(0, 12)
        other_count = random_.randint(0, 16 - grouped_count)
        t.check(
            "network_team_balance_fits",
            grouped_count,
            other_count,
            Buffer(longs(players(random_, grouped_count) + [random_.randrange(16) for _ in range(16 - grouped_count)])),
            team_count,
            random_.choice((1, 2, 4, 8, 16, 0)),
            mark,
            minimum_group,
            maximum_group,
            random_.choice((0, 1, 1)),
            random_.choice((0, 1, 2, 3, 8, -1)) & 0xFFFFFFFF,
        )

        # all the players with a group in the first: the others go in a
        # second group, and together they would not fit in a team
        grouped_count = random_.randint(1, 6)
        other_count = random_.randint(1, 6)
        t.check(
            "network_team_balance_fits",
            grouped_count,
            other_count,
            Buffer(longs([0] * 16)),
            2,
            max(grouped_count, other_count) + random_.choice((0, 0, 1)),
            0,
            0,
            16,
            1,
            0,
        )

        # so many teams and players that the room in them is more than there
        # is in a number: what they are short of turns round
        t.check(
            "network_team_balance_fits",
            0,
            random_.randint(1, 8),
            Buffer(longs([0] * 16)),
            *random_.choice(((4, 0x40000000), (2, 0x7FFFFFFF), (2, 0x40000000))),
            0,
            0,
            16,
            0,
            random_.choice((0, 1, 3, 8)),
        )
