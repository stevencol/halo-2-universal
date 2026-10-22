# five small functions of the teams that the matchmaking makes even: a player
# out of a group, a group into a team and out of it, and how far apart the
# players of the teams are, place by place

import random
import struct

from difftest import Buffer

GROUP = 0x90
TEAM = 0x998
TOTAL = 8 + 16 * GROUP
TEAMS = 4 + 4 * TEAM


def players(random_: random.Random, count: int, values: tuple, identifiers: set) -> list:
    """So many players (number, identifier), each identifier different."""
    result = []
    while len(result) < count:
        identifier = random_.getrandbits(32)
        if identifier not in identifiers:
            identifiers.add(identifier)
            result.append((random_.choice(values), identifier))
    return result


def group(random_: random.Random, members: list, identifier: int) -> bytes:
    """A group of 0x90 bytes with those players, from the highest number."""
    data = bytearray(random_.randbytes(GROUP))
    data[0] = random_.choice((0, 0, 1, 2))
    members = sorted(members, key=lambda player: -player[0])
    for index, (skill, member) in enumerate(members):
        struct.pack_into("<iI", data, 0x10 + index * 8, skill, member)
    struct.pack_into("<IiI", data, 4, identifier, len(members), sum(skill for skill, _ in members) & 0xFFFFFFFF)
    return bytes(data)


def team(random_: random.Random, values: tuple, sizes: tuple) -> tuple[bytes, list]:
    """A team whose groups and group of all the players agree; and its groups."""
    identifiers: set = set()
    groups = []
    total = []
    count = random_.choice((1, 1, 2, 3, 5, 8))
    for _ in range(count):
        size = random_.choice(sizes)
        if len(total) + size > 16:
            size = 16 - len(total)
        members = players(random_, size, values, identifiers)
        total += members
        groups.append(group(random_, members, random_.getrandbits(32)))
    data = bytearray(random_.randbytes(TEAM))
    struct.pack_into("<ii", data, 0, count, random_.randrange(-2, 9))
    for index, bytes_ in enumerate(groups):
        data[8 + index * GROUP : 8 + (index + 1) * GROUP] = bytes_
    # the group of all, in another order than the groups
    random_.shuffle(total)
    data[TOTAL : TOTAL + GROUP] = group(random_, total, random_.getrandbits(32))
    return bytes(data), groups


def run(t) -> None:
    random_ = random.Random(750)

    for n in range(300):
        values = random_.choice(((0, 1, 2, 3), (5, 5, 6), (-3, 0, 7, 100), (-0x80000000, 0x7FFFFFFF, 1)))
        sizes = random_.choice(((0, 1, 2), (1, 1, 2, 4), (3, 4)))

        # a player that is in the group, wherever it is
        identifiers: set = set()
        members = players(random_, random_.choice((1, 2, 5, 16, random_.randint(1, 16))), values, identifiers)
        removed = random_.choice(members)
        t.check(
            "network_team_balance_group_remove_player",
            Buffer(group(random_, members, random_.getrandbits(32))),
            Buffer(struct.pack("<iI", random_.choice(values), removed[1])),
        )

        data, groups = team(random_, values, sizes)
        count = struct.unpack_from("<i", data, 0)[0]
        if count < 16:
            room = 16 - struct.unpack_from("<i", data, TOTAL + 8)[0]
            new = group(random_, players(random_, random_.randint(0, min(room, 6)), values, set()), random_.getrandbits(32))
            t.check("network_team_balance_team_add_group", Buffer(data), Buffer(new))

        data, groups = team(random_, values, sizes)
        t.check("network_team_balance_team_remove_group", Buffer(data), Buffer(random_.choice(groups)))

        data, groups = team(random_, values, sizes)
        t.check("network_team_balance_team_take_largest_group", Buffer(data), Buffer(random_.randbytes(GROUP)))

        # teams with their players in order, from the highest number
        data = bytearray(random_.randbytes(TEAMS))
        struct.pack_into("<i", data, 0, random_.choice((0, 1, 2, 3, 4, 4, 4, -1)))
        for index in range(4):
            size = random_.choice((0, 1, 2, 5, 16, random_.randint(0, 16)))
            at = 4 + index * TEAM + TOTAL
            data[at : at + GROUP] = group(random_, players(random_, size, values, set()), random_.getrandbits(32))
        t.check("network_team_balance_place_difference", Buffer(bytes(data)))
