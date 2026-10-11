# source/networking/network_globals.c

import random
import struct

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(505)

    def call(name: str):
        return lambda harness, arguments: (0, tuple(harness.normalize(value) for value in arguments))

    def clock() -> tuple:
        ticks = [random_.getrandbits(32) for index in range(2)]

        def tick(harness, arguments, ticks=ticks):
            from test_async import calls_so_far

            return ticks[min(calls_so_far(harness, "GetTickCount"), 1)]

        return tick

    def main_globals() -> dict:
        return {
            "unknown_004d8ba0": bytes((random_.choice((1, 1, 1, 0)),)),
            "unknown_00510548": bytes((random_.choice((0, 1)),)),
            "unknown_0051054c": random_.getrandbits(32),
        }

    UPDATES = (
        "transport_global_update unknown_0007f070 unknown_000b2ea0 unknown_000b3200 unknown_00063d80 unknown_0006b4a0 "
        "game_voice_update unknown_00074aa0 unknown_000755d0 unknown_00080390 network_blob_tasks_update unknown_000b3de0"
    ).split()

    for n in range(300):
        t.check(
            "network_update",
            globals=main_globals(),
            externals=dict({name: call(name) for name in UPDATES}, GetTickCount=clock()),
        )

        # the game in the network, which has what the consoles share at 0x4
        world = bytearray(random_.randbytes(0x20))
        struct.pack_into("<Ii", world, 4, 0x10, random_.choice((0, 1, 2, 3, 4, 4, 4, 4, 5)))
        there = bytearray(random_.randbytes(0x10))
        there[2] = random_.choice((0, 0, 0, 1))
        game = bytearray(random_.randbytes(0x1124))
        game[0x1120] = random_.choice((0, 1, 1, 1))
        t.check(
            "network_send",
            globals=dict(
                main_globals(),
                **{
                    "unknown_004cf770": bytes(there),
                    "unknown_004cf770+0xc": Buffer(bytes(world), pointers=(4,)),
                    "main_game_globals": random_.choice((Buffer(bytes(game)), Buffer(bytes(game)), Buffer(bytes(game)), 0)),
                    "unknown_00510560": Buffer(random_.randbytes(0x10)),
                    "unknown_00510568": Buffer(random_.randbytes(0x10)),
                    "unknown_00510570": Buffer(random_.randbytes(0x10)),
                    "unknown_00510574": Buffer(random_.randbytes(3 * 0x78B8)),
                },
            ),
            externals={
                "GetTickCount": clock(),
                "network_session_update": call("network_session_update"),
                "network_observer_update": call("network_observer_update"),
                "simulation_entity_database_flush_updates": call("simulation_entity_database_flush_updates"),
                "network_link_update_channels": call("network_link_update_channels"),
                "network_message_gateway_flush": call("network_message_gateway_flush"),
            },
        )

        def things() -> dict:
            return dict(
                main_globals(),
                unknown_00510560=random_.choice((Buffer(random_.randbytes(0x10)), Buffer(random_.randbytes(0x10)), 0)),
                unknown_00510570=random_.choice((Buffer(random_.randbytes(0x4E04)), Buffer(random_.randbytes(0x4E04)), 0)),
            )

        t.check(
            "network_open_sockets",
            random_.getrandbits(32),
            globals=things(),
            externals={"network_link_open_sockets": lambda harness, arguments: (1, (harness.normalize(arguments[0]),))},
        )
        t.check(
            "network_close_sockets",
            random_.getrandbits(32),
            globals=things(),
            externals={
                "network_link_close_channels": call("network_link_close_channels"),
                "network_link_close_sockets": call("network_link_close_sockets"),
            },
        )
