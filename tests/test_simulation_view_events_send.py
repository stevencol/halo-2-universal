# source/networking/simulation_view.c: what a view has for the things that happen is sent

import hashlib
import random
import struct

from difftest import Buffer

VIEW = 0x90
EVENTS = 0x4CD0


def run(t) -> None:
    random_ = random.Random(588)

    for n in range(400):
        now_data = bytes((random_.choice((0, 1)),)) + random_.randbytes(0x4CC0 - 1)
        data = bytearray(random_.randbytes(EVENTS + VIEW))
        struct.pack_into("<I", data, 0, EVENTS)
        # what the view has: what there is now or something else, with its first byte clear or not
        if random_.choice((False, False, True)):
            data[0xC : 0xC + 0x4CC0] = now_data
        else:
            data[0xC] = random_.choice((0, 0, 1))
        struct.pack_into("<i", data, EVENTS + 0x24, random_.choice((-1, 2, 2, 5)))
        struct.pack_into("<I", data, EVENTS + 0x20, 0x8765)
        kept = random_.choice((0, 1, 0x80))
        now = random_.getrandbits(32)

        def takes(harness, arguments, now_data=now_data):
            harness.mu.mem_write(arguments[0], now_data)
            return 0, ()

        def changes(harness, arguments):
            old = bytes(harness.mu.mem_read(arguments[0], 0x4CC0))
            new = bytes(harness.mu.mem_read(arguments[1], 0x4CC0))
            before = bytes(harness.mu.mem_read(arguments[2], 0x4FB0))
            digest = hashlib.sha256(old + new).digest()
            harness.mu.mem_write(arguments[2], digest * (0x4FB0 // 32))
            return 0, (harness.normalize(arguments[0]), hashlib.sha256(old).hexdigest(), hashlib.sha256(new).hexdigest(), before.count(0))

        def sends(harness, arguments):
            message = bytes(harness.mu.mem_read(arguments[6], arguments[5]))
            return 0, (arguments[0] & 0xFF, *arguments[1:6], hashlib.sha256(message).hexdigest())

        t.check(
            "simulation_view_events_send",
            Buffer(bytes(data), pointers=(0,)),
            globals={
                "unknown_00510548": bytes((kept,)),
                "unknown_0051054c": now if kept else random_.getrandbits(32),
            },
            externals={
                "unknown_00197360": takes,
                "unknown_00198540": changes,
                "network_observer_channel_send": sends,
                "GetTickCount": lambda harness, arguments, now=now: now,
            },
        )
