# the options of a game: whether they can be played, and whether two are of
# the same game (source/game/game.c)
#
# The options are made good and then spoiled in one or more ways, at random

import random
import struct

from difftest import Buffer

OPTIONS_SIZE = 0x1118
MAP_NAME = 0x1C
UNKNOWN120 = 0x120
DIFFICULTY = 0x12A
COOP = 0x12C
VARIANT = 0x134
VARIANT_SIZE = 0x130
MACHINE_VALID_MASK = 0x268
MACHINES = 0x26C
LOCAL_MACHINE_EXISTS = 0x2CC
LOCAL_MACHINE = 0x2CD
PLAYERS = 0x2D4
PLAYER_SIZE = 0xE4
PLAYER_COUNT = 16
MACHINE_SIZE = 6

# a player
VALID = 0
UNKNOWN1 = 1
CONTROLLER = 2
MACHINE = 8
IDENTIFIER = 0xE


def player_offset(index: int) -> int:
    return PLAYERS + index * PLAYER_SIZE


def good_options(random_: random.Random, game_mode: int | None = None) -> bytearray:
    """Options that the game takes."""
    data = bytearray(random_.randbytes(OPTIONS_SIZE))
    game_mode = game_mode if game_mode is not None else random_.choice((1, 1, 2, 2, 3, 4, 5))
    coop = random_.randrange(2)
    struct.pack_into("<ibBh", data, 0, game_mode, random_.randrange(6), random_.getrandbits(8), random_.choice((30, 25, 1, 300)))
    name = random_.choice((b"scenarios\\solo\\03a_oldmombasa\\03a_oldmombasa", b"x", b"y" * 0x103))
    data[MAP_NAME : MAP_NAME + 0x104] = name.ljust(0x104, b"\0")
    struct.pack_into("<h", data, UNKNOWN120, random_.randrange(16))
    struct.pack_into("<h", data, DIFFICULTY, random_.randrange(4))
    data[COOP] = coop

    machine_count = random_.randint(1, 4)
    machine_slots = random_.sample(range(16), machine_count)
    mask = 0
    for slot in machine_slots:
        mask |= 1 << slot
        data[MACHINES + slot * MACHINE_SIZE : MACHINES + (slot + 1) * MACHINE_SIZE] = bytes([slot + 1]) * MACHINE_SIZE
    struct.pack_into("<I", data, MACHINE_VALID_MASK, mask)
    data[LOCAL_MACHINE_EXISTS] = 1
    data[LOCAL_MACHINE : LOCAL_MACHINE + MACHINE_SIZE] = bytes([machine_slots[0] + 1]) * MACHINE_SIZE

    if game_mode == 1:
        player_count = 2 if coop else 1
    else:
        player_count = random_.randint(1, 16 if game_mode != 2 else 16)
    for index in range(PLAYER_COUNT):
        data[player_offset(index) + VALID] = 0
    for number, index in enumerate(random_.sample(range(PLAYER_COUNT), player_count)):
        at = player_offset(index)
        slot = random_.choice(machine_slots)
        data[at + VALID] = 1
        data[at + UNKNOWN1] = 0
        struct.pack_into("<h", data, at + CONTROLLER, random_.randrange(4))
        data[at + MACHINE : at + MACHINE + MACHINE_SIZE] = bytes([slot + 1]) * MACHINE_SIZE
        data[at + IDENTIFIER : at + IDENTIFIER + 12] = struct.pack("<3I", 0x1000 + number, random_.getrandbits(32), 7)
    return data


def valid_players(data: bytearray) -> list[int]:
    return [index for index in range(PLAYER_COUNT) if data[player_offset(index) + VALID]]


