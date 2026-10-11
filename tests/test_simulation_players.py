# source/networking/simulation_players.c

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
SIZE = 0xC34
WORLD, WORLD_SIZE = 0xC40, 0x7C + 16 * 0x88
SESSION, SESSION_SIZE = WORLD + WORLD_SIZE + 4, 0x7620


def run(t) -> None:
    random_ = random.Random(567)
    told = lambda harness, arguments: (0, tuple(harness.normalize(value) for value in arguments))

    def thing(world: bool = True, session: bool = True) -> Buffer:
        """The thing, and after it its world and its session."""
        data = bytearray(random_.randbytes(SESSION + SESSION_SIZE))
        pointers = []
        struct.pack_into("<I", data, 4, WORLD if world else 0)
        struct.pack_into("<I", data, 0xC, SESSION if session else 0)
        if world:
            pointers.append(4)
        if session:
            pointers.append(0xC)
        data[0x84] = random_.choice((0, 0, 1))
        struct.pack_into("<I", data, 0x18, random_.choice((5, 5, NONE)))
        struct.pack_into("<I", data, 0x1C, random_.getrandbits(16))
        struct.pack_into("<I", data, 0x88, random_.getrandbits(16))
        struct.pack_into("<I", data, WORLD + 8, random_.choice((0, 1, 2, 3, 4, 4, 5, 6)))
        for index in range(16):
            struct.pack_into("<I", data, WORLD + 0x7C + index * 0x88, random_.choice((NONE, 0x10000 + index, 7)))
            data[WORLD + 0x7C + index * 0x88 + 0x24] = random_.choice((0, 1))
            data[WORLD + 0x7C + index * 0x88 + 0x25] = random_.choice((0, 1))
            data[0x8C + index * 0xB4 + 0xC] = random_.choice((0, 0, 1))
        struct.pack_into("<I", data, SESSION + 0x741C, random_.choice((0, 2, 3, 4, 8, 9)))
        struct.pack_into("<I", data, SESSION + 0x4980, random_.choice((4, 4, 4, 3, 5)))
        struct.pack_into("<I", data, SESSION + 0x7618, random_.choice((5, 5, 6, NONE)))
        return Buffer(bytes(data), pointers=tuple(pointers))

    for n in range(1500):
        t.check("simulation_players_reset", thing(), externals={"simulation_players_collection_clear": told})
        game_players = bytearray(random_.randbytes(0x90))
        t.check(
            "simulation_players_refresh",
            thing(),
            globals={"players_globals": Buffer(bytes(game_players))},
            externals={"simulation_players_collection_clear": told, "simulation_players_collection_build": told, "simulation_world_players_clear_unknown25": told},
        )
        t.check("simulation_players_mark_player", thing(), random_.choice((*range(16), *range(16), 16, 17, 0xFFFF)))
        t.check("simulation_players_changed", thing(world=random_.random() < 0.9, session=random_.random() < 0.8))

        players = thing()
        machine = random_.randrange(16)
        identifier = players.data[0x24 + machine * 6:0x24 + machine * 6 + 6]
        t.check("simulation_players_find_machine", players, Buffer(random_.choice((identifier, identifier, random_.randbytes(6)))))

        index = random_.randrange(16)
        who = players.data[0x8C + index * 0xB4:0x8C + index * 0xB4 + 12]
        t.check("simulation_players_has_player", index, players, Buffer(random_.choice((who, who, who, random_.randbytes(12)))))

        machines = bytearray(random_.randbytes(16 * 6))
        updates = bytearray(random_.randbytes(16 * 0x13C))
        for other in range(16):
            struct.pack_into("<I", updates, other * 0x13C + 0xC, random_.randrange(16))
        update = updates[index * 0x13C:(index + 1) * 0x13C]
        player = bytearray(random_.randbytes(0x30))
        if random_.random() < 0.85:
            # the same player, but for one thing now and then
            player[0:12] = update[0:12]
            at = struct.unpack_from("<I", update, 0xC)[0]
            player[0x14:0x1A] = machines[at * 6:at * 6 + 6]
            player[0x1C:0x24] = update[0x10:0x18]
            if random_.random() < 0.4:
                where = random_.choice((0, 5, 11, 0x14, 0x19, 0x1C, 0x23))
                player[where] ^= 1 << random_.randrange(8)
        t.check(
            "simulation_players_player_differs",
            index,
            Buffer(bytes(player)),
            Buffer(bytes(machines)),
            random_.choice((0xFFFF, 0xFFFF, random_.getrandbits(16))),
            Buffer(bytes(updates)),
        )

    # ---------- what there is of the players, from the players of the game

    COUNT, DATUM = 16, 0x170
    DATUMS = 0x4C
    BITS = DATUMS + COUNT * DATUM
    for n in range(1500):
        t.check("simulation_players_collection_clear", Buffer(random_.randbytes(0xB44 + 8)))

        data = bytearray(random_.randbytes(BITS + 4))
        data[0:0x20] = b"players".ljust(0x20, b"\0")
        there = [random_.random() < 0.5 for index in range(COUNT)]
        bits = 0
        for index in range(COUNT):
            struct.pack_into("<HH", data, DATUMS + index * DATUM, random_.getrandbits(15) | 0x8000 if there[index] else 0, random_.choice((0, 0, 2, 3, 0xFFFD, 0xFFFF)))
            bits |= there[index] << index
        struct.pack_into("<I", data, BITS, bits)
        struct.pack_into("<ii", data, 0x20, COUNT, DATUM)
        struct.pack_into("<ii", data, 0x34, next((index for index in range(COUNT) if not there[index]), COUNT), max((index + 1 for index in range(COUNT) if there[index]), default=0))
        struct.pack_into("<II", data, 0x44, DATUMS, BITS)
        t.check(
            "simulation_players_collection_build",
            Buffer(random_.randbytes(0xB44)),
            globals={"player_data": Buffer(bytes(data), pointers=(0x44, 0x48))},
        )

    # ---------- which players are still there, and a player that left

    for n in range(1500):
        collection = bytearray(random_.randbytes(0xB44))
        struct.pack_into("<I", collection, 0, random_.choice((0, 0xFFFF, random_.getrandbits(16), random_.getrandbits(32))))
        for index in range(16):
            collection[4 + index * 0xB4 + 0xC] = random_.choice((0, 0, 1, 0x80))
        t.check("simulation_players_collection_get_active_mask", Buffer(bytes(collection)))

        data = bytearray(random_.randbytes(BITS + 4))
        data[0:0x20] = b"players".ljust(0x20, b"\0")
        there = [random_.random() < 0.6 for index in range(COUNT)]
        for index in range(COUNT):
            struct.pack_into("<HH", data, DATUMS + index * DATUM, random_.getrandbits(15) | 0x8000 if there[index] else 0, random_.choice((0, 0, 0, 2, 0xFFFD, 0xFFFF)))
        struct.pack_into("<ii", data, 0x20, COUNT, DATUM)
        first_unallocated = random_.choice((COUNT, COUNT, 8))
        struct.pack_into("<ii", data, 0x34, 0, first_unallocated)
        struct.pack_into("<II", data, 0x44, DATUMS, BITS)
        place = random_.choice((*range(COUNT), *range(COUNT), NONE, COUNT, 0x80000000))
        who = bytes(data[DATUMS + (place % COUNT) * DATUM + 4:DATUMS + (place % COUNT) * DATUM + 16])
        update = struct.pack("<I", place) + random_.choice((who, who, who, random_.randbytes(12))) + random_.randbytes(8)
        t.check(
            "simulation_players_update_remove_player",
            Buffer(update),
            globals={"player_data": Buffer(bytes(data), pointers=(0x44, 0x48))},
            externals={"unknown_00152df0": told},
        )
