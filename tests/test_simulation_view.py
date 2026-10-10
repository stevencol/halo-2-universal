# source/networking/simulation_view.c

import random
import struct

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(447)

    for n in range(2500):
        now = random_.choice((5000, 100000, 0xFFFFFF00, random_.getrandbits(32)))
        view = bytearray(random_.randbytes(0x90))
        struct.pack_into("<i", view, 0x28, random_.choice((0, 0, 0, 1, 4, 7)))
        view[0x88] = random_.choice((0, 1, 1, 0x80))
        struct.pack_into("<I", view, 0x8C, (now - random_.choice((0, 1, 1999, 2000, 2001, 60000, -5))) & 0xFFFFFFFF)
        kept = random_.choice((0, 1, 0x80))
        t.check(
            "simulation_view_set_blocked",
            Buffer(bytes(view)),
            random_.choice((0, 1, 1, 1, 0x100)),
            globals={"unknown_00510548": bytes((kept,)), "unknown_0051054c": now if kept else random_.getrandbits(32)},
            externals={
                "GetTickCount": lambda harness, arguments, now=now: now,
                "simulation_view_set_state": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1], arguments[2])),
            },
        )

    # ---------- a view goes to a state

    for n in range(3000):
        view = bytearray(random_.randbytes(0x90))
        at = random_.choice((0, 1, 2, 3, 4, 5, 6))
        with_ = random_.choice((-1, 0, 5, random_.getrandbits(31)))
        struct.pack_into("<i", view, 0x28, random_.choice((0, 0, 0, 1, 7)))
        struct.pack_into("<2i", view, 0x2C, at, with_)
        struct.pack_into("<Ii", view, 0x20, 0x12345678, random_.choice((-1, 0, 3, 3)))
        t.check(
            "simulation_view_set_state",
            Buffer(bytes(view)),
            random_.choice((0, 1, 2, 3, at, at + 1, at + 1, at + 2, 7, 0xFFFFFFFF)),
            random_.choice((0xFFFFFFFF, 0xFFFFFFFF, 0, 5, with_ & 0xFFFFFFFF, with_ & 0xFFFFFFFF, 0x80000000, random_.getrandbits(32))),
            externals={
                "network_observer_channel_send": lambda harness, arguments: (
                    0,
                    (arguments[0] & 0xFF, arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], bytes(harness.mu.mem_read(arguments[6], 8))),
                ),
                "simulation_view_update_ready": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
            },
        )

    # ---------- a view that cannot go on, what the players of the other console do, and the state of the game

    VIEW, WORLD = 0x90, 0x1220

    def a_view() -> Buffer:
        """A view, and after it the thing that it is of."""
        data = bytearray(random_.randbytes(VIEW + WORLD))
        struct.pack_into("<I", data, 0xC, VIEW)
        struct.pack_into("<i", data, 0x28, random_.choice((0, 0, 0, 1, 7)))
        struct.pack_into("<2i", data, 0x80, random_.choice((0, 5, 100)), random_.choice((0, 5, 100)))
        struct.pack_into("<i", data, VIEW + 0x8, random_.choice((0, 1, 2, 3, 3, 3, 3, 4, 5)))
        struct.pack_into("<i", data, VIEW + 0x18, random_.choice((0, 4, 4, 4)))
        struct.pack_into("<i", data, VIEW + 0x28, random_.choice((0, 6, 101, 200)))
        struct.pack_into("<i", data, VIEW + 0x1200, random_.choice((-1, 0, 0, 100)))
        return Buffer(bytes(data), pointers=(0xC,))

    state = lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1], arguments[2]))

    for n in range(2500):
        t.check("simulation_view_fail", a_view(), random_.choice((2, 4, 5, 7)), externals={"simulation_view_set_state": state})
        t.check(
            "simulation_view_unknown_00085f50",
            a_view(),
            random_.choice((0, 0, 0, 0, 1, 0x80)),
            random_.choice((0, 4, 5, 6, 99, 100, 101, 199, 200, 0xFFFFFFFF)),
            random_.choice((0, 4, 5, 6, 99, 100, 101, 0xFFFFFFFF)),
            random_.getrandbits(4),
            Buffer(random_.randbytes(4 * 0x5C)),
            externals={
                "simulation_view_set_state": state,
                "simulation_world_set_player_actions": lambda harness, arguments: (
                    0,
                    (harness.normalize(arguments[0]), harness.normalize(arguments[1]), arguments[2], harness.normalize(arguments[3])),
                ),
            },
        )
        could = random_.choice((0, 1, 1, 0x100))
        t.check(
            "simulation_view_unknown_00085ed0",
            a_view(),
            random_.choice((0, 100, 0x3FE000)),
            random_.choice((0, 0, 1, 100, 0xFFFFFFFF, 0x80000000)),
            Buffer(random_.randbytes(0x80)),
            externals={
                "simulation_view_set_state": state,
                "simulation_world_receive_game_state": lambda harness, arguments, could=could: (
                    could,
                    (harness.normalize(arguments[0]), arguments[1], arguments[2], harness.normalize(arguments[3])),
                ),
                "simulation_world_finish_game_state": lambda harness, arguments, could=could: (could, (harness.normalize(arguments[0]), arguments[1])),
            },
        )

    # ---------- the number that the game goes on from

    for n in range(2500):
        view = a_view()
        data = bytearray(view.data)
        struct.pack_into("<i", data, VIEW + 0x18, random_.choice((2, 3, 3, 3, 3, 4)))
        data[VIEW + 0x24] = random_.choice((0, 1))
        could = random_.choice((0, 1, 1, 0x100))
        t.check(
            "simulation_view_unknown_00085dc0",
            Buffer(bytes(data), pointers=view.pointers),
            random_.choice((0, 5, 1000, 0x80000000, random_.getrandbits(32))),
            externals={
                "simulation_view_set_state": state,
                "simulation_world_begin_game_state": lambda harness, arguments, could=could: (could, (harness.normalize(arguments[0]),)),
                "simulation_world_clear_updates": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                "simulation_world_unknown_00069350": lambda harness, arguments: (0, (arguments[0] & 0xFF, harness.normalize(arguments[1]))),
            },
        )

    # ---------- the players that the other console knows of

    for n in range(2500):
        view = a_view()
        data = bytearray(view.data)
        struct.pack_into("<i", data, 0x3C, random_.choice((-1, 0, 0, 0, 3)))
        data[0x75] = random_.choice((0, 1, 1, 1, 0x80))
        struct.pack_into("<H", data, 2, random_.choice((2, 2, 4, 1)))
        struct.pack_into("<i", data, 0x2C, random_.choice((0, 2, 3, 4, 5, 6)))
        pattern = random_.getrandbits(16)
        needed = random_.choice((0, 0, random_.getrandbits(16), random_.getrandbits(16) & random_.getrandbits(16)))
        t.check(
            "simulation_view_set_players",
            Buffer(bytes(data), pointers=view.pointers),
            random_.choice((0xFFFF, random_.getrandbits(32))),
            random_.choice((0xFFFF, random_.getrandbits(32))),
            Buffer(random_.randbytes(16 * 12)),
            externals={
                "simulation_view_set_state": state,
                "simulation_world_player_matches": lambda harness, arguments, pattern=pattern: (
                    pattern >> (arguments[0] & 15) & 1,
                    (arguments[0], harness.normalize(arguments[1]), harness.normalize(arguments[2])),
                ),
                "simulation_world_players_unknown25_mask": lambda harness, arguments, needed=needed: (needed, (harness.normalize(arguments[0]),)),
            },
        )

    # ---------- the state of the other end of a view, and one of the things that come

    sends = lambda harness, arguments: (
        0,
        (arguments[0] & 0xFF, arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], bytes(harness.mu.mem_read(arguments[6], arguments[5]))),
    )
    told = lambda harness, arguments: (0, (harness.normalize(arguments[0]),))

    for n in range(6000):
        view = a_view()
        data = bytearray(view.data)
        there = random_.random() < 0.95
        struct.pack_into("<I", data, 0xC, VIEW if there else 0)
        struct.pack_into("<i", data, 0x3C, random_.choice((-1, 0, 0, 0, 0, 3)))
        struct.pack_into("<H", data, 2, random_.choice((1, 2, 2, 3, 4)))
        own = random_.choice((0, 1, 2, 2, 3, 4, 5, 6))
        own_number = random_.choice((-1, 7, 7))
        was = random_.choice((0, 1, 2, 3, 4, own, own - 1))
        struct.pack_into("<2i", data, 0x2C, own, own_number)
        struct.pack_into("<2i", data, 0x34, was, random_.choice((-1, 7, 7, 8)))
        struct.pack_into("<Ii", data, 0x20, 0x12345678, random_.choice((-1, 0, 3, 3)))
        t.check(
            "simulation_view_handle_remote_state",
            Buffer(bytes(data), pointers=view.pointers if there else ()),
            random_.choice((0, 1, 2, 2, 3, 4, 5, was + 1, was + 1, own, own + 1)) & 0xFFFFFFFF,
            random_.choice((-1, 7, 7, 7, 8)) & 0xFFFFFFFF,
            externals={
                "simulation_view_set_state": state,
                "simulation_view_fail": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
                "network_observer_channel_send": sends,
                "simulation_view_update_ready": told,
            },
        )

    for n in range(400):
        view = a_view()
        data = bytearray(view.data) + bytearray(8)
        struct.pack_into("<i", data, 0x2C, random_.choice((5, 5, 5, 0, 3, 4, 6)))
        struct.pack_into("<Ii", data, 0x20, 0x12345678, random_.choice((-1, 0, 3, 3)))
        data += bytearray(8)
        struct.pack_into("<I", data, 0x94, random_.choice((0, 1, 1, 1)))
        could = random_.choice((0, 1, 1, 0x100))
        t.check(
            "simulation_view_send_update",
            Buffer(bytes(data), pointers=view.pointers),
            Buffer(random_.randbytes(0x4048)),
            externals={
                "simulation_view_set_state": state,
                "network_observer_channel_send": sends,
                "unknown_000869a0": lambda harness, arguments, could=could: (could, (harness.normalize(arguments[0]), harness.normalize(arguments[1]))),
                "unknown_000865d0": told,
            },
        )

    # ---------- what follows from the two states of a view

    for n in range(5000):
        view = a_view()
        data = bytearray(view.data) + bytearray(random_.randbytes(0x30))
        there = random_.random() < 0.95
        struct.pack_into("<I", data, 0xC, VIEW if there else 0)
        struct.pack_into("<I", data, 0x8, 0x200000)
        struct.pack_into("<i", data, 0x3C, random_.choice((-1, 0, 0, 0, 0, 3)))
        struct.pack_into("<H", data, 2, random_.choice((1, 2, 2, 3, 4, 5, 0x102)))
        struct.pack_into("<2i", data, 0x2C, random_.choice((0, 1, 2, 3, 4, 5, 6)), random_.choice((-1, 7, 7)))
        struct.pack_into("<2i", data, 0x34, random_.choice((0, 1, 2, 3, 4, 5, 6)), random_.choice((-1, 7, 7, 7)))
        data[0x75] = random_.choice((0, 1, 1, 0x80))
        data[0x78] = random_.choice((0, 1, 1, 0x80))
        buffer = bytearray(data[:VIEW]) + bytearray(random_.randbytes(0x30)) + bytearray(data[VIEW:])
        # the view is longer here: the thing goes after what it has up to 0xC0
        struct.pack_into("<I", buffer, 0xC, VIEW + 0x30 if there else 0)
        struct.pack_into("<I", buffer, 0x94, random_.choice((0, 0x5000, 0x5000)))
        byte_and = lambda harness, arguments: (0, (arguments[0] & 0xFF, arguments[1]))
        t.check(
            "simulation_view_update_ready",
            Buffer(bytes(buffer), pointers=(0xC,) if there else ()),
            externals={
                "texture_cache_return_memory": lambda harness, arguments: 0,
                "simulation_view_events_set_ready": byte_and,
                "simulation_world_unknown_0006b090": lambda harness, arguments: (0, (arguments[0] & 0xFF, harness.normalize(arguments[1]), harness.normalize(arguments[2]))),
                "simulation_world_unknown_0006b110": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1] & 0xFF, harness.normalize(arguments[2]))),
            },
        )
        t.check(
            "simulation_view_unknown_00086590",
            Buffer(bytes(buffer), pointers=(0xC,) if there else ()),
            externals={"texture_cache_return_memory": lambda harness, arguments: 0},
        )

    # ---------- whether a view has more to send

    for n in range(1500):
        view = bytearray(random_.randbytes(0xC0))
        struct.pack_into("<H", view, 2, random_.choice((1, 2, 2, 3, 4, 0x103)))
        struct.pack_into("<I", view, 0x94, random_.choice((0, 0x5000, 0x5000)))
        struct.pack_into("<i", view, 0xAC, random_.choice((0, 1, 100, -1)))
        more = random_.choice((0, 1, 0x100))
        t.check(
            "simulation_view_nothing_to_send",
            Buffer(bytes(view)),
            externals={"simulation_view_entities_pending": lambda harness, arguments, more=more: (more, (harness.normalize(arguments[0]),))},
        )

    # ---------- a view says what the players of this console do

    for n in range(2000):
        data = bytearray(random_.randbytes(0xC0 + 0x40))
        struct.pack_into("<I", data, 0xC, 0xC0)
        data[0x78] = random_.choice((0, 1, 1, 1, 0x80))
        struct.pack_into("<Ii", data, 0x20, 0x12345678, random_.choice((-1, 0, 3, 3)))
        struct.pack_into("<I", data, 0xB0, random_.choice((0, 5, 0xFFFFFFFF)))
        struct.pack_into("<i", data, 0xC0 + 8, random_.choice((1, 2, 3, 3, 4, 5)))
        struct.pack_into("<I", data, 0xC0 + 0x28, random_.choice((0, 1, 1000)))
        data[0xC0 + 0x2C] = random_.choice((0, 1, 0x80))
        t.check(
            "simulation_view_send_player_actions",
            Buffer(bytes(data), pointers=(0xC,)),
            random_.choice((0xF, random_.getrandbits(4), random_.getrandbits(32))),
            Buffer(random_.randbytes(4 * 0x5C)),
            externals={"network_observer_channel_send": sends},
        )

    # ---------- whether a view has things of the game left to tell of

    from difftest import Mock, mock_address

    OBJECT, THING, UNKNOWN4, DATABASE = 0x40, 0x80, 0x90, 0x90 + 0x2098
    ENTITIES = DATABASE + 0x14
    CLIENTS = ENTITIES + 0x400 * 0x20
    CLIENT = CLIENTS + 9 * 4
    FUNCTIONS = CLIENT + 4

    for n in range(300):
        data = bytearray(random_.randbytes(FUNCTIONS + 7 * 4))
        struct.pack_into("<2I", data, 0x8, OBJECT, THING)
        data[OBJECT + 0x39] = random_.choice((0, 1, 1, 1, 0x80))
        struct.pack_into("<I", data, THING + 4, UNKNOWN4)
        struct.pack_into("<I", data, DATABASE + 0x10, CLIENTS)
        pointers = [0x8, 0xC, THING + 4, DATABASE + 0x10, CLIENT]
        for kind in range(8):
            struct.pack_into("<I", data, CLIENTS + 4 + kind * 4, CLIENT)
            pointers.append(CLIENTS + 4 + kind * 4)
        struct.pack_into("<I", data, CLIENT, FUNCTIONS)
        struct.pack_into("<7I", data, FUNCTIONS, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, mock_address(0))
        # the things that the view goes through, each after the one before, and which of them has something
        order = random_.sample(range(0x400), random_.choice((0, 1, 3, 8)))
        has = random_.choice((None, None, 0, len(order) - 1, 1))
        for place, entity in enumerate(order):
            struct.pack_into("<h", data, ENTITIES + entity * 0x20 + 4, random_.randrange(8))
            data[ENTITIES + entity * 0x20] = 1 if place == has else 0
        after = {0xFFFFFFFF: order[0] | 0x50000 if order else 0xFFFFFFFF}
        for place, entity in enumerate(order):
            after[entity | 0x50000] = order[place + 1] | 0x50000 if place + 1 < len(order) else 0xFFFFFFFF
        t.check(
            "simulation_view_entities_pending",
            Buffer(bytes(data), pointers=tuple(pointers)),
            externals={"replication_entity_view_next_entity": lambda harness, arguments, after=after: (after[arguments[0]], (arguments[0], harness.normalize(arguments[1])))},
            mocks=[Mock(lambda call: t.harness.mu.mem_read(call.stack[0], 1)[0], stack_arguments=1, registers=("ecx",))],
        )

    # ---------- what a view has at 0x6060 of what it has at 0x8 is told whether the view is ready

    for n in range(600):
        data = bytearray(random_.randbytes(0x4CD0 + 8))
        struct.pack_into("<I", data, 0, 0x4CD0)
        data[0x4CD0 + 2] = random_.choice((1, 2, 3, 4, 0x22))
        data[5] = random_.choice((0, 1))
        data[6] = random_.choice((0, 1))
        t.check(
            "simulation_view_events_set_ready",
            random_.choice((0, 1, 1, 0x100)),
            Buffer(bytes(data), pointers=(0,)),
            globals={"unknown_00510ca2": bytes((random_.choice((0, 1)),))},
        )

    # ---------- the state of the game goes into the ring of a view

    for n in range(1500):
        view = bytearray(random_.randbytes(0xC0))
        struct.pack_into("<H", view, 2, random_.choice((2, 2, 2, 1, 4)))
        memory = random_.choice((0, 0x5000, 0x5000, 0x6000))
        struct.pack_into("<I", view, 0x94, memory)
        t.check(
            "simulation_view_reclaim_game_state",
            random_.choice((0x5000, 0x5000, 0x6000, 0)),
            Buffer(bytes(view)),
            externals={"simulation_view_unknown_00086590": told},
        )
        room = random_.choice((0x10, 0x40, 0x500, 0x1000, 0x1000))
        kept = random_.choice((0, 0, 8, room // 2))
        struct.pack_into("<iI2i", view, 0x9C, room, 0xC0, random_.randrange(room), kept)
        struct.pack_into("<i", view, 0xAC, random_.choice((0, 3)))
        packed = random_.choice((0, 5, 0x400, 0x401, 0x850, -1))
        good = random_.choice((1, 1, 1, 0, 0x100))

        def packs(harness, arguments, packed=packed, good=good):
            harness.mu.mem_write(arguments[2], struct.pack("<i", packed))
            return good, (arguments[0], harness.normalize(arguments[1]), arguments[3], arguments[4])

        t.check(
            "simulation_view_queue_game_state",
            0x40000,
            Buffer(bytes(view) + random_.randbytes(room + 4), pointers=(0xA0,)),
            0x3FE000,
            Buffer(random_.randbytes(0x900)),
            globals={"game_state_globals+0x2520": 0x61000},
            externals={"unknown_001995a0": packs},
        )

    # ---------- a view starts to send the state of the game

    for n in range(400):
        data = bytearray(random_.randbytes(0xC0 + 0x1220))
        struct.pack_into("<I", data, 0xC, 0xC0)
        struct.pack_into("<i", data, 0x90, random_.choice((0, 2)))
        cache = bytearray(random_.randbytes(0x40))
        # a cache with no pages lends nothing; how long the game would wait for one that has none free is a matter of
        # the clock of the processor
        struct.pack_into("<i", cache, 0x30, random_.choice((0, 100, 100, 100, 100)))
        ring_memory = Buffer(random_.randbytes(0x40))
        queued = random_.choice((0, 1, 1, 1, 0x100))

        def lends(harness, arguments, ring_memory=ring_memory):
            first = not any(call[0] == "texture_cache_steal_memory_now" for call in harness._calls)
            return (ring_memory.address if first else 0x700000), (arguments[0], harness.normalize(arguments[1]), arguments[2], arguments[3] != 0)

        def before(harness, arguments):
            harness.mu.mem_write(arguments[1], struct.pack("<I", 0x3FE000))
            return 0, (harness.normalize(arguments[0]),)

        t.check(
            "simulation_view_begin_send_game_state",
            Buffer(bytes(data), pointers=(0xC,)),
            extra=[ring_memory],
            globals={
                "texture_cache_globals+0x10": Buffer(bytes(cache)),
                "game_state_after_save_procs": struct.pack("<I", mock_address(0)),
            },
            externals={
                "texture_cache_steal_memory_now": lends,
                "texture_cache_idle": lambda harness, arguments: 0,
                "texture_cache_return_memory": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                "simulation_world_before_send_game_state": before,
                # the size is not compared: the function does nothing with it, and the compiler of source/, which
                # has it in the same file, does not set it
                "simulation_view_queue_game_state": lambda harness, arguments, queued=queued: (
                    queued,
                    (arguments[0], harness.normalize(arguments[1]), arguments[3]),
                ),
            },
            mocks=[Mock(lambda call: 0, stack_arguments=1)],
        )
