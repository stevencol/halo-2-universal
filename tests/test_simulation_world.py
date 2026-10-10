# source/networking/simulation_world.c

import random
import struct

from difftest import Buffer

WORLD = 0x80
VIEW = 0x90
OTHER = 0x28


def a_world(random_, channel: int) -> Buffer:
    """What the game in the network has going, with some of its fifteen views after it, and after each what it has at 0xC."""
    data = bytearray(random_.randbytes(WORLD))
    struct.pack_into("<I", data, 8, random_.choice((0, 1, 1, 1, 1, 0x1234)))
    struct.pack_into("<i", data, 0x28, random_.choice((0, 5, 100, 1000, 1129, 1130, random_.getrandbits(12))))
    pointers = []
    chance = random_.choice((0.1, 0.5, 0.9))
    for index in range(15):
        if random_.random() >= chance:
            struct.pack_into("<I", data, 0x40 + index * 4, 0)
            continue
        at = len(data)
        view = bytearray(random_.randbytes(VIEW))
        struct.pack_into("<H", view, 2, random_.choice((0, 1, 1, 1, 2, 2, 3, 3, 4, 5, 0x101, 0x21, random_.getrandbits(8))))
        struct.pack_into("<i", view, 0x3C, random_.choice((channel, channel, random_.randrange(4), -1)))
        struct.pack_into("<I", view, 0xC, at + VIEW)
        view[0x78] = random_.choice((0, 1, 1, 0x80))
        struct.pack_into("<i", view, 0x84, random_.choice((0, 5, 100, 1000, 1200, random_.getrandbits(12))))
        other = bytearray(random_.randbytes(OTHER))
        other[0x24] = random_.choice((0, 1, 1, 0x80))
        data += view + other
        struct.pack_into("<I", data, 0x40 + index * 4, at)
        pointers += [0x40 + index * 4, at + 0xC]
    return Buffer(bytes(data), pointers=tuple(pointers))


