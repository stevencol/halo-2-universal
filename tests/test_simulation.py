# source/networking/simulation.c

import random
import struct

from difftest import Buffer

from test_simulation_world import a_world


def run(t) -> None:
    random_ = random.Random(477)

    for n in range(1500):
        view = bytearray(random_.randbytes(0x40))
        struct.pack_into("<i", view, 0x28, random_.choice((0, 0, 0, 7)))
        t.check(
            "simulation_view_disconnected",
            Buffer(bytes(view)),
            externals={
                "simulation_view_set_state": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1], arguments[2])),
                "simulation_view_release_channel": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
            },
        )

        # what writes one of the things: so many bits, of which some bytes are set
        bits = random_.choice((0, 1, 8, 9, 0x123))

        def writes(harness, arguments, bits=bits):
            mu = harness.mu
            where, size, unknown8, mode, position = struct.unpack("<I4i", mu.mem_read(arguments[0], 0x14))
            mu.mem_write(where, bytes((0xA5,)) * ((bits + 7) // 8))
            mu.mem_write(arguments[0] + 0x10, struct.pack("<i", position + bits))
            more = struct.unpack("<B3xi16x2i", mu.mem_read(arguments[0] + 0x14, 0x20))
            return 0, (harness.normalize(where), size, unknown8, mode, position, more[0], more[1], more[2], more[3], harness.normalize(arguments[1]))

        capacity = random_.choice((0x40, 0x41, 0x100))
        t.check(
            "simulation_update_encode",
            Buffer(random_.randbytes(capacity + 4)),
            capacity,
            Buffer(random_.randbytes(0x40)),
            Buffer(random_.randbytes(4)),
            externals={"unknown_00087d00": writes},
        )

        good = random_.choice((0, 1, 1, 0x100))
        size = random_.choice((4, 8, 0x40))
        data = random_.choice((random_.randbytes(size), b"gbed" + random_.randbytes(size - 4)))

        def reads(harness, arguments, good=good):
            mu = harness.mu
            where, size, unknown8, mode, position, error = struct.unpack("<I4iB", mu.mem_read(arguments[0], 0x15))
            count = struct.unpack("<i", mu.mem_read(arguments[0] + 0x18, 4))[0]
            mu.mem_write(arguments[1] + 0x10, b"\x77")
            return good, (harness.normalize(where), size, unknown8, mode, position, error, count, harness.normalize(arguments[1]))

        t.check(
            "simulation_update_decode",
            Buffer(data + random_.randbytes(4)),
            size,
            Buffer(random_.randbytes(0x4048 + 4)),
            externals={"unknown_00087e90": reads},
        )

        thing = a_world(random_, 0)
        world = bytearray(thing.data)
        struct.pack_into("<i", world, 0x8, random_.choice((0, 1, 3, 3, 3)))
        struct.pack_into("<i", world, 0x18, random_.choice((1, 2, 3, 4)))
        own = bytes((random_.choice((1, 2)),)) * 5 + bytes((random_.choice((1, 2, 3)),))
        world[0xD:0x13] = own
        thing = Buffer(bytes(world), pointers=thing.pointers)
        there = bytearray(random_.randbytes(0x10))
        there[0] = random_.choice((0, 1, 1, 1, 1))
        there[2] = random_.choice((0, 0, 1))
        struct.pack_into("<I", there, 0xC, 0)
        globals_ = {"unknown_004cf770": bytes(there), "unknown_004cf770+0xc": thing}
        t.check(
            "simulation_machine_established",
            Buffer(random_.choice((own, own, bytes((random_.choice((1, 2)),)) * 5 + bytes((random_.choice((1, 2, 3)),)))) + random_.randbytes(2)),
            globals=globals_,
        )
        kept = random_.choice((0, 1, 0x80))
        now = random_.getrandbits(32)
        t.check(
            "simulation_fail",
            globals=dict(globals_, unknown_00510548=bytes((kept,)), unknown_0051054c=now if kept else random_.getrandbits(32)),
            externals={
                "GetTickCount": lambda harness, arguments, now=now: now,
                "simulation_world_unknown_0006b040": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                "simulation_world_set_unknown18": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
            },
        )
        t.check(
            "simulation_unknown_000687e0",
            globals={"unknown_004cf770": bytes(there), "unknown_004cf780": 0x4321},
            externals={"unknown_00083db0": lambda harness, arguments: random_.choice((7,))},
        )

    # ---------- what the rest of the game asks of the game in the network, and what it does each tick

    for n in range(2500):
        world = bytearray(random_.randbytes(0x1220))
        state = random_.choice((0, 1, 2, 2, 3, 3, 4, 4, 5))
        struct.pack_into("<i", world, 0x8, state)
        struct.pack_into("<i", world, 0x18, random_.choice((1, 3, 4, 4, 4)))
        world[0x2C] = random_.choice((0, 0, 0, 1))
        number = random_.choice((0, 5, 1000))
        struct.pack_into("<i", world, 0x28, number)
        thing = Buffer(bytes(world))
        there = bytearray(random_.randbytes(0x10))
        there[0] = random_.choice((0, 1, 1, 1, 1, 1))
        there[2] = random_.choice((0, 0, 0, 1))
        struct.pack_into("<I", there, 0xC, 0)
        game = bytearray(random_.randbytes(0x1130))
        game[0x1120] = random_.choice((0, 1, 1, 1, 1))
        struct.pack_into("<h", game, 2, random_.choice((-1, 0, 0, 0, 3)))
        time = bytearray(random_.randbytes(0x10))
        struct.pack_into("<I", time, 8, 777)
        seed = Buffer(struct.pack("<I", 0x5EED))
        globals_ = {
            "unknown_004cf770": bytes(there),
            "unknown_004cf770+0xc": thing,
            "main_game_globals": random_.choice((0, 1, 1, 1, 1, 1)) and Buffer(bytes(game)),
            "game_time_globals": Buffer(bytes(time)),
            "global_random_seed_reference": seed,
            "unknown_004cf780": 0x4321,
        }
        t.check("simulation_before_save", random_.choice((0, 0, 1, 2, 3)), globals=globals_)
        waits = random_.choice((0, 3, 0x7FFFFFFF))
        t.check(
            "simulation_updates_waiting",
            Buffer(random_.randbytes(4)),
            globals=globals_,
            externals={"simulation_world_unknown_00069300": lambda harness, arguments, waits=waits: (waits, (harness.normalize(arguments[0]), harness.normalize(arguments[1])))},
        )
        for name in ("simulation_in_progress", "simulation_starting", "simulation_failed"):
            t.check(name, globals=globals_)

        made = struct.pack("<I", random_.choice((number, number, number, number + 1)))
        made_time = random_.choice((777, 777, 777, 778))
        made_seed = random_.choice((0x5EED, 0x5EED, 0x5EED, 1))

        def makes(harness, arguments, made=made, made_time=made_time, made_seed=made_seed):
            harness.mu.mem_write(arguments[1], made)
            harness.mu.mem_write(arguments[1] + 0x4040, struct.pack("<2I", made_time, made_seed))
            return 0, (harness.normalize(arguments[0]), harness.normalize(arguments[1]))

        t.check("simulation_build_update", Buffer(random_.randbytes(0x4048)), globals=globals_, externals={"simulation_world_build_update": makes})
        passes = lambda harness, arguments: (0, (harness.normalize(arguments[0]), harness.normalize(arguments[1]), arguments[2]))
        t.check(
            "simulation_apply_update",
            Buffer(random_.randbytes(0x4048)),
            globals=globals_,
            externals={
                "simulation_world_unknown_00069880": lambda harness, arguments: (0, (harness.normalize(arguments[0]), harness.normalize(arguments[1]))),
                "unknown_00069a00": passes,
                "unknown_00069b50": passes,
            },
        )
        lets = random_.choice((0, 1, 1, 1))
        # the thing that is made is in the stack: where it is is not compared
        t.check(
            "simulation_update",
            globals=globals_,
            externals={
                "unknown_000835c0": lambda harness, arguments, lets=lets: lets,
                "simulation_build_update": lambda harness, arguments: (0, ()),
                "unknown_000683a0": lambda harness, arguments: (0, ()),
                "simulation_apply_update": lambda harness, arguments: (0, ()),
            },
        )

    # ---------- the thing that comes of this tick is made

    for n in range(2000):
        other = bytearray(random_.randbytes(0x90))
        other[0x84] = random_.choice((0, 1, 1, 0x80))
        other = Buffer(bytes(other))
        t.check(
            "simulation_take_unknown_004cf780_change",
            Buffer(random_.randbytes(0x64)),
            Buffer(random_.randbytes(4)),
            globals={"unknown_004cf780": other},
            externals={"unknown_00084270": lambda harness, arguments: (0, (harness.normalize(arguments[0]),))},
        )
        world = bytearray(random_.randbytes(0x1220))
        struct.pack_into("<i", world, 0x8, random_.choice((1, 2, 2, 3, 4, 5)))
        world[0x2E] = random_.choice((0, 1, 1))
        time = bytearray(random_.randbytes(0x10))
        playing = random_.choice((0, 1, 1, 0x100))
        two = lambda harness, arguments: (0, (harness.normalize(arguments[0]), harness.normalize(arguments[1])))
        t.check(
            "simulation_world_build_update",
            Buffer(bytes(world)),
            Buffer(random_.randbytes(0x4048)),
            globals={
                "game_time_globals": Buffer(bytes(time)),
                "global_random_seed_reference": Buffer(random_.randbytes(4)),
                "unknown_004cf780": 0x4321,
            },
            externals={
                "simulation_world_take_update": two,
                "simulation_take_unknown_004cf780_change": two,
                "unknown_00083610": lambda harness, arguments: (0, (arguments[0], harness.normalize(arguments[1]), harness.normalize(arguments[2]))),
                "simulation_in_progress": lambda harness, arguments, playing=playing: playing,
                "unknown_00069110": two,
            },
        )
