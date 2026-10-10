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
                "unknown_00085880": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
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
