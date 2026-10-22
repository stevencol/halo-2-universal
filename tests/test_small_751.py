# four small functions of the teams that the matchmaking makes even: making a
# team and the two things that hold teams, and putting the groups of players
# in the teams

import random
import struct

from difftest import Buffer
from test_small_750 import TEAM, team

TEAMS = 4 + 4 * TEAM
BALANCE = 4 + TEAMS + 4
IMPROVEMENT = TEAMS + 4 + TEAMS


def run(t) -> None:
    random_ = random.Random(751)

    for n in range(300):
        t.check("network_team_balance_team_construct", Buffer(random_.randbytes(TEAM)))
        t.check("network_team_balance_construct", Buffer(random_.randbytes(BALANCE)))
        t.check("network_team_balance_improvement_construct", Buffer(random_.randbytes(IMPROVEMENT)))

        values = random_.choice(((0, 1, 2, 3), (5, 5, 6), (-3, 0, 7, 100), (-0x80000000, 0x7FFFFFFF, 1)))
        sizes = random_.choice(((0, 1, 2), (1, 1, 2, 4), (3, 4), (1,)))
        groups, _ = team(random_, values, sizes)
        if random_.random() < 0.1:
            groups = struct.pack("<i", random_.choice((0, -1))) + groups[4:]
        t.check(
            "network_team_balance_distribute",
            Buffer(random_.randbytes(BALANCE)),
            random_.choice((1, 2, 2, 3, 4, 4)),
            Buffer(groups),
            random_.choice((0, 1, 2, 3, 4, 5, 8, 16, -1)) & 0xFFFFFFFF,
        )