def run(t) -> None:
    random_ = random.Random(444)

    for n in range(2500):
        channel = random_.randrange(4)
        mask = random_.choice((0xFFFFFFFF, 0xFFFFFFFF, 1 << random_.randrange(6), 0x14, 0, random_.getrandbits(32)))
        index = random_.choice((0, 0, 0, 3, 14, 15, 16, -1, -5, random_.randrange(15)))
        t.check(
            "simulation_world_next_view",
            Buffer(struct.pack("<Ii", mask, index)),
            a_world(random_, channel),
            Buffer(random_.randbytes(4)),
        )
        t.check("simulation_world_get_channel_view", a_world(random_, channel), random_.choice((channel, channel, 0xFFFFFFFF, 7)))

    # ---------- what the thing has of each player, its first view of two kinds, and memory that is given back

    for n in range(1500):
        players = bytearray(random_.randbytes(0x7C + 16 * 0x88))
        for player in range(16):
            if random_.random() < 0.4:
                struct.pack_into("<i", players, 0x7C + player * 0x88, -1)
            players[0x7C + player * 0x88 + 0x25] = random_.choice((0, 0, 1, 1, 0x80))
        t.check(
            "simulation_world_player_unknown25",
            random_.choice((random_.randrange(16), random_.randrange(16) | 0x30000, random_.randrange(16) | 0xFFFF0000)),
            Buffer(bytes(players)),
        )
        t.check("simulation_world_players_clear_unknown25", Buffer(bytes(players)))
        t.check("simulation_world_unknown_0006acb0", a_world(random_, random_.randrange(4)))
        t.check(
            "simulation_world_unknown_0006aae0",
            Buffer(random_.randbytes(0x1210)),
            externals={"texture_cache_return_memory": lambda harness, arguments: 0},
        )

    # ---------- the things that have come and wait, and the game brought up to them

    from difftest import Pointer

    THING, WAIT = 0x1400, 0x1300

    def a_thing(state=None) -> Buffer:
        """What a view has at 0xC: going or not, in a state, with some things that have come; and at the end, what the
        function that counts them in the second state is to say."""
        data = bytearray(random_.randbytes(THING))
        struct.pack_into("<i", data, 0x8, random_.choice((1, 2, 2, 3, 3, 3, 4, 5)) if state is None else state)
        struct.pack_into("<i", data, 0x18, random_.choice((0, 3, 4, 4)))
        data[0x24] = random_.choice((0, 1, 1, 1, 0x80))
        data[0x25] = random_.choice((0, 1, 1, 0x80))
        data[0x2C] = random_.choice((0, 0, 0, 1))
        struct.pack_into("<i", data, 0x1200, random_.choice((-1, -1, -1, 0, 7)))
        first = random_.choice((0, 5, 1000, 0x7FFFFFF0))
        struct.pack_into("<2i", data, 0x120C, first, first + random_.choice((-1, -1, 0, 1, 3, -2)))
        struct.pack_into("<i", data, WAIT, random_.choice((0, 0, 1, 2, 4, -1, 0x7FFFFFFF)))
        return Buffer(bytes(data))

    def waits(harness, arguments):
        return struct.unpack("<i", harness.mu.mem_read(arguments[0] + WAIT, 4))[0]

    def ticks(thing: Buffer):
        """A tick of the game: one thing fewer waits, in either state; and now and then the thing stops going."""

        def handler(harness, arguments):
            mu = harness.mu
            wait, = struct.unpack("<i", mu.mem_read(thing.address + WAIT, 4))
            mu.mem_write(thing.address + WAIT, struct.pack("<i", 0 if wait == 0x7FFFFFFF else wait - 1))
            first, = struct.unpack("<i", mu.mem_read(thing.address + 0x120C, 4))
            mu.mem_write(thing.address + 0x120C, struct.pack("<i", first + 1))
            if first % 7 == 3:
                mu.mem_write(thing.address + 0x24, b"\0")
            if first % 5 == 2:
                mu.mem_write(thing.address + 0x2C, b"\1")
            return 0

        return handler

    for n in range(2500):
        thing = a_thing()
        t.check("simulation_world_unknown_00069300", thing, Buffer(random_.randbytes(4)), externals={"simulation_world_unknown_00069790": waits})
        thing = a_thing()
        t.check(
            "simulation_world_unknown_00069350",
            random_.choice((0, 1, 1, 1, 0x80)),
            thing,
            externals={"simulation_world_unknown_00069790": waits, "game_tick": ticks(thing)},
        )
        thing = a_thing(random_.choice((None, None, 3, 2)))
        last, = struct.unpack_from("<i", thing.data, 0x1210)
        game = bytearray(random_.randbytes(0x1130))
        game[0x1120] = random_.choice((0, 1, 1, 1, 1, 0x80))
        there = bytearray(random_.randbytes(0x10))
        there[0] = random_.choice((0, 1, 1))
        there[2] = random_.choice((0, 0, 0, 1))
        struct.pack_into("<I", there, 0xC, 0)
        could = random_.choice((1, 1, 1, 0))
        t.check(
            "simulation_world_unknown_000698e0",
            thing,
            Buffer(struct.pack("<i", last + random_.choice((1, 1, 1, 1, 0, 2, -3))) + random_.randbytes(0x1C)),
            globals={"main_game_globals": random_.choice((0, 1, 1, 1, 1)) and Buffer(bytes(game)), "unknown_004cf770": bytes(there)},
            externals={
                "simulation_world_unknown_00069790": waits,
                "game_tick": ticks(thing),
                "unknown_0006ab90": lambda harness, arguments, could=could: could,
            },
        )
        view = bytearray(random_.randbytes(0x40))
        struct.pack_into("<i", view, 0x2C, random_.choice((0, 1, 1, 1, 2, 3)))
        struct.pack_into("<i", view, 0x34, random_.choice((0, 1, 1, 2)))
        t.check(
            "simulation_world_unknown_0006a2a0",
            Buffer(bytes(view)),
            Buffer(random_.randbytes(0x40)),
            externals={"unknown_00085a70": lambda harness, arguments: 0},
        )

    # ---------- the things that wait in the second state, and what the views of the second kind are told

    blocked = lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1] & 0xFF))

    for n in range(2500):
        t.check(
            "simulation_world_unknown_00069790",
            a_world(random_, 0),
            externals={"simulation_view_set_blocked": blocked},
        )
        t.check(
            "simulation_world_unknown_00069880",
            a_world(random_, 0),
            random_.getrandbits(32),
            externals={"unknown_000860b0": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1]))},
        )
