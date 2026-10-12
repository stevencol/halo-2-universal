# source/networking/network_hoppers.c

import random
import struct

from difftest import Buffer, GlobalData

NONE = 0xFFFFFFFF
COUNT, SIZE = 16, 0x614


def hopper(random_) -> bytearray:
    data = bytearray(random_.randbytes(SIZE))
    struct.pack_into("<I", data, 0, random_.choice((0, 1, 2, 3, 4, 4, 5, 5, 6, NONE)))
    struct.pack_into("<3I", data, 0x5EC, *(random_.choice((0, 1, 2, 8, 16, 17, NONE, 0x80000000)) for index in range(3)))
    data[0x604] = random_.choice((0, 0, 1, 0xFF))
    return data


def run(t) -> None:
    random_ = random.Random(541)

    for n in range(2500):
        one = Buffer(bytes(hopper(random_)))
        for name in (
            "network_hopper_get_unknown5F4",
            "network_hopper_get_maximum",
            "network_hopper_unknown_00193470",
            "network_hopper_unknown_00193490",
            "network_hopper_unknown_001934b0",
            "network_hopper_unknown_001934d0",
            "network_hopper_players_valid",
        ):
            t.check(name, one)

        blob = bytearray(random_.randbytes(0x30))
        struct.pack_into("<HHH", blob, 4, random_.choice((2, 2, 2, 3, 0, 0xFFFD, 0xFFFF)), random_.getrandbits(16), random_.choice((0, 1, 1, 4, 0xFFFF)))
        table = b"".join(bytes(hopper(random_)) for index in range(COUNT))
        globals_ = {"unknown_0047d8f4": bytes(blob), "network_hoppers": GlobalData(table)}
        good = lambda harness, arguments, so=random_.choice((0, 1, 1)): (so, (harness.normalize(arguments[0]),))
        index = random_.choice((0, 1, 7, 15, 16, 17, NONE, 0x80000000, random_.randrange(16)))
        t.check("network_hoppers_index_valid", index, globals=globals_, externals={"network_hopper_valid": good})
        t.check("network_hoppers_get", index, globals=globals_, externals={"network_hopper_valid": good})

    # ---------- what one is asked by its kind: the game does not look at whether the kind is one of the five

    for n in range(2500):
        data = hopper(random_)
        struct.pack_into("<I", data, 0, random_.randrange(1, 6))
        struct.pack_into("<3I", data, 0x5EC, random_.choice((1, 1, 2, 4)), random_.choice((1, 2, 3, 4)), random_.choice((1, 2, 3, 4)))
        struct.pack_into("<I", data, 0x5F8, random_.choice((0, 1, 0x100, 0x1FF, random_.getrandbits(32))))
        one = Buffer(bytes(data))
        t.check("network_hopper_get_players_value", one, random_.choice((0, 1, 2, 3, 4, 6, 8, 12, 16, 17, NONE, random_.randrange(18))))
        for name in (
            "network_hopper_unknown_00193250",
            "network_hopper_get_minimum_players",
            "network_hopper_get_maximum_players",
            "network_hopper_get_minimum_teams",
            "network_hopper_get_maximum_teams",
            "network_hopper_unknown_00193400",
            "network_hopper_get_team_size",
        ):
            t.check(name, one)
        says = lambda so: (lambda harness, arguments: (so, (bytes(harness.mu.mem_read(arguments[0], 4)),)))
        t.check(
            "network_hopper_valid",
            one,
            externals={
                "network_hopper_header_valid": says(random_.choice((0, 1, 1, 1))),
                "network_hopper_players_valid": says(random_.choice((0, 1))),
                "network_hopper_type4_valid": says(random_.choice((0, 1))),
                "network_hopper_teams_valid": says(random_.choice((0, 1))),
            },
        )

    # ---------- the three checks

    SMALL = (0, 1, 1, 2, 2, 3, 4, 8, 16, 17, NONE, 0x80000000)
    for n in range(6000):
        data = hopper(random_)
        struct.pack_into("<3I", data, 0, random_.choice((1, 2, 3, 4, 5, 5, 0, 6)), random_.choice((0, 1, 3, 3, 4, NONE)), random_.choice((1, 50, 100, 100, 101, 0, NONE)))
        if random_.random() < 0.8:
            # bytes that are right, but for one now and then
            for index in range(128):
                data[0x51C + index] = random_.choice((index, index, 0x7F, min(index + 3, 0x7F)))
            if random_.random() < 0.3:
                index = random_.randrange(128)
                data[0x51C + index] = random_.choice((max(index - 1, 0), 0, 0x80, 0xFF))
        struct.pack_into("<6I", data, 0x5EC, *(random_.choice(SMALL) for index in range(6)))
        if random_.random() < 0.5:
            # numbers of teams that are right, or nearly
            size = random_.choice((2, 2, 4, 8))
            most = random_.choice((16 // size, 16 // size, 16 // size + 1, 2))
            least = random_.choice((1, 2, 2, most, most + 1))
            second = random_.choice((most, most, 2, most + 1, 1))
            first = random_.choice((1, 1, second, second + 1, 0))
            struct.pack_into("<6I", data, 0x5EC, size, least, most, random_.choice((0, 0, 1, max(most - least, 0), max(most - least, 0) + 1, NONE)), first, second)
        one = Buffer(bytes(data))
        t.check("network_hopper_header_valid", one)
        t.check("network_hopper_type4_valid", one)
        t.check("network_hopper_teams_valid", one)

    # ---------- two more

    for n in range(3000):
        data = hopper(random_)
        if random_.random() < 0.7:
            for index in range(128):
                data[0x51C + index] = random_.choice((index, index, 0x7F, min(index + 3, 0x7F), max(index - 2, 0), 0x80))
        t.check("network_hopper_unknown_001931a0", random_.choice((0, 1, 2, 5, 64, 127, 128, NONE, 0x80000000, random_.randrange(129))), Buffer(bytes(data)))

        blob = bytearray(random_.randbytes(0x30))
        struct.pack_into("<HHH", blob, 4, random_.choice((2, 2, 2, 3, 0, 0xFFFD)), random_.getrandbits(16), random_.choice((0, 1, 1, 4, 0xFFFF)))
        table = bytearray()
        for index in range(COUNT):
            one = hopper(random_)
            if random_.random() < 0.8:
                # one that is good: without teams, from 1 to 16 players
                struct.pack_into("<3I", one, 0, random_.choice((1, 2)), random_.choice((0, 0, 1, 2, 3)), random_.choice((1, 100)))
                one[0x51C:0x59C] = bytes((0x7F,)) * 128
                struct.pack_into("<2I", one, 0x5EC, 1, 16)
            table += one
        # network_hopper_valid is not stood in: the game asks it three times and source/ once, which is the same
        t.check(
            "network_hoppers_get_unknown4",
            random_.choice((0, 1, 7, 15, 16, NONE, NONE, 0x80000000, random_.randrange(16), random_.randrange(16))),
            globals={"unknown_0047d8f4": bytes(blob), "network_hoppers": GlobalData(bytes(table))},
            externals={"unknown_00194660": lambda harness, arguments: (0, tuple(arguments))},
        )

    # ---------- the game that one of the list plays next

    GAME, GAMES, COUNT_AT = 0x37C, 0x48, 0x44
    for n in range(3000):
        count = random_.choice((0, 1, 1, 2, 3, 5, 8, NONE))
        games = bytearray(random_.randbytes(GAMES + 8 * GAME))
        struct.pack_into("<I", games, COUNT_AT, count)
        have = set()
        for index in range(8):
            map_ = random_.randrange(1, 40)
            struct.pack_into("<I", games, GAMES + index * GAME + 4, map_)
            struct.pack_into("<I", games, GAMES + index * GAME + 0x248, random_.choice((0, 1, 1, 10, 100, 1000, 1001, 30000, NONE, 0x80000000)))
            if random_.random() < 0.7:
                have.add(map_)

        def games_of(harness, arguments, so=random_.choice((1, 1, 1, 1, 0)), games=bytes(games)):
            harness.mu.mem_write(arguments[1], games)
            return so, (arguments[0],)

        t.check(
            "network_hopper_choose_game",
            random_.randrange(16),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(0x130)),
            globals={"global_random_seed_reference": Buffer(random_.randbytes(8))},
            externals={
                "unknown_001943d0": games_of,
                "levels_get_multiplayer_level": lambda harness, arguments, have=frozenset(have): (0x12340000 if arguments[0] in have else 0, tuple(arguments)),
            },
        )
