# source/networking/network_message_handler.c

import random
import struct

from difftest import Buffer

from test_simulation_world import a_world


def run(t) -> None:
    random_ = random.Random(444)

    def view_and(*kinds):
        """What stands in for a function of a view: the view, and each other thing as its kind says."""

        def handler(harness, arguments):
            record = [harness.normalize(arguments[0])]
            for kind, value in zip(kinds, arguments[1:]):
                record.append(harness.normalize(value) if kind == "pointer" else value & 0xFF if kind == "byte" else value)
            return random_.choice((0, 1)), tuple(record)

        return handler

    externals = {
        "simulation_view_handle_remote_state": view_and("number", "number"),
        "simulation_view_set_players": view_and("number", "number", "pointer"),
        "simulation_view_unknown_00085dc0": view_and("number"),
        "simulation_view_events_receive": view_and("number", "number", "pointer"),
        "simulation_view_unknown_00085ed0": view_and("number", "number", "number"),
        "simulation_view_unknown_00085f50": view_and("byte", "number", "number", "number", "pointer"),
        "simulation_world_unknown_000698e0": view_and("pointer"),
    }

    for n in range(1500):
        channel = random_.randrange(4)

        def game() -> dict:
            there = bytearray(random_.randbytes(0x10))
            there[0] = random_.choice((0, 1, 1, 1, 1, 0x80))
            return {"unknown_004cf770": bytes(there), "unknown_004cf770+0xc": a_world(random_, channel)}

        def message() -> Buffer:
            return Buffer(random_.randbytes(0x20))

        for name in ("00094910", "00094940", "000949d0", "00094aa0", "00094990", "00094a10"):
            t.check(f"network_message_handler_unknown_{name}", message(), channel, globals=game(), externals=externals)
        data = random_.randbytes(0x20)
        t.check(
            "network_message_handler_unknown_00094a50",
            Buffer(data),
            random_.choice((struct.unpack_from("<I", data, 4)[0], struct.unpack_from("<I", data, 4)[0], random_.getrandbits(32))),
            random_.getrandbits(32),
            channel,
            globals=game(),
            externals=externals,
        )
