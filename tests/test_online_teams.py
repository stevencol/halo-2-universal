# source/networking/online_teams.c

import random
import struct

from difftest import Buffer
from test_online_storage import tasks_table

NONE = 0xFFFFFFFF
FAILED = (0x80004005, 0x80151000, 0xFFFFFFFF)


def run(t) -> None:
    random_ = random.Random(533)

    def closed(harness, arguments):
        return 0, tuple(arguments)

    def starts(handle_at: int):
        """A function of the libraries that starts a task: what it is given but for where the task's number goes,
        which gets one if the task starts."""

        def stand_in(harness, arguments, result=random_.choice((0, 0, 0, 1) + FAILED), handle=random_.getrandbits(32)):
            if result < 0x80000000:
                harness.mu.mem_write(arguments[handle_at], struct.pack("<I", handle))
            return result, tuple(arguments[:handle_at])

        return stand_in

    def new_task(number: int):
        return lambda harness, arguments, index=random_.choice((number, number, number, NONE, number ^ 0x10000)): (index, ())

    # ---------- the tasks that start

    STARTS = (
        ("online_teams_enumerate_start", "unknown_003afc26", 5, 1, 0),
        ("online_team_unknown_000abf10_start", "unknown_003afb8f", 5, 1, 0),
        ("online_team_unknown_000abfa0_start", "unknown_003a8407", 6, 1, 1),
        ("online_team_unknown_000ac050_start", "unknown_003afbf1", 8, 2, 0),
        ("online_team_members_enumerate_start", "unknown_003afc76", 6, 1, 0),
    )
    for n in range(1500):
        for name, library, handle_at, identifiers, numbers in STARTS:
            tasks, number = tasks_table(random_)
            arguments = [Buffer(random_.randbytes(12)) for index in range(identifiers)]
            # the owner goes after the first of them
            arguments.insert(1, random_.randrange(4))
            arguments += [random_.choice((0, 1, 2, 3, NONE, random_.getrandbits(32))) for index in range(numbers)]
            t.check(
                name,
                *arguments,
                globals={"online_tasks": tasks},
                externals={"online_task_new_signed_in": new_task(number), library: starts(handle_at), "online_task_close": closed},
            )

        tasks, number = tasks_table(random_)

        def started(harness, arguments, result=random_.choice((0, 0, 0, 1) + FAILED), handle=random_.getrandbits(32)):
            # of what goes with the number, only the first six bytes are set
            if result < 0x80000000:
                harness.mu.mem_write(arguments[5], struct.pack("<I", handle))
            return result, (arguments[0], arguments[1], bytes(harness.mu.mem_read(arguments[2], 6)), arguments[3], arguments[4])

        t.check(
            "online_team_unknown_000abe60_start",
            random_.randrange(4),
            random_.getrandbits(32),
            globals={"online_tasks": tasks},
            externals={"online_task_new_signed_in": new_task(number), "unknown_003afb80": started, "online_task_close": closed},
        )

    # ---------- what a task found

    for n in range(2000):
        signed_in = {
            "online_tasks_get_logon_state": lambda harness, arguments, state=random_.choice((1, 1, 1, 1, 0, 2)): (state, tuple(arguments)),
            "online_task_get_status": lambda harness, arguments, status=random_.choice((0, 1, 2, 2, 3, 4, 5)): (status, tuple(arguments)),
        }

        def found(harness, arguments, result=random_.choice((0, 0, 0x80004005)), count=random_.randrange(5), data=random_.randbytes(48)):
            asked = struct.unpack("<I", bytes(harness.mu.mem_read(arguments[1], 4)))[0]
            harness.mu.mem_write(arguments[1], struct.pack("<I", count))
            harness.mu.mem_write(arguments[2], data[:count * 12])
            return result, (arguments[0], asked)

        def one(size: int, flags_at: int = -1):
            def stand_in(harness, arguments, result=random_.choice((0, 0, 0, 0x80004005)), data=random_.randbytes(size)):
                data = bytearray(data)
                if flags_at >= 0:
                    struct.pack_into("<I", data, flags_at, random.Random(data[0]).choice((0, 1, 3, 4, 7, 0x10, 0x1C, 0x1D, 0x1E, 0x1F, 0x0F, 0xFFFFFFFF, 0xFFFFFFE0, int.from_bytes(data[4:8], "little"))))
                harness.mu.mem_write(arguments[4], bytes(data))
                return result, tuple(arguments[:4])

            return stand_in

        for name, library in (
            ("online_teams_enumerate_get_results", "unknown_003aeba5"),
            ("online_team_members_enumerate_get_results", "unknown_003aebb0"),
        ):
            tasks, number = tasks_table(random_)
            t.check(
                name,
                random_.choice((number, number, number, number, NONE, number ^ 0x10000, number | 0xFFF8)),
                Buffer(struct.pack("<I", random_.choice((8, 100, 0, 4)))),
                Buffer(random_.randbytes(48)),
                globals={"online_tasks": tasks, "unknown_00467214": random_.choice((0, 0, 0, 0x80010002, NONE))},
                externals={**signed_in, library: found},
            )

        for name, library, size, flags_at in (
            ("online_team_get", "unknown_003afc50", 0x6A2, -1),
            ("online_team_member_get", "unknown_003afca3", 0x92, 0x1C),
        ):
            tasks, number = tasks_table(random_)
            t.check(
                name,
                random_.choice((number, number, number, number, NONE, number ^ 0x10000, number | 0xFFF8)),
                Buffer(random_.randbytes(12)),
                Buffer(random_.randbytes(size)),
                globals={"online_tasks": tasks, "unknown_00467214": random_.choice((0, 0, 0, 0x80010002, NONE))},
                externals={**signed_in, library: one(size, flags_at)},
            )
