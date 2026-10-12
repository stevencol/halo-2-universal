# the squads and their groups, for a new map

import random
import struct

from difftest import Buffer

from test_data import array

NONE = 0xFFFFFFFF
GROUPS, GROUP_SIZE, SQUADS, SQUAD_SIZE = 0x64, 0x38, 0x14F, 0x98
SCENARIO = 0x400


def scenario(random_, group_count: int, squad_count: int) -> Buffer:
    """The scenario, with its groups and after them its squads: each is of a group that there is, or of none."""
    groups_at = SCENARIO
    squads_at = groups_at + max(group_count, 1) * 0x24
    data = bytearray(random_.randbytes(squads_at + max(squad_count, 1) * 0x74))
    struct.pack_into("<iIiI", data, 0x158, group_count, groups_at, squad_count, squads_at)

    def group() -> int:
        return random_.choice((-1, random_.randrange(group_count))) if group_count > 0 else -1

    for index in range(max(group_count, 0)):
        struct.pack_into("<h", data, groups_at + index * 0x24 + 0x20, group())
    for index in range(max(squad_count, 0)):
        struct.pack_into("<h", data, squads_at + index * 0x74 + 0x26, group())
    return Buffer(bytes(data), pointers=(0x15C, 0x164))


def tables(random_) -> dict:
    data = {}
    for name, maximum_count, size in (("squad_data", SQUADS, SQUAD_SIZE), ("squad_group_data", GROUPS, GROUP_SIZE)):
        in_use = {index: random_.randint(1, 0x7FFF) for index in random_.sample(range(maximum_count), random_.randint(0, 12))}
        data[name] = array(random_, maximum_count, size, in_use)
    return data


def run(t) -> None:
    random_ = random.Random(673)

    for n in range(300):
        data = tables(random_)
        # the squads of a group, one after the other: the list of a group ends
        squads = bytearray(data["squad_data"].data)
        order = list(range(SQUADS))
        random_.shuffle(order)
        for at, index in enumerate(order):
            later = -1 if at + 1 >= SQUADS or random_.random() < 0.3 else order[at + 1]
            struct.pack_into("<h", squads, 0x4C + index * SQUAD_SIZE + 0x74, later)
        groups = bytearray(data["squad_group_data"].data)
        for index in range(GROUPS):
            struct.pack_into(
                "<I",
                groups,
                0x4C + index * GROUP_SIZE + 8,
                random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(SQUADS))),
            )
        t.check(
            "squad_group_add_squad",
            random_.getrandbits(16) << 16 | random_.randrange(GROUPS),
            random_.getrandbits(32),
            globals={
                "squad_data": Buffer(bytes(squads), pointers=data["squad_data"].pointers),
                "squad_group_data": Buffer(bytes(groups), pointers=data["squad_group_data"].pointers),
            },
        )

    for n in range(150):
        group_count = random_.choice((0, -1, 1, 2, 7, 20))
        squad_count = random_.choice((0, -1, 1, 3, 12, 40))
        t.check(
            "squads_unknown_002009d0",
            globals={"global_scenario": scenario(random_, group_count, squad_count), **tables(random_)},
        )
        t.check(
            "squads_initialize_for_new_map",
            globals={"global_scenario": scenario(random_, group_count, squad_count), **tables(random_)},
            externals={"squad_new_from_scenario": lambda harness, arguments: 0},
        )
