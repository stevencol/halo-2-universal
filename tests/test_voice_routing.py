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
                            "unknown_00056f60",
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
                                    "unknown_000589e0": gives(says),
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
                            "unknown_00056c60": lambda harness, arguments, hears=hears: (
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
                    externals={"unknown_00056990": gives(unknown4), "unknown_000565c0": gives(0)},
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
                        "unknown_000565c0": words_for,
                        "voice_queue_add": add,
                    },
                )
