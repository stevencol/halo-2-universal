# source/networking/voice_routing.c

import random
import struct

from difftest import Buffer, GlobalData, GlobalRange

NONE = 0xFFFFFFFF
VOICE_SIZE = 0x90
VOICE_INITIALIZED = 4
QUEUES_SIZE = 0x1704
QUEUE_SIZE = 0x170
CHANNELS_SIZE = 0xC4


def run(t) -> None:
    random_ = random.Random(279)

    def active():
        yield 1, 1
        yield 0x80, 0x80
        yield 0, 1
        yield 1, 0

    def state(enabled: int, initialized: int) -> dict:
        data = bytearray(random_.randbytes(VOICE_SIZE))
        data[VOICE_INITIALIZED] = initialized
        return {"game_voice_enabled": bytes((enabled,)), "voice_globals": GlobalData(bytes(data))}

    def gives(value: object):
        return lambda harness, arguments: value

    # ---------- who each player talks to

    for on in (1, 0, 0x80):
        for player in (0, 7, 15):
            for local in (1 << player, 0xFFFFFFFF ^ (1 << player), 0xFFFF):
                data = bytearray(random_.randbytes(0x48))
                data[0] = on
                t.check(
                    "voice_targets_get",
                    Buffer(bytes(data)),
                    player,
                    externals={"game_voice_get_local_players": gives(local)},
                )

    # ---------- the queues

    def queues(on: int) -> bytearray:
        data = bytearray(random_.randbytes(QUEUES_SIZE))
        data[0] = on
        return data

    for on in (1, 0, 0x80):
        t.check("voice_queues_reset", Buffer(bytes(queues(on))))

        for player in (0, 9, 15):
            for size in (10, 9, 11, 0, 0x10A):
                t.check(
                    "voice_queues_add",
                    player,
                    Buffer(bytes(queues(on))),
                    0x80345000,
                    0x80346000,
                    size,
                    0x80347000,
                    externals={"voice_queue_add": gives(0)},
                )

        for peer in (0, 5, 15):
            for case in range(8):
                data = queues(on)
                for player in range(16):
                    at = 4 + player * QUEUE_SIZE
                    data[at] = 0
                    struct.pack_into("<H", data, at + 8, 0xFFFF)
                if case:
                    # one queue in use; it goes to the console, or not
                    player = random_.choice((0, 1, 2, 3, 7, 15))
                    at = 4 + player * QUEUE_SIZE
                    data[at] = random_.choice((1, 0x80))
                    struct.pack_into("<H", data, at + 8, (1 << peer) if case % 2 else 0xFFFF ^ (1 << peer))
                t.check("voice_queues_go_to_peer", peer, Buffer(bytes(data)))

    # ---------- who hears whom

    def channels(on: int) -> bytearray:
        data = bytearray(random_.randbytes(CHANNELS_SIZE))
        data[0] = on
        return data

    def stage(harness, arguments):
        return 0, (harness.normalize(arguments[0]),)

    for on in (1, 0, 0x80):
        for player in (0, 4, 15):
            t.check("voice_channels_get_unknown84", Buffer(bytes(channels(on))), player)

        for enabled, initialized in active():
            for network in (1, 2, 3, 0, 4):
                t.check(
                    "voice_channels_update",
                    Buffer(bytes(channels(on))),
                    globals={**state(enabled, initialized), "unknown_004c9888": network},
                    externals={
                        name: gives(0)
                        for name in (
                            "voice_channels_update_ended",
                            "unknown_00057080",
                            "unknown_00057360",
                            "unknown_000577d0",
                            "unknown_00057960",
                        )
                    },
                )

            for network in (1, 1, 2, 0):
                for player in (0, 6, 15):
                    for is_one, has_datum, says in ((1, 1, 1), (1, 1, 0), (0, 1, 1), (1, 0, 1)):
                        for global_on in (1, 0):
                            t.check(
                                "voice_channels_get_player",
                                Buffer(bytes(channels(on))),
                                player,
                                globals={
                                    **state(enabled, initialized),
                                    "unknown_004c9888": network,
                                    "unknown_00527108": GlobalData(bytes(channels(global_on))),
                                },
                                externals={
                                    "game_voice_unknown_00054df0": gives(is_one),
                                    "game_voice_get_player_datum": gives(0x80345000 if has_datum else 0),
                                    "voice_player_unknown_000589e0": gives(says),
                                },
                            )

    # ---------- who each player talks to, each frame

    MEMBERSHIP_PLAYERS = 0x10D0

    for on in (1, 1, 1, 0):
        for enabled, initialized in active():
            for network in (1, 2, 3, 0):
                for n in range(6):
                    membership = bytearray(random_.randbytes(MEMBERSHIP_PLAYERS + 4))
                    struct.pack_into("<I", membership, MEMBERSHIP_PLAYERS, random_.choice((0xFFFF, random_.getrandbits(16), 0xFFFFFFFF)))
                    membership = Buffer(bytes(membership))
                    local = random_.choice((1, 0x8001, random_.getrandbits(16), 0xF))
                    # of each player: its flags, by the bits that are looked at
                    flags = [random_.choice((0, 0, 0, 2, 4, 6, 0xFFF9)) for _ in range(16)]
                    hears = [random_.choice((0xFFFF, 0xFFFF, random_.getrandbits(16))) for _ in range(16)]
                    data = bytearray(random_.randbytes(0x48))
                    data[0] = on
                    t.check(
                        "voice_targets_update",
                        Buffer(bytes(data)),
                        globals={
                            **state(enabled, initialized),
                            "unknown_004c9888": network,
                            "game_voice_talkers": struct.pack("<H", random_.choice((0xFFFF, local, random_.getrandbits(16)))),
                            "game_voice_player_mutes": struct.pack(
                                "<16H", *(random_.choice((0, 0, random_.getrandbits(16))) for _ in range(16))
                            ),
                            "unknown_00527108": GlobalRange(CHANNELS_SIZE),
                        },
                        externals={
                            "game_voice_get_membership": lambda harness, arguments, membership=membership, n=n: (
                                membership.address if n != 5 else 0
                            ),
                            "game_voice_get_local_players": gives(local),
                            "game_voice_player_flags": lambda harness, arguments, flags=flags: flags[arguments[0] & 15],
                            "voice_channels_get_player": lambda harness, arguments, hears=hears: (
                                hears[arguments[1] & 15],
                                (harness.normalize(arguments[0]), arguments[1]),
                            ),
                            "voice_channels_get_player_by_team": lambda harness, arguments, hears=hears: (
                                hears[arguments[1] & 15] ^ 0x5555,
                                (harness.normalize(arguments[0]), arguments[1]),
                            ),
                        },
                        extra=[membership],
                    )

    for on in (1, 0, 0x80):
        for targets in (0x55, 0, 0x80000000):
            for unknown4 in (7, 0):
                data = bytearray(random_.randbytes(4))
                data[0] = on
                t.check(
                    "voice_unknown_00527104_get_words",
                    targets,
                    Buffer(random_.randbytes(4)),
                    Buffer(bytes(data)),
                    externals={"voice_targets_to_peers": gives(unknown4), "voice_route_peers": gives(0)},
                )

    # ---------- a queue

    for active_ in (1, 0x80, 0):
        # a count that is not of a queue is not tried: the game writes where it takes it
        for count in (0, 1, 7, 14, 15, 16):
            for words in (0, 1, 0x8000, 0xFFFF, 0x12345678, 0xFFFFFFFF, 0x00010000):
                for size in (10, 0, 3, 9):
                    data = bytearray(random_.randbytes(QUEUE_SIZE + 0x10))
                    data[0] = active_
                    struct.pack_into("<I", data, 4, count)
                    t.check(
                        "voice_queue_add",
                        Buffer(struct.pack("<I", words)),
                        Buffer(bytes(data)),
                        Buffer(random_.randbytes(0x10)),
                        size,
                        Buffer(bytes((random_.choice((0, 1, 0x80)),))),
                        externals={"game_voice_get_local_peer": gives(random_.choice((0, 3, NONE)))},
                    )

    for on in (1, 0, 0x80):
        for enabled, initialized in active():
            for peer in (0, 5, 15):
                for chosen in (peer, 0, NONE, 7):
                    for goes in (1, 0):
                        data = queues(on)
                        for player in range(16):
                            at = 4 + player * QUEUE_SIZE
                            data[at] = random_.choice((0, 1, 1, 0x80))
                            struct.pack_into("<H", data, at + 8, random_.choice((0, 0xFFFF, 1 << peer, random_.getrandbits(16))))
                            struct.pack_into("<16I", data, at + 0x130, *(random_.randint(0, 15) for _ in range(16)))
                        t.check(
                            "voice_queues_size_for_peer",
                            Buffer(bytes(data)),
                            peer,
                            globals={**state(enabled, initialized), "unknown_004c9878": chosen},
                            externals={"voice_queues_go_to_peer": gives(goes)},
                        )

    # ---------- what goes from one console to another

    ENTRY_SIZE = 0x12

    for count in (0, 1, 2, 7, 15):
        for entries in (0, 0xFFFF, 1, 0x8000, 0x5555, 0xAAAA, 0xFFFF0000, 0x10001, random_.getrandbits(16), random_.getrandbits(16)):
            data = bytearray(random_.randbytes(QUEUE_SIZE))
            struct.pack_into("<I", data, 4, count)
            t.check("voice_queue_keep", Buffer(bytes(data)), entries)

    def nth(name: str, values: list):
        def handler(harness, arguments):
            calls = sum(1 for call in harness._calls if call[0] == name)
            # whether the console is the chosen one comes as a byte in a place of four
            return values[min(calls, len(values) - 1)], (*arguments[:-1], arguments[-1] & 0xFF)

        return handler

    for on in (1, 1, 1, 0):
        for enabled, initialized in active():
            for peer in (0, 9):
                for size in (0x200, 15, 14, 0x40):
                    for n in range(6):
                        data = queues(on)
                        for player in range(16):
                            at = 4 + player * QUEUE_SIZE
                            data[at] = random_.choice((0, 1, 1, 0x80))
                            struct.pack_into("<H", data, at + 8, random_.choice((0, 0xFFFF, 1 << peer, random_.getrandbits(16))))
                        # what each queue writes: enough of them fill the room, and one can fail
                        written = [random_.choice((5, 13, 24, 24, 0x30)) for _ in range(16)]
                        if n == 4:
                            written[random_.randrange(4)] = random_.choice((0, 0xFFFFFFFF))
                        t.check(
                            "voice_queues_build_packet",
                            Buffer(random_.randbytes(0x210)),
                            Buffer(bytes(data)),
                            peer,
                            size,
                            globals={**state(enabled, initialized), "unknown_004c9878": random_.choice((peer, peer, 3, NONE))},
                            externals={
                                "voice_queues_go_to_peer": gives(1 if n != 5 else 0),
                                "voice_queue_write": nth("voice_queue_write", written),
                            },
                        )

    for on in (1, 1, 1, 0):
        for flags in (0, 1, 0xFE, 0xFF):
            for chosen in (1, 0):
                for queue_count in (1, 2, 5, 15, 0, 16, 0xFF):
                    for n in range(5):
                        data = bytearray(random_.randbytes(0x200))
                        data[0], data[1] = flags, queue_count
                        # the queues are the test's: each takes so much, and the next starts there
                        taken = [random_.choice((4, 12, 20)) for _ in range(16)]
                        if n == 3:
                            taken[random_.randrange(3)] = random_.choice((0, 0xFFFFFFFF))
                        at = 2
                        for index in range(16):
                            data[at] = random_.randint(0, 15) if n != 4 or index != 1 else 16
                            data[at + 1] = random_.choice((1, 3, 15, 15, 0, 16)) if n == 2 else random_.randint(1, 15)
                            if 1 <= data[at + 1] <= 15 and taken[index] not in (0, 0xFFFFFFFF):
                                at += taken[index]
                        calls_taken = taken

                        def take(harness, arguments, calls_taken=calls_taken, data=data):
                            calls = sum(1 for call in harness._calls if call[0] == "voice_queue_read")
                            return calls_taken[min(calls, 15)], (*arguments[:-1], arguments[-1] & 0xFF)

                        t.check(
                            "voice_queues_read_packet",
                            random_.choice((0x200, 0x40, 2)),
                            Buffer(bytes(queues(on))),
                            Buffer(bytes(data)),
                            externals={"game_voice_unknown_000549d0": gives(chosen), "voice_queue_read": take},
                        )

    # ---------- a queue, in what goes from one console to another

    def a_queue(active_: int, count: int, peer: int) -> bytearray:
        """A queue with its things: each for some consoles, and the queue with
        the bits of all of them and how many go to each."""
        data = bytearray(random_.randbytes(QUEUE_SIZE))
        data[0] = active_
        struct.pack_into("<I", data, 4, count)
        all_peers = 0
        counts = [0] * 16
        for index in range(count):
            peers = random_.choice((1 << peer, 1 << peer, (1 << peer) | random_.getrandbits(16), random_.getrandbits(16) & ~(1 << peer), 0))
            struct.pack_into("<H", data, 0x10 + index * ENTRY_SIZE + 6, peers)
            struct.pack_into("<H", data, 0x10 + index * ENTRY_SIZE + 2, random_.choice((0, 0, random_.getrandbits(16))))
            all_peers |= peers
            for other in range(16):
                counts[other] += peers >> other & 1
        struct.pack_into("<H", data, 8, all_peers)
        struct.pack_into("<16I", data, 0x130, *counts)
        return data

    for active_ in (1, 1, 1, 0x80, 0):
        for count in (0, 1, 2, 5, 15):
            for peer in (0, 7, 15):
                for is_chosen in (0, 1, 0x80):
                    for size in (0x200, 0, 2, 12, 13, 14, 15, 28, 40, 60):
                        data = a_queue(active_, count, peer)
                        if size == 0:
                            # a queue that says it has something for the console, and has not
                            struct.pack_into("<H", data, 8, struct.unpack_from("<H", data, 8)[0] | 1 << peer)
                        t.check(
                            "voice_queue_write",
                            Buffer(bytes(data)),
                            peer,
                            Buffer(random_.randbytes(0x110)),
                            Buffer(random_.randbytes(2)),
                            size,
                            is_chosen,
                        )

    for peer in (16, 31, NONE, 0x80000000):
        data = a_queue(1, 3, 0)
        struct.pack_into("<H", data, 8, 0xFFFF)
        t.check("voice_queue_write", Buffer(bytes(data)), peer, Buffer(random_.randbytes(0x110)), Buffer(random_.randbytes(2)), 0x100, 0)

    def routed(name: str):
        def handler(harness, arguments):
            return 0, tuple(harness.normalize(argument) for argument in arguments)

        return handler

    def words_for(harness, arguments):
        # the two numbers are in the stack
        harness.mu.mem_write(arguments[2], struct.pack("<2H", arguments[1] & 0xFFFF, 0x4321))
        return 0, (harness.normalize(arguments[0]), arguments[1])

    def add(harness, arguments):
        words, queue, data, size, flag = arguments
        return 0, (bytes(harness.mu.mem_read(words, 4)), queue, data, size, flag)

    for for_chosen in (1, 0, 0x80):
        entry = 13 if for_chosen else 11
        for count in (1, 2, 5, 16, 0, 17, 0xFF):
            for n in range(14):
                local_peer = random_.choice((0, 3, 15)) if n != 9 else NONE
                packet = bytearray(random_.randbytes(2 + 17 * 13 + 8))
                packet[1] = count
                for index in range(17):
                    at = 2 + index * entry
                    packet[at] = random_.choice((0, 1)) if n != 10 or index != 1 else random_.choice((2, 0xFF))
                    if for_chosen:
                        peers = random_.choice((1 << local_peer & 0xFFFF, (1 << local_peer & 0xFFFF) | random_.getrandbits(16), random_.getrandbits(16) | 0x10000 >> 16 | 1 << ((local_peer + 1) & 15)))
                        if n == 11 and index == min(count, 16) - 1:
                            peers = 0
                        struct.pack_into("<H", packet, at + 1, peers & 0xFFFF)
                size = 2 + min(max(count, 1), 16) * entry + random_.choice((0, 0, 5, -1, -entry)) if n < 12 else random_.choice((0, 1, 2, entry + 1))
                data = bytearray(random_.randbytes(QUEUE_SIZE))
                struct.pack_into("<I", data, 0xC, random_.randint(0, 15))
                t.check(
                    "voice_queue_read",
                    Buffer(bytes(data)),
                    Buffer(bytes(packet)),
                    size & 0xFFFFFFFF,
                    for_chosen,
                    globals={
                        **state(*random_.choice(((1, 1), (1, 1), (0, 1), (1, 0)))),
                        "unknown_00527108": GlobalRange(CHANNELS_SIZE),
                        "unknown_00527104": GlobalRange(4),
                    },
                    externals={
                        "game_voice_get_local_peer": gives(local_peer),
                        "game_voice_has_remote_talker": gives(random_.choice((1, 1, 0))),
                        "voice_has_remote_talker": lambda harness, arguments, has=random_.choice((1, 1, 0)): (
                            has,
                            (harness.normalize(arguments[0]), arguments[1]),
                        ),
                        "unknown_00056df0": routed("unknown_00056df0"),
                        "game_voice_remote_talker_data": gives(0),
                        "voice_route_peers": words_for,
                        "voice_queue_add": add,
                    },
                )

    # ---------- from players to consoles

    PLAYERS = 0x10D4
    PLAYER_SIZE = 0x13C

    for enabled, initialized in active():
        for n in range(24):
            local_peer = random_.choice((0, 2, 15, NONE))
            data = bytearray(random_.randbytes(PLAYERS + 16 * PLAYER_SIZE))
            struct.pack_into("<I", data, MEMBERSHIP_PLAYERS, random_.choice((0xFFFF, 0, random_.getrandbits(16), 0xFFFFFFFF)))
            for player in range(16):
                struct.pack_into(
                    "<I",
                    data,
                    PLAYERS + player * PLAYER_SIZE + 0xC,
                    random_.choice((local_peer, random_.randint(0, 15), random_.randint(0, 15), NONE, 31)) & 0xFFFFFFFF,
                )
            membership = Buffer(bytes(data))
            t.check(
                "voice_targets_to_peers",
                random_.choice((0xFFFF, 0, random_.getrandbits(16), 0xFFFFFFFF, 1)),
                globals=state(enabled, initialized),
                externals={
                    "game_voice_get_membership": lambda harness, arguments, membership=membership, n=n: (
                        membership.address if n % 8 else 0
                    ),
                    "game_voice_get_local_peer": gives(local_peer),
                },
                extra=[membership],
            )

    # ---------- which consoles something goes to

    for through_chosen in (0, 1, 0x80):
        for n in range(60):
            chosen_peer = random_.choice((0, 3, 15, NONE, 16))
            peers = random_.choice((0xFFFF, 0, 1 << random_.randrange(16), random_.getrandbits(16), random_.getrandbits(32)))
            t.check(
                "voice_route_assign",
                Buffer(struct.pack("<i", random_.choice((1, 1, 2, 3, 16, 0, -1)))),
                Buffer(struct.pack("<I", peers)),
                random_.choice((0xFFFFFFFF, 0xFFFFFFFF, random_.getrandbits(16), 0)),
                Buffer(struct.pack("<I", random_.choice((0, 0, random_.getrandbits(32))))),
                through_chosen,
                chosen_peer,
                Buffer(bytes((random_.choice((0, 0, 1, 0x80)), 0x55))),
            )

    def assign(budgets: list):
        """The test's own way of giving consoles their bits: all that are left
        and allowed, and what it is told with is compared."""

        def handler(harness, arguments):
            budget, left, allowed, direct, through_chosen, chosen_peer, more = arguments
            (now,) = struct.unpack("<I", harness.mu.mem_read(left, 4))
            (have,) = struct.unpack("<I", harness.mu.mem_read(direct, 4))
            (count,) = struct.unpack("<i", harness.mu.mem_read(budget, 4))
            flag = bytes(harness.mu.mem_read(more, 1))
            calls = sum(1 for call in harness._calls if call[0] == "voice_route_assign")
            given = now & allowed & budgets[calls % len(budgets)]
            harness.mu.mem_write(left, struct.pack("<I", now & ~given))
            harness.mu.mem_write(direct, struct.pack("<I", have | given & 0xFFFF))
            harness.mu.mem_write(budget, struct.pack("<i", count - bin(given).count("1")))
            if calls % 3 == 1:
                harness.mu.mem_write(more, b"\1")
            return 0, (count, now, allowed, have, through_chosen & 0xFF, chosen_peer, flag)

        return handler

    for on in (1, 1, 1, 1, 0):
        for n in range(120):
            chosen_peer = random_.choice((0, 3, 15, NONE))
            enabled, initialized = random_.choice(((1, 1), (1, 1), (1, 1), (0, 1), (1, 0)))
            joined = random_.choice((0xFFFF, 0xFFFF, random_.getrandbits(16), 0, (1 << (chosen_peer & 15)) | random_.getrandbits(16)))
            data = bytearray(random_.randbytes(4))
            data[0] = on
            t.check(
                "voice_route_peers",
                Buffer(bytes(data)),
                random_.choice((0xFFFF, 0, 1 << random_.randrange(16), random_.getrandbits(16), random_.getrandbits(32), 0x10000)),
                Buffer(random_.randbytes(4)),
                globals={
                    **state(enabled, initialized),
                    "unknown_004c9878": chosen_peer,
                    "unknown_004c987c": random_.choice((0, 1, 2, 3)),
                },
                externals={
                    "game_voice_unknown_00054cc0": gives(random_.choice((1, 1, 2, 4, 16, 0, 0xFFFFFFFF))),
                    "game_voice_unknown_00054c70": gives(random_.choice((1, 1, 0, 0x100))),
                    "game_voice_local_peer_unknownA8": gives(joined),
                    "game_voice_get_session_source": gives(random_.choice((1, 2, 2, 0))),
                    "game_voice_unknown_00053c70": gives(random_.choice((0xFFFF, random_.getrandbits(16), 0))),
                    "voice_route_assign": assign([random_.choice((0xFFFF, 0, random_.getrandbits(16))) for _ in range(3)]),
                },
            )

    # ---------- a player and its object

    def a_player_case(unknown2c: int, has_object: bool, ticks_ago: int, seconds_per_tick: float):
        now = random_.choice((1000, 0x7FFFFFF0, 0x80000010, 0xFFFFFFF0))
        # the objects, in one buffer: the array, sixteen headers of twelve
        # bytes that it points to, and the object that one of them points to
        index = random_.randrange(16)
        array = bytearray(random_.randbytes(0x4C))
        struct.pack_into("<I", array, 0x44, 0x4C)
        headers = bytearray(random_.randbytes(16 * 12))
        struct.pack_into("<I", headers, index * 12 + 8, 0x4C + 16 * 12)
        an_object = bytearray(random_.randbytes(0x2E4))
        struct.pack_into("<I", an_object, 0x2E0, (now - ticks_ago) & 0xFFFFFFFF)
        objects = Buffer(bytes(array) + bytes(headers) + bytes(an_object), pointers=(0x44, 0x4C + index * 12 + 8))
        datum = bytearray(random_.randbytes(0x34))
        struct.pack_into("<I", datum, 0x2C, unknown2c & 0xFFFFFFFF)
        struct.pack_into("<I", datum, 0x30, (random_.getrandbits(16) << 16 | index) if has_object else NONE)
        time = bytearray(random_.randbytes(0xC))
        struct.pack_into("<fI", time, 4, seconds_per_tick, now)
        return objects, Buffer(bytes(datum)), Buffer(bytes(time))

    for is_one in (1, 0):
        for has_datum in (True, False):
            for unknown2c in (NONE, 0, 5):
                for has_object in (True, False):
                    for ticks_ago, seconds_per_tick in ((0, 1 / 30), (30, 1 / 30), (31, 1 / 30), (29, 1 / 30), (-5, 1 / 30), (60, 1 / 60), (61, 1 / 60), (1, 1.0), (2, 1.0), (10, float("nan")), (0x7FFFFFFF, 1 / 30)):
                        objects, datum, time = a_player_case(unknown2c, has_object, ticks_ago, seconds_per_tick)
                        t.check(
                            "voice_player_unknown_000589e0",
                            random_.getrandbits(4),
                            globals={"object_header_data": objects, "game_time_globals": time},
                            externals={
                                "game_voice_unknown_00054df0": gives(is_one),
                                "game_voice_get_player_datum": lambda harness, arguments, datum=datum, has_datum=has_datum: (
                                    datum.address if has_datum else 0
                                ),
                            },
                            extra=[datum],
                        )

    # ---------- a time that who hears whom has

    for player in (0, 7, 15):
        for seconds_per_tick in (1 / 30, 1 / 60):
            for limit in (2.0, 0.5, 0.0, -1.0, float("nan"), float("inf")):
                for ticks_ago in (None, 0, 1, 14, 15, 16, 29, 30, 31, 59, 60, 61, 119, 120, 121, 100000, -1, -100):
                    now = random_.choice((1000000, 0x7FFFFFF0, 0x80000010))
                    data = bytearray(random_.randbytes(0x194 + 16 * 4))
                    struct.pack_into("<I", data, 0x194 + player * 4, 0 if ticks_ago is None else (now - ticks_ago) & 0xFFFFFFFF)
                    time = bytearray(random_.randbytes(0xC))
                    struct.pack_into("<fI", time, 4, seconds_per_tick, now)
                    t.check(
                        "voice_channels_time_is_recent",
                        Buffer(bytes(data)),
                        player,
                        globals={"game_time_globals": Buffer(bytes(time)), "unknown_004cf738": struct.pack("<f", limit)},
                    )

    # ---------- who hears whom, by teams

    for on in (1, 1, 1, 0):
        for enabled, initialized in active():
            for network in (3, 3, 2, 1, 0, 4):
                for n in range(8):
                    player = random_.choice((0, 5, 15))
                    data = bytearray(random_.randbytes(PLAYERS + 16 * PLAYER_SIZE))
                    mask = random_.choice((0xFFFF, 0xFFFF, random_.getrandbits(16), 0xFFFF ^ (1 << player), 0xFFFFFFFF))
                    struct.pack_into("<I", data, MEMBERSHIP_PLAYERS, mask)
                    for other in range(16):
                        data[PLAYERS + other * PLAYER_SIZE + 0x124] = random_.choice((0, 0, 1, 1, 7, 0xFF, 0x80))
                    if n == 1:
                        data[PLAYERS + player * PLAYER_SIZE + 0x124] = 0xFF
                    membership = Buffer(bytes(data))
                    channels_ = channels(on)
                    t.check(
                        "voice_channels_get_player_by_team",
                        Buffer(bytes(channels_)),
                        player,
                        globals={**state(enabled, initialized), "unknown_004c9888": network},
                        externals={
                            # the game does not see whether there are players: there are, here, in the third state
                            "game_voice_get_membership": lambda harness, arguments, membership=membership, n=n, network=network: (
                                membership.address if n != 7 or network == 3 else 0
                            ),
                            "game_voice_get_player_array": lambda harness, arguments, membership=membership: membership.address
                            + PLAYERS,
                        },
                        extra=[membership],
                    )

    # ---------- a voice that ends

    for enabled, initialized in active():
        for n in range(30):
            now = random_.choice((1000000, 0x7FFFFFF0, 0x80000010))
            seconds_per_tick = random_.choice((1 / 30, 1 / 60))
            data = bytearray(random_.randbytes(0x1D8))
            for player in range(16):
                # some started a moment ago, some a while ago, and the limit itself
                ago = random_.choice((0, 1, 5, 8, 9, 10, 17, 18, 19, 60, -3, 100000))
                struct.pack_into("<I", data, 0x148 + player * 4, (now - ago) & 0xFFFFFFFF)
            membership = bytearray(random_.randbytes(MEMBERSHIP_PLAYERS + 4))
            struct.pack_into("<I", membership, MEMBERSHIP_PLAYERS, random_.choice((0xFFFF, 0xFFFF, random_.getrandbits(16))))
            membership = Buffer(bytes(membership))
            time = bytearray(random_.randbytes(0xC))
            struct.pack_into("<fI", time, 4, seconds_per_tick, now)
            local = random_.choice((1, 3, 0x8001, random_.getrandbits(16), 0))
            has = random_.choice((0xFFFF, 0xFFFF, random_.getrandbits(16)))
            still = random_.choice((0, 0, random_.getrandbits(16)))
            can = [random_.choice((0xFFFF, 0xFFFF, random_.getrandbits(16))) for _ in range(16)]
            muted = [random_.choice((0, 0, 0, random_.getrandbits(16))) for _ in range(16)]
            t.check(
                "voice_channels_update_ended",
                Buffer(bytes(data)),
                globals={**state(enabled, initialized), "game_time_globals": Buffer(bytes(time))},
                externals={
                    "game_voice_get_membership": lambda harness, arguments, membership=membership, n=n: (
                        membership.address if n % 10 else 0
                    ),
                    "game_voice_get_local_players": gives(local),
                    "game_voice_unknown_00054df0": lambda harness, arguments, has=has: has >> (arguments[0] & 15) & 1,
                    "voice_player_unknown_000589e0": lambda harness, arguments, still=still: still >> (arguments[0] & 15) & 1,
                    "voice_players_can_hear": lambda harness, arguments, can=can: can[arguments[0] & 15] >> (arguments[1] & 15) & 1,
                    "game_voice_player_has_muted": lambda harness, arguments, muted=muted: (
                        muted[arguments[0] & 15] >> (arguments[1] & 15) & 1
                    ),
                    # what the function is given and does not look at is not compared: the
                    # compiler of source/ does not pass it
                    "voice_play_sound_by_number": lambda harness, arguments: (0, tuple(arguments[:2])),
                },
                extra=[membership],
            )

    # ---------- whether one player can hear another

    for enabled, initialized in active():
        for n in range(80):
            player, local_player = random_.sample(range(16), 2)
            data = bytearray(random_.randbytes(PLAYERS + 16 * PLAYER_SIZE))
            mask = random_.choice((0xFFFF, 0xFFFF, 0xFFFF, random_.getrandbits(16), 0xFFFF ^ (1 << player), 0xFFFF ^ (1 << local_player)))
            struct.pack_into("<I", data, MEMBERSHIP_PLAYERS, mask)
            teams = [random_.choice((0, 0, 1, 0xFF)) for _ in range(16)]
            if n % 4 == 0:
                teams[player] = teams[local_player]
            for other in range(16):
                data[PLAYERS + other * PLAYER_SIZE + 0x124] = teams[other]
            membership = Buffer(bytes(data))
            local = random_.choice((1 << local_player, 1 << local_player, (1 << local_player) | (1 << player), 0, random_.getrandbits(16)))
            says = [random_.choice((0, 0, 1)) for _ in range(16)]
            t.check(
                "voice_players_can_hear",
                player,
                local_player,
                globals=state(enabled, initialized),
                externals={
                    # the game does not see whether the session has players: it always has here
                    "game_voice_get_membership": lambda harness, arguments, membership=membership: membership.address,
                    "game_voice_get_local_players": gives(local),
                    "game_voice_unknown_00053d40": gives(random_.choice((1, 1, 0))),
                    "game_voice_unknown_00053d90": gives(random_.choice((1, 0, 0))),
                    "unknown_00192950": lambda harness, arguments, says=says: says[arguments[0] & 15],
                },
                extra=[membership],
            )

    # ---------- the sounds of the voice

    def sounds_case(count: int, sound_tags: list):
        """The globals of the game, the tags and what has the sounds, in one
        buffer: each points to the next."""
        tag_index = random_.randrange(8)
        game_globals = bytearray(random_.randbytes(0x170))
        struct.pack_into("<I", game_globals, 0x16C, random_.getrandbits(16) << 16 | tag_index)
        instances = bytearray(random_.randbytes(8 * 16))
        tag = bytearray(random_.randbytes(0x10))
        block = bytearray(random_.randbytes(0x98))
        elements = bytearray(random_.randbytes(8 * 8))
        for n, sound_tag in enumerate(sound_tags):
            struct.pack_into("<I", elements, n * 8 + 4, sound_tag)
        instances_at = 0
        tag_at = len(instances)
        block_at = tag_at + len(tag)
        elements_at = block_at + len(block)
        struct.pack_into("<I", instances, tag_index * 16 + 8, tag_at)
        struct.pack_into("<I", tag, 0xC, block_at)
        struct.pack_into("<iI", block, 0x90, count, elements_at)
        tags = Buffer(
            bytes(instances + tag + block + elements),
            pointers=(tag_index * 16 + 8, tag_at + 0xC, block_at + 0x94),
        )
        return Buffer(bytes(game_globals)), tags

    for count in (8, 3, 2, 0, -1):
        for sound in (0, 2, 7):
            sound_tags = [random_.choice((0xE1740005, 0xE1740005, NONE)) for _ in range(8)]
            game_globals, tags = sounds_case(count, sound_tags)
            t.check(
                "voice_play_sound_by_number",
                sound,
                random_.randrange(16),
                0x80345000,
                globals={"global_game_globals": game_globals, "global_tag_instances": tags},
                externals={"voice_play_sound": lambda harness, arguments: 0},
            )

    def request(harness, arguments):
        # what it is played with is in the stack
        return 0, (bytes(harness.mu.mem_read(arguments[0], 0x10)),)

    for enabled, initialized in active():
        for player in (0, 9, 15):
            for controllers in ((0, 0), (3, 3), (NONE, 0), (1, NONE), (2, 1)):
                for setting in (0, 1, 2, 3):
                    for talks in (1, 0):
                        for says in (1, 0):
                            calls = {"n": 0}

                            def controller(harness, arguments, controllers=controllers):
                                n = sum(1 for call in harness._calls if call[0] == "game_voice_player_controller")
                                return controllers[min(n, 1)]

                            settings = [random_.randint(0, 3) for _ in range(4)]
                            if controllers[0] != NONE:
                                settings[controllers[0]] = setting
                            t.check(
                                "voice_play_sound",
                                player,
                                0xE1740005,
                                globals={
                                    **state(enabled, initialized),
                                    "game_voice_controller_settings": struct.pack("<4I", *settings),
                                    "game_voice_talkers": struct.pack(
                                        "<H", (1 << player) if talks else 0xFFFF ^ (1 << player)
                                    ),
                                },
                                externals={
                                    "game_voice_player_controller": controller,
                                    "unknown_001906da": gives(says),
                                    "unknown_001896c0": lambda harness, arguments: 0,
                                    "unknown_00189760": request,
                                },
                            )
