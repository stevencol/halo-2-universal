# source/networking/online_presence.c

import random
import struct

from difftest import Buffer
from test_online_storage import tasks_table

NONE = 0xFFFFFFFF
FAILED = (0x80004005, 0x80151000, 0xFFFFFFFF)
BITS = (0x1, 0x2, 0x8, 0x10, 0x100, 0x200, 0x400, 0x800, 0x8000000, 0x80000000)


def run(t) -> None:
    random_ = random.Random(551)

    def same(harness, arguments):
        return (random.Random(arguments[0]).choice((0, 1, 1, 0x100)), tuple(arguments))

    def state_bits() -> int:
        value = 0
        for bit in BITS:
            if random_.random() < 0.4:
                value |= bit
        return value | (random_.getrandbits(32) if random_.random() < 0.2 else 0)

    for n in range(2500):
        signed_in = {
            "online_tasks_get_logon_state": lambda harness, arguments, state=random_.choice((1, 1, 1, 1, 0, 2)): (state, tuple(arguments)),
        }
        task_global = {"unknown_00467214": random_.choice((0, 0, 0, 0x80010002, NONE))}
        told = lambda harness, arguments: (0, tuple(arguments))

        t.check(
            "online_presence_state_from_library",
            state_bits(),
            random_.getrandbits(32),
            globals=task_global,
            externals={**signed_in, "XOnlineTitleIdIsSameTitle": same},
        )

        tasks, number = tasks_table(random_)

        def starts(harness, arguments, result=random_.choice((0, 0, 0, 1) + FAILED), handle=random_.getrandbits(32)):
            if result < 0x80000000:
                harness.mu.mem_write(arguments[2], struct.pack("<I", handle))
            return result, tuple(arguments[:2])

        t.check(
            "online_presence_start",
            random_.randrange(4),
            globals={"online_tasks": tasks},
            externals={
                "online_task_new_signed_in": lambda harness, arguments, index=random_.choice((number, number, number, NONE, number ^ 0x10000)): (index, ()),
                "unknown_003a8316": starts,
                "online_task_close": told,
            },
        )

        for name, library, arguments in (
            ("online_presence_unknown_0008c3a0", "unknown_003a695e", 3),
            ("online_presence_unknown_0008c410", "unknown_003a6f9b", 0),
            ("online_presence_unknown_0008c550", "unknown_003a6969", 0),
        ):
            tasks, number = tasks_table(random_)
            t.check(
                name,
                random_.choice((number, number, number, number, NONE, number ^ 0x10000, number | 0xFFF8)),
                *(random_.getrandbits(32) for index in range(arguments)),
                globals={"online_tasks": tasks, **task_global},
                externals={**signed_in, library: told},
            )

        tasks, number = tasks_table(random_)
        count = random_.choice((0, 1, 2, 4))

        def latest(harness, arguments, result=random_.choice((0, 0, 0, 0x80004005)), count=count):
            data = bytearray(random.Random(arguments[1]).randbytes(count * 0x3D))
            for index in range(count):
                struct.pack_into("<I", data, index * 0x3D + 0xC, random.Random(arguments[1] + index).choice((0, 1, 0x8000001, 0x8000011, 0xF1B, 0xFFFFFFFF)))
            harness.mu.mem_write(arguments[3], bytes(data))
            return result, tuple(arguments[:3])

        t.check(
            "online_presence_get_latest",
            random_.choice((number, number, number, number, NONE, number ^ 0x10000)),
            random_.getrandbits(16),
            count,
            Buffer(random_.randbytes(4 * 0x3D + 4)),
            globals={"online_tasks": tasks, **task_global},
            externals={
                **signed_in,
                "online_task_get_status": lambda harness, arguments, status=random_.choice((0, 1, 2, 2, 3, 5)): (status, tuple(arguments)),
                "unknown_003a6fa6": latest,
                "XOnlineTitleIdIsSameTitle": same,
            },
        )

    # ---------- the names that the game remembers

    for n in range(3000):
        count = 150
        names = bytearray(random_.randbytes(count * 0x40))
        users = []
        for index in range(count):
            base = index * 0x40
            struct.pack_into("<II", names, base + 0x38, random_.choice((100, 5000, 5000, 90000, random_.getrandbits(32))), random_.choice((0, 1, 1, 3, 3, 3, 2)))
            users.append(bytes(names[base:base + 12]))
        if random_.random() < 0.3:
            # no place is free, and none has a name
            for index in range(count):
                struct.pack_into("<I", names, index * 0x40 + 0x3C, random_.choice((1, 2)))
        user = random_.choice((random_.choice(users), random_.choice(users), random_.randbytes(12)))
        now = random_.choice((5000, 6000, 65000, 100000, 0x80001000))
        t.check(
            "online_presence_names_find",
            Buffer(bytes(names)),
            random_.choice((0, 0, 0, 1)),
            Buffer(user),
            Buffer(random_.randbytes(0x24)),
            Buffer(random_.randbytes(12)),
            globals={
                "unknown_004cf768": random_.choice((0, 1, 60, 60, 86400)),
                "unknown_00510548": bytes((random_.choice((0, 0, 1)),)),
                "unknown_0051054c": random_.choice((5000, 70000, 0x90000000)),
            },
            externals={"GetTickCount": lambda harness, arguments, now=now: (now, ())},
        )
