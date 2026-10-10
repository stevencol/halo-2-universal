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
        struct.pack_into("<I", view, 0x1C, random_.choice((0, 1, 2, 3)))
        view[0x14:0x1A] = bytes((random_.choice((1, 2)),)) * 5 + bytes((random_.choice((1, 2, 3)),))
        struct.pack_into("<i", view, 0x28, random_.choice((0, 0, 0, 4)))
        view[0x78] = random_.choice((0, 1, 1, 0x80))
        view[0x75] = random_.choice((0, 1, 1, 0x80))
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
                "simulation_world_add_update": lambda harness, arguments, could=could: could,
            },
        )
        view = bytearray(random_.randbytes(0x40))
        struct.pack_into("<i", view, 0x2C, random_.choice((0, 1, 1, 1, 2, 3)))
        struct.pack_into("<i", view, 0x34, random_.choice((0, 1, 1, 2)))
        t.check(
            "simulation_world_unknown_0006a2a0",
            Buffer(bytes(view)),
            Buffer(random_.randbytes(0x40)),
            externals={"simulation_view_set_state": lambda harness, arguments: 0},
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

    # ---------- a view by what it is known by, the first thing that waits, and the first view on its way

    from difftest import Mock, mock_address

    NODE = 0x404C
    HEAP, ALLOCATOR, TABLE, HEAP_SIZE = 0x0, 0x8, 0xC, 0x40

    for n in range(1500):
        t.check("simulation_world_get_view_by_unknown1c", a_world(random_, 0), random_.choice((0, 1, 2, 3, 7)))
        t.check(
            "simulation_world_get_view_by_unknown14",
            a_world(random_, 0),
            Buffer(bytes((random_.choice((1, 2)),)) * 5 + bytes((random_.choice((1, 2, 3)),)) + random_.randbytes(2)),
        )
        t.check(
            "simulation_world_unknown_00069eb0",
            a_world(random_, random_.randrange(4)),
            externals={
                "simulation_view_set_state": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1], arguments[2])),
                "simulation_world_unknown_0006b1f0": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
            },
        )

    for n in range(300):
        world = bytearray(random_.randbytes(0x1220))
        node = len(world)
        world += random_.randbytes(NODE)
        last = random_.choice((True, True, False))
        struct.pack_into("<2I", world, 0x1218, node, node if last else 0x1234)
        struct.pack_into("<i", world, 0x1214, random_.choice((1, 2, 5)))
        struct.pack_into("<i", world, 0x120C, random_.choice((0, 5, 1000)))
        heap = bytearray(random_.randbytes(HEAP_SIZE))
        struct.pack_into("<Ii", heap, HEAP, ALLOCATOR, random_.choice((1, 5, 5000)))
        struct.pack_into("<I", heap, ALLOCATOR, TABLE)
        struct.pack_into("<2I", heap, TABLE, mock_address(0), mock_address(1))
        memory = Buffer(bytes(heap), pointers=(HEAP, ALLOCATOR))
        t.check(
            "simulation_world_take_update",
            Buffer(bytes(world), pointers=(0x1218, 0x121C) if last else (0x1218,)),
            Buffer(random_.randbytes(0x4048 + 8)),
            extra=[memory],
            globals={"unknown_004d87f8": Pointer(memory, HEAP)},
            mocks=[
                Mock(lambda call: 0, stack_arguments=2, registers=("ecx",)),
                # where the size goes is in the stack
                Mock(lambda call, known=random_.choice((0, 1)): known, stack_arguments=2, registers=("ecx",), record=lambda call: (call.registers["ecx"], call.stack[0])),
            ],
        )

    # ---------- the thing starts to go, and what it is told of a view

    for n in range(2000):
        world = bytearray(random_.randbytes(0x1220))
        world[0x24] = random_.choice((0, 0, 1, 0x80))
        struct.pack_into("<i", world, 0x8, random_.choice((0, 2, 3, 3, 3, 4)))
        struct.pack_into("<i", world, 0x18, random_.choice((2, 3, 4, 4, 5)))
        struct.pack_into("<I", world, 0x28, random_.choice((0, 5, 1000, 0x80000000)))
        kept = random_.choice((0, 1, 0x80))
        now = random_.getrandbits(32)
        t.check(
            "simulation_world_unknown_0006b2a0",
            Buffer(bytes(world)),
            globals={"unknown_00510548": bytes((kept,)), "unknown_0051054c": now if kept else random_.getrandbits(32)},
            externals={
                "GetTickCount": lambda harness, arguments, now=now: now,
                "simulation_world_clear_updates": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
            },
        )
        thing = a_world(random_, 0)
        data = bytearray(thing.data)
        struct.pack_into("<i", data, 0x18, random_.choice((2, 3, 3, 3, 4)))
        views = [struct.unpack_from("<I", data, 0x40 + index * 4)[0] for index in range(15)]
        views = [view for view in views if view]
        first = [view for view in views if 1 << (data[view + 2] & 31) & 0xA]
        of = random_.choice(((first[0] if first else 0), (first[0] if first else 0), (random_.choice(views) if views else 0), 0))
        thing = Buffer(bytes(data), pointers=thing.pointers)
        t.check(
            "simulation_world_unknown_0006b110",
            thing,
            random_.choice((0, 1, 1, 1, 0x100)),
            Pointer(thing, of) if of else 0,
            externals={"simulation_world_unknown_00069f10": lambda harness, arguments: (0, (harness.normalize(arguments[0]),))},
        )

    # ---------- the number that the thing has at 0x18

    for n in range(2500):
        world = bytearray(random_.randbytes(0x1220))
        world[0x24] = random_.choice((0, 1, 1, 0x80))
        struct.pack_into("<i", world, 0x8, random_.choice((0, 2, 3, 3, 3, 4, 5)))
        struct.pack_into("<i", world, 0x18, random_.choice((1, 2, 3, 3, 4, 4, 5)))
        struct.pack_into("<i", world, 0x1200, random_.choice((-1, -1, 0, 100)))
        kept = random_.choice((0, 1, 0x80))
        now = random_.getrandbits(32)
        setup = dict(
            globals={"unknown_00510548": bytes((kept,)), "unknown_0051054c": now if kept else random_.getrandbits(32)},
            externals={
                "GetTickCount": lambda harness, arguments, now=now: now,
                "simulation_world_clear_updates": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                "texture_cache_return_memory": lambda harness, arguments: 0,
            },
        )
        t.check("simulation_world_set_unknown18", Buffer(bytes(world)), random_.choice((1, 2, 3, 4, 4, 5, 6)), **setup)
        t.check("simulation_world_unknown_0006b1f0", Buffer(bytes(world)), random_.getrandbits(32), **setup)

    # ---------- the thing gives up

    views_done = lambda harness, arguments: (0, (harness.normalize(arguments[0]),))
    next_number = lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1]))

    for n in range(2500):
        world = bytearray(random_.randbytes(0x1220))
        struct.pack_into("<i", world, 0x18, random_.choice((0, 1, 2, 3, 4, 5, 6, 7)))
        struct.pack_into("<i", world, 0x30, random_.choice((0, 1, 4, 5, 6)))
        kept = random_.choice((0, 1, 0x80))
        now = random_.getrandbits(32)
        struct.pack_into("<I", world, 0x34, (now - random_.choice((0, 1, 999, 1000, 1001, 60000))) & 0xFFFFFFFF)
        setup = dict(
            globals={
                "unknown_00510548": bytes((kept,)),
                "unknown_0051054c": now if kept else random_.getrandbits(32),
                "unknown_004ced58": struct.pack("<i", 5),
                "unknown_004ced5c": struct.pack("<i", 1000),
            },
            externals={
                "GetTickCount": lambda harness, arguments, now=now: now,
                "unknown_000693a0": views_done,
                "simulation_world_set_unknown18": next_number,
            },
        )
        t.check("simulation_world_unknown_0006b040", Buffer(bytes(world)), **setup)
        setup["externals"]["simulation_world_unknown_0006b040"] = views_done
        t.check("simulation_world_unknown_00069f90", Buffer(bytes(world)), **setup)
        thing = a_world(random_, 0)
        data = bytearray(thing.data)
        struct.pack_into("<i", data, 0x18, random_.choice((2, 3, 3, 4)))
        t.check(
            "simulation_world_unknown_00069fe0",
            Buffer(bytes(data), pointers=thing.pointers),
            externals={"simulation_world_unknown_0006b040": views_done},
        )

    # ---------- the thing is told that a view is ready

    for n in range(2000):
        thing = a_world(random_, 0)
        data = thing.data
        views = [struct.unpack_from("<I", data, 0x40 + index * 4)[0] for index in range(15)]
        views = [view for view in views if view]
        first = [view for view in views if 1 << (data[view + 2] & 31) & 0xA]
        of = random_.choice(((first[0] if first else 0), (first[0] if first else 0), (random_.choice(views) if views else 0), 0))
        there = bytearray(random_.randbytes(0x10))
        there[8] = random_.choice((0, 1, 0x80))
        struct.pack_into("<I", there, 0xC, 0)
        t.check(
            "simulation_world_unknown_0006b090",
            random_.choice((0, 1, 1, 1, 0x100)),
            thing,
            Pointer(thing, of) if of else 0,
            globals={"unknown_004cf770": bytes(there)},
            externals={"unknown_0006a560": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1] & 0xFF))},
        )

    # ---------- the things that wait are done with, one more comes, a piece of the state of the game, and the first view

    ASKED, ANSWERS, GIVEN = 0x38, 0x3C, 0x40

    def a_heap(room: int = 0) -> Buffer:
        heap = bytearray(random_.randbytes(GIVEN + room))
        struct.pack_into("<Ii", heap, HEAP, ALLOCATOR, random_.choice((1, 5, 5000)))
        struct.pack_into("<I", heap, ALLOCATOR, TABLE)
        struct.pack_into("<11I", heap, TABLE, mock_address(0), mock_address(1), 0x33, 0x44, 0x55, mock_address(2), 0x77, 0x88, 0x99, 0xAA, mock_address(3))
        struct.pack_into("<I4B", heap, ASKED, 0, *random_.choice(((1, 1, 1, 1), (1, 1, 1, 1), (0, 1, 1, 1), (0, 0, 0, 0))))
        return Buffer(bytes(heap), pointers=(HEAP, ALLOCATOR))

    def a_heap_mocks() -> list:
        def gives(call):
            mu = t.harness.mu
            base = call.registers["ecx"] - ALLOCATOR
            asked = struct.unpack("<I", mu.mem_read(base + ASKED, 4))[0]
            mu.mem_write(base + ASKED, struct.pack("<I", asked + 1))
            return base + GIVEN if mu.mem_read(base + ANSWERS + (asked & 3), 1)[0] else 0

        return [
            Mock(lambda call: 0, stack_arguments=2, registers=("ecx",)),
            Mock(lambda call, known=random_.choice((0, 1)): known, stack_arguments=2, registers=("ecx",), record=lambda call: (call.registers["ecx"], call.stack[0])),
            Mock(gives, stack_arguments=3, registers=("ecx",)),
            Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
        ]

    for n in range(300):
        world = bytearray(random_.randbytes(0x1220))
        pointers = []
        where = 0x1218
        for node in range(random_.choice((0, 1, 2, 3))):
            at = len(world)
            world += random_.randbytes(NODE)
            struct.pack_into("<I", world, where, at)
            pointers.append(where)
            where = at + 0x4048
        struct.pack_into("<I", world, where, 0)
        memory = a_heap()
        t.check(
            "simulation_world_clear_updates",
            Buffer(bytes(world), pointers=tuple(pointers)),
            extra=[memory],
            globals={"unknown_004d87f8": Pointer(memory, HEAP)},
            mocks=a_heap_mocks(),
        )
        world = bytearray(random_.randbytes(0x1220))
        last = random_.choice((True, False))
        if last:
            struct.pack_into("<I", world, 0x121C, len(world))
            world += random_.randbytes(NODE)
        else:
            struct.pack_into("<I", world, 0x121C, 0)
        memory = a_heap(NODE + 4)
        t.check(
            "simulation_world_add_update",
            Buffer(bytes(world), pointers=(0x121C,) if last else ()),
            Buffer(random_.randbytes(0x4048 + 8)),
            extra=[memory],
            globals={"unknown_004d87f8": Pointer(memory, HEAP)},
            mocks=a_heap_mocks(),
        )

    for n in range(2500):
        world = bytearray(random_.randbytes(0x1220))
        struct.pack_into("<i", world, 0x8, random_.choice((0, 2, 3, 3, 3, 3, 4, 5)))
        come = random_.choice((-1, 0, 0, 0x40, 0x100))
        there = random_.choice((True, True, True, False))
        struct.pack_into("<iI", world, 0x1200, come, 0x1220 if there else 0)
        world += random_.randbytes(0x300)
        t.check(
            "simulation_world_receive_game_state",
            Buffer(bytes(world), pointers=(0x1204,) if there else ()),
            random_.choice((come, come, come, come + 1, 0, 0xFFFFFFFF)) & 0xFFFFFFFF,
            random_.choice((1, 4, 0x33, 0x100, 0, 0xFFFFFFFF, 0x80000000, 0x40001, 0x50000)),
            Buffer(random_.randbytes(0x104)),
        )
        thing = a_world(random_, 0)
        data = bytearray(thing.data)
        kept = random_.choice((0, 1, 0x80))
        now = random_.getrandbits(32)
        struct.pack_into("<I", data, 0x1C, (now - random_.choice((0, 1, 4999, 5000, 5001, 100000))) & 0xFFFFFFFF)
        t.check(
            "simulation_world_unknown_00069f10",
            Buffer(bytes(data), pointers=thing.pointers),
            globals={
                "unknown_00510548": bytes((kept,)),
                "unknown_0051054c": now if kept else random_.getrandbits(32),
                "unknown_004ced4c": struct.pack("<i", 5000),
            },
            externals={
                "GetTickCount": lambda harness, arguments, now=now: now,
                "simulation_world_unknown_0006b2a0": views_done,
                "simulation_world_unknown_0006b040": views_done,
            },
        )
