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
                    externals={"unknown_00055b40": gives(0)},
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