def spoil(random_: random.Random, data: bytearray) -> None:
    """Changes one thing of the options: most of the changes make them bad."""
    players = valid_players(data)
    player = player_offset(random_.choice(players)) if players else player_offset(0)
    (mask,) = struct.unpack_from("<I", data, MACHINE_VALID_MASK)
    machines = [slot for slot in range(16) if mask & (1 << slot)]
    choice = random_.randrange(32)
    if choice == 0:
        struct.pack_into("<i", data, 0, random_.choice((0, 6, -1, 7, 1, 2, 3, 4, 5)))
    elif choice == 1:
        struct.pack_into("<b", data, 4, random_.choice((6, -1, 0x7F, 0)))
    elif choice == 2:
        struct.pack_into("<h", data, 6, random_.choice((0, 301, -1, 300)))
    elif choice == 3:
        data[MAP_NAME] = 0
    elif choice == 4:
        struct.pack_into("<h", data, UNKNOWN120, random_.choice((16, -1, 15)))
    elif choice == 5:
        struct.pack_into("<I", data, MACHINE_VALID_MASK, mask | random_.choice((0x10000, 0x80000000)))
    elif choice == 6 and len(machines) > 1:
        # two consoles the same
        a, b = random_.sample(machines, 2)
        data[MACHINES + b * MACHINE_SIZE : MACHINES + (b + 1) * MACHINE_SIZE] = data[
            MACHINES + a * MACHINE_SIZE : MACHINES + (a + 1) * MACHINE_SIZE
        ]
    elif choice == 7:
        struct.pack_into("<h", data, player + CONTROLLER, random_.choice((4, -1, 3, 0x100)))
    elif choice == 8:
        data[player + IDENTIFIER : player + IDENTIFIER + 12] = bytes(12)
    elif choice == 9 and len(players) > 1:
        # two players the same
        a, b = random_.sample(players, 2)
        data[player_offset(b) + IDENTIFIER : player_offset(b) + IDENTIFIER + 12] = data[
            player_offset(a) + IDENTIFIER : player_offset(a) + IDENTIFIER + 12
        ]
    elif choice == 10:
        data[player + UNKNOWN1] = 1
        if random_.random() < 0.6:
            data[player + MACHINE : player + MACHINE + MACHINE_SIZE] = bytes(MACHINE_SIZE)
    elif choice == 11:
        # at a console that is not in the game
        data[player + MACHINE : player + MACHINE + MACHINE_SIZE] = b"\x77" * MACHINE_SIZE
    elif choice == 12:
        struct.pack_into("<h", data, DIFFICULTY, random_.choice((4, -1, 3)))
    elif choice == 13:
        data[COOP] ^= 1
    elif choice == 14:
        data[LOCAL_MACHINE_EXISTS] = 0
    elif choice == 15:
        # one player more or one less
        missing = [index for index in range(PLAYER_COUNT) if index not in players]
        if missing and random_.random() < 0.5:
            at = player_offset(random_.choice(missing))
            data[at + VALID] = 1
            data[at + UNKNOWN1] = 0
            struct.pack_into("<h", data, at + CONTROLLER, 1)
            data[at + MACHINE : at + MACHINE + MACHINE_SIZE] = data[player + MACHINE : player + MACHINE + MACHINE_SIZE]
            data[at + IDENTIFIER : at + IDENTIFIER + 12] = random_.randbytes(12)
        else:
            data[player + VALID] = 0
    elif choice == 16:
        for index in players:
            data[player_offset(index) + VALID] = 0
    elif choice == 17:
        data[LOCAL_MACHINE : LOCAL_MACHINE + MACHINE_SIZE] = b"\x66" * MACHINE_SIZE
    elif choice == 18:
        data[VARIANT + random_.randrange(VARIANT_SIZE)] ^= 0x40
    elif choice == 19:
        data[MAP_NAME + random_.randrange(1, 4)] ^= 0x20
    elif choice == 20:
        struct.pack_into("<I", data, MACHINE_VALID_MASK, 0)
    elif choice == 21:
        struct.pack_into("<h", data, player + CONTROLLER, random_.randrange(4))
    elif choice in (22, 23) and machines:
        # a console that is not in the game, the same as one that is or as another that is not
        others = [slot for slot in range(16) if slot not in machines]
        if others:
            a = random_.choice(machines if choice == 22 or len(others) < 2 else others)
            b = random_.choice([slot for slot in others if slot != a] or others)
            data[MACHINES + b * MACHINE_SIZE : MACHINES + (b + 1) * MACHINE_SIZE] = data[
                MACHINES + a * MACHINE_SIZE : MACHINES + (a + 1) * MACHINE_SIZE
            ]
    elif choice in (24, 25) and players:
        # a player that is not in the game, the same as one that is
        missing = [index for index in range(PLAYER_COUNT) if index not in players]
        if missing:
            at = player_offset(random_.choice(missing))
            data[at + IDENTIFIER : at + IDENTIFIER + 12] = data[player + IDENTIFIER : player + IDENTIFIER + 12]
    elif choice in (26, 27) and len(players) > 1:
        # two players the same but for the end of their identifiers
        a, b = random_.sample(players, 2)
        identifier = bytearray(data[player_offset(a) + IDENTIFIER : player_offset(a) + IDENTIFIER + 12])
        identifier[random_.randrange(8, 12)] ^= 1
        data[player_offset(b) + IDENTIFIER : player_offset(b) + IDENTIFIER + 12] = identifier
    elif choice in (28, 29):
        data[VARIANT + VARIANT_SIZE - 1 - random_.randrange(8)] ^= 0x40
    # the others: no change


def run(t) -> None:
    random_ = random.Random(28)

    for case in range(2500):
        data = good_options(random_)
        if case >= 40:
            for _ in range(random_.choice((1, 1, 1, 2, 3))):
                spoil(random_, data)
        t.check("game_options_verify", Buffer(bytes(data)))

    for case in range(2500):
        first = good_options(random_, random_.choice((1, 1, 2, 2, 3, 0)))
        second = bytearray(first)
        # what the comparison does not look at
        if random_.random() < 0.5:
            second[4:6] = random_.randbytes(2)
            second[8:0x1C] = random_.randbytes(0x14)
        if case >= 40:
            for _ in range(random_.choice((0, 1, 1, 2))):
                spoil(random_, random_.choice((first, second)))
        t.check("game_options_match", Buffer(bytes(first)), Buffer(bytes(second)))
