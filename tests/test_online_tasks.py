# source/networking/online_tasks.c

import dataclasses
import random
import struct

from difftest import Buffer, GlobalRange, Mock, Pointer, mock_address

NONE = 0xFFFFFFFF
COUNT, TASK = 8, 0x14
TASKS = 0x4C
BITS = TASKS + COUNT * TASK
# after the table: what gives its memory, with its two functions; and something that goes over it
ALLOCATION = BITS + 4
ITERATOR = ALLOCATION + 12
SIZE = ITERATOR + 0x10


def run(t) -> None:
    random_ = random.Random(508)

    def table(allocation: bool = False) -> tuple:
        """A table of tasks, some of them there, and the number of one of them or of one that there is not."""
        data = bytearray(random_.randbytes(SIZE))
        data[0:0x20] = b"online tasks".ljust(0x20, b"\0")
        there = [random_.random() < 0.6 for index in range(COUNT)]
        numbers = []
        bits = 0
        for index in range(COUNT):
            identifier = random_.getrandbits(15) | 0x8000 if there[index] else 0
            struct.pack_into(
                "<HHIII",
                data,
                TASKS + index * TASK,
                identifier,
                random_.choice((0, 1, 2, 3, 4, 6, 0x24, 0x26, 0x20, random_.getrandbits(16))),
                random_.choice((2, 2, 3, 3, 0x21, 0x21, 0, 5)),
                random_.getrandbits(32),
                random_.choice((0, NONE, 0x1234, 0x1234, 0x77)),
            )
            numbers.append(identifier << 16 | index)
            bits |= there[index] << index
        struct.pack_into("<I", data, BITS, bits)
        struct.pack_into(
            "<iiBBHIIiiihHII",
            data,
            0x20,
            COUNT,
            TASK,
            0,
            1,
            4,
            0x64407440,
            ALLOCATION if allocation else 0,
            next((index for index in range(COUNT) if not there[index]), COUNT),
            max((index + 1 for index in range(COUNT) if there[index]), default=0),
            sum(there),
            random_.getrandbits(15) - 0x8000,
            0,
            TASKS,
            BITS,
        )
        struct.pack_into("<3I", data, ALLOCATION, ALLOCATION + 4, mock_address(0), mock_address(1))
        struct.pack_into("<4I", data, ITERATOR, random_.getrandbits(32), 0, NONE, random_.choice((NONE, NONE, 0, 3, 7)))
        number = random_.choice(numbers)
        number = random_.choice((number, number, number, number ^ 0x10000, NONE, number | 0xFFF8, 0x80000000 + COUNT))
        pointers = [0x44, 0x48, ALLOCATION, ITERATOR + 4] + ([0x30] if allocation else [])
        return Buffer(bytes(data), pointers=tuple(pointers), places=(ITERATOR,)), number

    for n in range(2000):
        tasks, number = table()
        t.check("online_task_iterator_next", Pointer(tasks, ITERATOR), extra=[tasks])

        tasks, number = table()
        t.check("online_task_get_status", number, globals={"online_tasks": tasks})

        tasks, number = table()
        t.check("online_task_new", globals={"online_tasks": tasks})

        # how a task goes on each time that it is asked to, while it is done with
        steps = random_.choice(((0x1500F2,), (0, 0x1500F2), (0, 0, 5, 0x1500F2), (0x80004005,), (0, 0x80070057)))

        def goes_on(harness, arguments, steps=steps):
            from test_async import calls_so_far

            return steps[min(calls_so_far(harness, "online_task_continue"), len(steps) - 1)], (harness.normalize(arguments[0]),)

        closes = {
            "online_task_continue": goes_on,
            "XOnlineFriendsEnumerateFinish": lambda harness, arguments, said=random_.choice((0, 0, 0x80004005)): (said, tuple(arguments)),
            "unknown_0008c550": lambda harness, arguments: (0, tuple(arguments)),
            "XOnlineTaskClose": lambda harness, arguments: (0, tuple(arguments)),
        }
        tasks, number = table()
        t.check("online_task_close", number, globals={"online_tasks": tasks}, externals=closes)

        tasks, number = table(allocation=random_.random() < 0.8)
        t.check(
            "online_tasks_dispose",
            globals={"online_tasks": tasks, "unknown_00479748": random_.choice((NONE, NONE, number))},
            externals=dict(closes, online_tasks_close_all=lambda harness, arguments: (0, ())),
            mocks=[Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)) for index in range(2)],
        )

    for n in range(300):
        made = Buffer(random_.randbytes(0x230), places=(0x44, 0x48))
        allocator = Buffer(struct.pack("<3I", 4, mock_address(0), mock_address(1)), pointers=(0,))
        t.check(
            "online_tasks_initialize",
            globals={
                "heap_allocation": allocator,
                "online_tasks": random_.getrandbits(32),
                "unknown_004771c8": random_.randbytes(0x2580),
                "unknown_00479748": random_.getrandbits(32),
            },
            extra=[made],
            externals={"online_tasks_check_address": lambda harness, arguments: (0, ())},
            mocks=[
                Mock(lambda call, made=made: made.address, stack_arguments=1, registers=("ecx",)),
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
            ],
        )

    # ---------- more of the tasks

    for n in range(2000):
        session = lambda harness, arguments, said=random_.choice((1, 1, 1, 0, 2)): (said, tuple(arguments))
        tasks, number = table()
        t.check(
            "online_task_new_signed_in",
            globals={"online_tasks": tasks, "unknown_00467214": random_.choice((0, 0, 0, 2, NONE))},
            externals={"online_tasks_get_logon_state": session},
        )

        tasks, number = table()
        t.check("online_task_get", number, globals={"online_tasks": tasks})

        # of the tasks that there are: the kind and the owner of one, or others
        data = tasks.data
        index = random_.randrange(COUNT)
        kind, owner = struct.unpack_from("<II", data, TASKS + index * TASK + 4)
        kind = random_.choice((kind, kind, 2, 3, 0x21, 7))
        owner = random_.choice((owner, owner, NONE, 0xFF, 0, random_.getrandbits(32)))
        t.check("online_task_find", owner, kind, globals={"online_tasks": tasks})
        tasks, number = table()
        t.check("online_task_exists", owner, kind, globals={"online_tasks": tasks})

        # a task that there is, for what does not look
        tasks, number = table()
        bits = struct.unpack_from("<I", tasks.data, BITS)[0]
        there = [index for index in range(COUNT) if bits >> index & 1]
        if there:
            index = random_.choice(there)
            number = struct.unpack_from("<H", tasks.data, TASKS + index * TASK)[0] << 16 | index
            t.check("online_task_get_type", number, globals={"online_tasks": tasks})
            tasks = Buffer(tasks.data, pointers=tasks.pointers, places=tasks.places)
            t.check(
                "online_task_restart",
                number,
                globals={"online_tasks": tasks},
                externals={
                    "online_task_continue": lambda harness, arguments: (0x1500F2, (harness.normalize(arguments[0]),)),
                    "XOnlineFriendsEnumerateFinish": lambda harness, arguments: (0, tuple(arguments)),
                    "unknown_0008c550": lambda harness, arguments: (0, tuple(arguments)),
                    "XOnlineTaskClose": lambda harness, arguments: (0, tuple(arguments)),
                },
            )

    # ---------- the tasks go on, and are all done with

    def ordered() -> Buffer:
        """A table of tasks in which no more than one is of each kind that waits for the others, so that all can end."""
        tasks, number = table()
        data = bytearray(tasks.data)
        last = [0, 1, 2, 0x21]
        random_.shuffle(last)
        for index in range(COUNT):
            kind = random_.choice((3, 5, 0xB, 0xB, 0xC, 0x2C, 0x2D, NONE, 0x100))
            if last and random_.random() < 0.4:
                kind = last.pop()
            struct.pack_into("<I", data, TASKS + index * TASK + 4, kind)
        return Buffer(bytes(data), pointers=tasks.pointers, places=tasks.places)

    session = lambda said: lambda harness, arguments: (said, tuple(arguments))

    for n in range(1500):
        t.check(
            "online_tasks_close_all",
            globals={"online_tasks": ordered()},
            externals={
                "online_task_continue": lambda harness, arguments: (0x1500F2, (harness.normalize(arguments[0]),)),
                "XOnlineFriendsEnumerateFinish": lambda harness, arguments: (0, tuple(arguments)),
                "unknown_0008c550": lambda harness, arguments: (0, tuple(arguments)),
                "XOnlineTaskClose": lambda harness, arguments: (0, tuple(arguments)),
            },
        )

        stops = random_.choice((NONE, NONE, 0, 1, 3))

        def does(harness, arguments, stops=stops):
            from test_async import calls_so_far

            return calls_so_far(harness, "online_task_update") != stops, (arguments[0], harness.normalize(arguments[1]))

        tasks, number = table()
        t.check(
            "online_tasks_update",
            globals={
                "online_tasks": tasks,
                "unknown_00467214": random_.choice((0, 0, 0, 2, NONE)),
                "unknown_004771c8": GlobalRange(0x2580),
            },
            externals={
                "online_tasks_get_logon_state": session(random_.choice((0, 1, 1, 1, 2, NONE))),
                "online_task_update": does,
                "unknown_0008bf70": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
            },
        )
        t.check(
            "online_tasks_unknown_0006b550",
            globals={"unknown_00467214": random_.choice((0, 0, 0, 2, NONE))},
            externals={
                "online_tasks_get_logon_state": session(random_.choice((0, 1, 1, 1, 2))),
                "unknown_003a11ae": lambda harness, arguments, said=random_.choice((0, 1, 2, 3, 4, NONE)): said,
            },
        )

    # ---------- a task goes on, and what signing in is at

    ADDRESSES = bytes.fromhex("000d3a5dd1f1") + bytes.fromhex("0050f2105220")

    for n in range(2000):
        t.check(
            "online_tasks_check_address",
            globals={
                "unknown_004cf7cc": random_.choice((ADDRESSES[:6], ADDRESSES[6:], ADDRESSES[:5] + b"\x00", random_.randbytes(6))),
                "unknown_0043ff84": ADDRESSES,
                "unknown_0050944e": bytes((random_.choice((0, 1)),)),
            },
        )

        task = struct.pack(
            "<HHIIII",
            random_.getrandbits(16),
            random_.getrandbits(16),
            random_.choice((0, 0, 1, 2, 3)),
            random_.getrandbits(32),
            random_.choice((0, NONE, 0x1234, 0x1234, 0x1234)),
            random_.getrandbits(32),
        )
        t.check(
            "online_task_continue",
            random_.choice((Buffer(task),) * 6 + (0,)),
            globals={"unknown_00467214": random_.choice((0, 0, 0, 2, NONE))},
            externals={
                "transport_available": lambda harness, arguments, there=random_.choice((0, 1)): there,
                "online_tasks_get_logon_state": session(random_.choice((0, 1, 1, 5))),
                "XOnlineTaskContinue": lambda harness, arguments, said=random_.choice((0, 0x1500F2, 0x80004005)): (said, tuple(arguments)),
            },
        )

        tasks, number = table()
        data = bytearray(tasks.data)
        for index in range(COUNT):
            struct.pack_into("<I", data, TASKS + index * TASK + 4, random_.choice((1, 0, 0, 0, 2, 5)))
        said = random_.choice(
            (0, 0, 0x1510F0, 0x1510F0, 0x1512F0, 0x80151200, 0x80151001, 0x80151002, 0x80151003, 0x80151004, 0x80151005)
            + (0x80151006, 0x80151007, 0x80151000, 0x80004005, 5, 0x801511FF, 0x80151201, NONE)
        )
        t.check(
            "online_tasks_get_logon_state",
            number,
            globals={
                "online_tasks": Buffer(bytes(data), pointers=tasks.pointers, places=tasks.places),
                "unknown_00467218": random_.choice((10, 0, 1, 5)),
                "unknown_0050944f": bytes((random_.choice((0, 0, 1)),)),
            },
            externals={
                "transport_available": lambda harness, arguments, there=random_.choice((1, 1, 1, 0)): there,
                "XOnlineLogonTaskGetResults": lambda harness, arguments, said=said: (said, tuple(arguments)),
            },
        )

    # ---------- who is signed in

    SERVICES = b"".join(
        struct.pack("<3I", number, there, 0)
        for number, there in [(2, 1), (4, 1), (6, 1), (7, 1), (8, 1), (9, 1), (13, 1), (15, 1), (16, 1), (18, 1), (19, 1), (20, 1), (0x4D530064, 0)]
    )
    WAYS = (0, 0, 0x1512F0, 0x80151200, 0x80151001, 5)

    def users() -> Buffer:
        data = bytearray(random_.randbytes(4 * 0x70))
        for index in range(4):
            if random_.random() < 0.3:
                data[index * 0x70:index * 0x70 + 8] = bytes(8)
            struct.pack_into("<I", data, index * 0x70 + 0x6C, random_.choice(WAYS))
        return Buffer(bytes(data))

    def logged() -> tuple:
        there = users()
        return there, lambda harness, arguments, there=there, has=random_.random() < 0.85: there.address if has else 0

    for n in range(2000):
        there, get = logged()
        t.check("online_users_get_result", extra=[there], externals={"XOnlineGetLogonUsers": get})
        there, get = logged()
        t.check("online_get_logon_users", Buffer(random_.randbytes(4 * 0x70)), extra=[there], externals={"XOnlineGetLogonUsers": get})

        def changed(harness, arguments, results=struct.pack("<4I", *(random_.choice(WAYS) for index in range(4)))):
            harness.mu.mem_write(arguments[1], results)
            return 0, (arguments[0],)

        t.check(
            "online_task_users_get_result",
            Buffer(random_.randbytes(0x14)),
            externals={"XOnlineChangeLogonUsersTaskGetResults": changed},
        )
        t.check(
            "online_services_get_identifiers",
            Buffer(random_.randbytes(13 * 4)),
            globals={"unknown_00467178": SERVICES},
        )

        task = random_.choice((0, 0, 0, 2, NONE))
        state = session(random_.choice((0, 1, 1, 1, 5, 10)))
        t.check(
            "online_logon_get_state",
            Buffer(random_.randbytes(4)),
            globals={"unknown_00467214": task},
            externals={"online_tasks_get_logon_state": state},
        )
        t.check("online_signed_in", globals={"unknown_00467214": task}, externals={"online_tasks_get_logon_state": state})

        def got(harness, arguments, value=random_.getrandbits(32), said=random_.choice((0, 0, 1, 0x80004005, NONE))):
            harness.mu.mem_write(arguments[1], struct.pack("<I", value))
            return said, (arguments[0],)

        t.check(
            "online_unknown_0006c800",
            random_.getrandbits(32),
            externals={"GetTickCount": lambda harness, arguments: 0x1234, "XOnlineGetUsers": got},
        )
        thing = bytearray(random_.randbytes(0x20))
        thing[8] = random_.choice((0, 0, 4, 1, 2, 3, 0xFC))
        thing[0x1C] = random_.choice((0, 1, 1, 0xFE, 0xFF))
        t.check("online_unknown_0006c830", Buffer(bytes(thing)))
        t.check("online_unknown_0006c850", random_.choice((*range(14), 0xFF, 0x102, 0xABCD08)))
        t.check(
            "online_notification_set_state",
            random_.choice((Buffer(random_.randbytes(8)), Buffer(random_.randbytes(8)), 0)),
            random_.randrange(4),
            Buffer(random_.randbytes(4)),
            random_.getrandbits(32),
            globals={"unknown_00467214": task},
            externals={
                "online_tasks_get_logon_state": state,
                "XOnlineNotificationSetState": lambda harness, arguments: (
                    0,
                    (*arguments[:5], harness.normalize(arguments[5])),
                ),
            },
        )

    # ---------- what a task does each time around

    SAID = (0, 0, 0x1500F0, 0x1500F1, 0x1500F2, 0x153101, 5, 0x80004005, 0x80151000, 0x8007000E, 0x80150001, 0x80150002)
    SAID += (0x80150003, 0x80150004, 0x80150005, 0x80150006, 0x80150007, 0x80150008, 0x8015C002, 0x8015C003, 0x8015C004)
    SAID += (0x8015C006, 0x8015C007, 0x8015C008, 0x8015C009, 0x8015C00A, NONE)

    def a_task(kinds) -> Buffer:
        return Buffer(
            struct.pack(
                "<HHIIII",
                random_.getrandbits(16),
                random_.choice((1, 1, 1, 1, 3, 9, 0x11, 0x15, 0x1D, 5, 0, 4, 0xC, 0x14, random_.getrandbits(16))),
                random_.choice(kinds),
                random_.getrandbits(32),
                0x1234,
                random_.getrandbits(32),
            )
        )

    for n in range(3000):
        goes = lambda harness, arguments, said=random_.choice(SAID): (said, (harness.normalize(arguments[0]),))
        t.check("online_task_step", a_task((2, 3)), externals={"online_task_continue": goes})
        t.check(
            "online_task_storage_done",
            random_.choice(SAID),
            random_.getrandbits(32),
            externals={"network_blob_task_completed": lambda harness, arguments: (0, tuple(arguments))},
        )
        t.check(
            "online_task_update",
            random_.getrandbits(32),
            a_task((0, 1, 2, 2, 0xD, 0xD, 0x29, 0x2A, 0x2B, 0x2C, 0x2C, 3, 0xC, 0x28, 0x2D, NONE)),
            globals={"unknown_0051055d": bytes((random_.choice((0, 0, 1)),))},
            externals={
                "online_task_continue": goes,
                "online_task_logon_update": lambda harness, arguments, others=random_.choice((0, 1, 1)): (
                    others,
                    (harness.normalize(arguments[0]), arguments[1]),
                ),
                "online_task_change_users_update": lambda harness, arguments: (1, (harness.normalize(arguments[0]),)),
                "unknown_0008cc40": lambda harness, arguments: (0, ()),
                "network_blob_task_completed": lambda harness, arguments: (0, tuple(arguments)),
                "online_task_close": lambda harness, arguments: (0, tuple(arguments)),
            },
        )

    # ---------- the task that signs in, and the one that changes who is signed in

    def logon_task() -> Buffer:
        return Buffer(
            struct.pack(
                "<HHIIII",
                random_.getrandbits(16),
                random_.choice((1, 1, 1, 3, 3, 5, 0x21, 0, random_.getrandbits(16))),
                random_.choice((0, 1)),
                random_.getrandbits(32),
                0x1234,
                random_.choice((0, 1, 2, 5, 10, random_.getrandbits(32))),
            )
        )

    for n in range(3000):
        could = lambda harness, arguments, did=random_.choice((0, 1, 1)): (did, (harness.normalize(arguments[0]),))
        told = lambda harness, arguments: (0, tuple(harness.normalize(value) for value in arguments))
        t.check(
            "online_task_logon_update",
            logon_task(),
            random_.getrandbits(32),
            globals={
                "unknown_004d8b18": bytes((random_.choice((1, 1, 1, 0)),)),
                "unknown_004d8b19": bytes((random_.choice((1, 1, 1, 0)),)),
                "unknown_004cf8e4": Buffer(random_.randbytes(0x4E04)),
                "unknown_004cf95c": bytes((random_.choice((0, 1)),)),
            },
            externals={
                "online_task_continue": told,
                "online_tasks_get_logon_state": session(random_.choice((0, 0, 1, 1, 1, 5, 10))),
                "unknown_0007f410": told,
                "transport_unknown_0008d730": told,
                "online_logon_complete": could,
            },
        )
        said = random_.choice(
            (0, 0x1513F0, 0x1513F1, 0x1513F1, 0x1513F2, 0x80151300, 0x80151300, 0x80151001, 0x80151002, 0x80151003)
            + (0x80151004, 0x80151005, 0x80151006, 0x80151007, 0x80151000, 0x80151301, 0x80004005, 5, NONE)
        )
        t.check(
            "online_task_change_users_update",
            logon_task(),
            externals={
                "online_task_continue": lambda harness, arguments, said=said: (said, (harness.normalize(arguments[0]),)),
                "online_task_users_get_result": lambda harness, arguments, got=random_.choice((0, 1, 2, 2)): (
                    got,
                    (harness.normalize(arguments[0]),),
                ),
                "online_logon_complete": could,
            },
        )

    # ---------- what is done when someone is signed in

    for n in range(3000):
        there = users()
        services = bytearray(random_.randbytes(13 * 12))
        for index in range(13):
            struct.pack_into("<II", services, index * 12, index + 2, random_.choice((1, 1, 0)))
        missing = random_.choice((NONE, NONE, NONE, 0, 5, 12))
        told = lambda harness, arguments: (0, tuple(harness.normalize(value) for value in arguments))

        def service(harness, arguments, missing=missing):
            from test_async import calls_so_far

            return (0x80004005 if calls_so_far(harness, "XOnlineGetServiceInfo") == missing else 0), tuple(arguments)

        t.check(
            "online_logon_complete",
            logon_task(),
            globals={
                "unknown_00467214": random_.choice((0, 0, 0, 0, NONE)),
                "unknown_0050944f": bytes((random_.choice((0, 0, 1)),)),
                "unknown_00467178": bytes(services),
                "unknown_00477058": GlobalRange(0x30),
                "unknown_0047d8f4": GlobalRange(0x30),
                "unknown_00479780": GlobalRange(0x30),
            },
            extra=[there],
            externals={
                "online_tasks_get_logon_state": session(random_.choice((1, 1, 1, 1, 0, 5))),
                "XOnlineGetLogonUsers": lambda harness, arguments, there=there, has=random_.random() < 0.9: there.address if has else 0,
                "online_users_get_result": lambda harness, arguments, got=random_.choice((0, 0, 1, 2)): got,
                "online_task_users_get_result": lambda harness, arguments, got=random_.choice((0, 0, 1, 2)): (
                    got,
                    (harness.normalize(arguments[0]),),
                ),
                "XOnlineGetServiceInfo": service,
                "online_users_start_mute_lists": told,
                "online_users_set_state": told,
                "game_voice_start": told,
                "user_interface_controllers_first_online_user": lambda harness, arguments, first=random_.choice((0, 1, 3, NONE)): (
                    first,
                    (harness.normalize(arguments[0]),),
                ),
                "unknown_0008c660": told,
                "network_blob_start_read": lambda harness, arguments: (1, tuple(harness.normalize(value) for value in arguments)),
                "unknown_0008cc40": told,
                "unknown_0008d4d0": told,
            },
        )

    # ---------- the lists of muted players

    for n in range(2000):
        data = bytearray(random_.randbytes(4 * 0x70))
        for index in range(4):
            if random_.random() < 0.25:
                data[index * 0x70:index * 0x70 + 8] = bytes(8)
            data[index * 0x70 + 8] = random_.choice((0, 0, 0, 4, 1, 2, 0xFC))
            data[index * 0x70 + 0xC] = random_.choice((0, 0x41, 0x41, 0x41))
            struct.pack_into("<I", data, index * 0x70 + 0x6C, random_.choice((0, 0, 0x1512F0, 0x80151200, 0x80000000)))
        told = lambda harness, arguments: (0, tuple(arguments))
        t.check(
            "online_users_start_mute_lists",
            Buffer(bytes(data)),
            externals={"voice_mute_list_stop": told, "voice_mute_list_startup": told, "voice_mute_list_request": told},
        )

    # ---------- what a player is at

    TIMES = (0, 0, 1, 59, 60, 61, 64, 65, 179, 180, 181, 189, 190, 599, 600, 601, 0x7FFF, -1, -0x8000)

    for n in range(3000):
        state = struct.pack(
            "<3I2h",
            random_.choice((1, 2, 3, 4, 5, 6, 0, 7, NONE)),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.choice(TIMES + (random_.randrange(-0x8000, 0x8000),)),
            random_.choice(TIMES + (random_.randrange(-0x8000, 0x8000),)),
        )
        t.check("online_state_encode", Buffer(random_.randbytes(4)), Buffer(state))

        data = bytearray(random_.randbytes(4 * 0x70))
        for index in range(4):
            if random_.random() < 0.25:
                data[index * 0x70:index * 0x70 + 8] = bytes(8)
            data[index * 0x70 + 8] = random_.choice((0, 0, 0, 4, 1, 2))
            data[index * 0x70 + 0xC] = random_.choice((0, 0x41, 0x41, 0x41))
            struct.pack_into("<I", data, index * 0x70 + 0x6C, random_.choice((0, 0, 0x1512F0, 0x80151200)))
        t.check(
            "online_users_set_state",
            Buffer(bytes(data)),
            globals={"unknown_00467214": random_.choice((0, 0, 0, NONE))},
            externals={
                "online_tasks_get_logon_state": session(random_.choice((1, 1, 1, 0))),
                "XOnlineNotificationSetState": lambda harness, arguments: (
                    0,
                    (*arguments[:5], bytes(harness.mu.mem_read(arguments[5], 4))),
                ),
            },
        )

    # ---------- what the service says that a player is at

    for n in range(2000):
        controllers = bytearray(random_.randbytes(4 * 0xC70))
        for index in range(4):
            controllers[index * 0xC70 + 0x478] = random_.choice((0, 0, 0, 0, 4, 1, 2))
        known = random_.getrandbits(32)
        said = bytes((random_.choice((*range(9), 0xFF)),)) + struct.pack("<II", known, random_.choice((0, 4, 8, 0x100, 0x1000000, 0x10C, random_.getrandbits(32)))) + random_.randbytes(3)
        count = random_.choice((0, 0, 1, 3, 6, 125, NONE))
        entries = bytearray(random_.randbytes(8 * 0x40))
        for index in range(8):
            struct.pack_into("<I", entries, index * 0x40 + 0x20, random_.choice((known, random_.getrandbits(32), random_.getrandbits(32))))

        def says(harness, arguments, said=said, is_known=random_.choice((1, 1, 1, 0, 0x100))):
            harness.mu.mem_write(arguments[1], said)
            return is_known, (arguments[0],)

        def has(harness, arguments, count=count, entries=bytes(entries)):
            asked = bytes(harness.mu.mem_read(arguments[2], 4))
            harness.mu.mem_write(arguments[1], entries)
            harness.mu.mem_write(arguments[2], struct.pack("<I", min(count, 8) if count != NONE else NONE))
            return 0, (arguments[0], asked)

        t.check(
            "online_user_get_state",
            random_.randrange(4),
            Buffer(random_.randbytes(0x4C)),
            globals={"unknown_0054e8e0": bytes(controllers), "unknown_00467214": random_.choice((0, 0, 0, 0, NONE))},
            externals={
                "online_tasks_get_logon_state": session(random_.choice((1, 1, 1, 1, 0))),
                "unknown_003a0798": says,
                "unknown_0008e750": has,
            },
        )

    # ---------- no one is signed in any more

    for n in range(1000):
        told = lambda harness, arguments: (0, tuple(harness.normalize(value) for value in arguments))
        t.check(
            "online_logoff",
            globals={
                "unknown_00467214": random_.choice((0, 0x80010002, NONE)),
                "unknown_004cf8e4": Buffer(random_.randbytes(0x4E04)),
                "unknown_004cf95c": bytes((random_.choice((0, 1)),)),
                "unknown_00477058": random_.randbytes(0x30),
                "unknown_0047d8f4": random_.randbytes(0x30),
                "unknown_00479780": random_.randbytes(0x30),
                # who is told: the game has transport_unknown_0008d730 inside
                "unknown_004d8b1c": random_.choice((0, 1, 3)),
                "unknown_004d8b60": struct.pack("<8I", *(mock_address(index) if random_.random() < 0.7 else 0 for index in range(8))),
                "unknown_004d8b80": struct.pack("<8I", *(random_.getrandbits(28) for index in range(8))),
            },
            externals={
                "network_blob_task_end": told,
                "online_tasks_close_all": told,
                "user_interface_controller_sign_out": told,
                "unknown_0007f410": told,
            },
            mocks=[Mock(lambda call: 0, stack_arguments=1) for index in range(8)],
        )

    # ---------- the name of a game, the invitation that was taken, and the address of a service

    FAILED = (0x80004005, 0x80151000, 0xFFFFFFFF)

    for n in range(1500):
        size = random_.choice((1, 2, 3, 8, 16, 16))
        text = "".join(random_.choice("Halo 2?x") for index in range(random_.randrange(size)))

        def named(harness, arguments, result=random_.choice((0, 0, 1, 0x7FFFFFFF) + FAILED), text=text):
            if result < 0x80000000:
                harness.mu.mem_write(arguments[3], (text + "\0").encode("utf-16-le"))
            return result, (*arguments[:3], harness.normalize(arguments[3]))

        t.check(
            "online_title_get_name",
            random_.getrandbits(32),
            Buffer(random_.randbytes(size * 2 + 4)),
            size,
            externals={"XGetLanguage": lambda harness, arguments, language=random_.randrange(10): (language, ()), "XOnlineFriendsGetTitleName": named},
        )

        t.check(
            "online_get_accepted_game_invite",
            Buffer(random_.randbytes(0x40)),
            externals={
                "XOnlineFriendsGetAcceptedGameInvite": lambda harness, arguments, result=random_.choice((0, 0, 1, 0x7FFFFFFF) + FAILED): (
                    result,
                    (harness.normalize(arguments[0]),),
                ),
            },
        )

        services = bytearray()
        for index in range(13):
            services += struct.pack(
                "<III",
                random_.choice((0x14, 0x14, 2, 4, 0x15, 0x4D530064, random_.getrandbits(32))),
                random_.choice((0, 1)),
                random_.choice((0, 0, 1, 1, 0x100, random_.getrandbits(32))),
            )

        def service(harness, arguments, result=random_.choice((0, 0, 0, 1) + FAILED), info=random_.randbytes(12)):
            harness.mu.mem_write(arguments[1], info)
            return result, (arguments[0],)

        t.check(
            "online_service_unregister_address",
            globals={"unknown_00467178": bytes(services)},
            externals={
                "XOnlineGetServiceInfo": service,
                "XNetUnregisterInAddr": lambda harness, arguments: (0, tuple(arguments)),
            },
        )

    # ---------- a player is taken out of the ones that are signed in

    for n in range(2000):
        controllers = bytearray(random_.randbytes(4 * 0xC70))
        for index in range(4):
            struct.pack_into("<I", controllers, index * 0xC70, random_.choice((0, 0x20, 0x20, 0x10, 0xFFFFFFDF, 0xFFFFFFFF)))
        tasks, number = table()

        def changed(harness, arguments, result=random_.choice((0, 0, 0, 1) + FAILED), handle=random_.getrandbits(32)):
            users = bytes(harness.mu.mem_read(arguments[0], 4 * 0x70))
            if result < 0x80000000:
                harness.mu.mem_write(arguments[2], struct.pack("<I", handle))
            return result, (users, arguments[1])

        t.check(
            "online_logon_remove_user",
            random_.choice((0, 1, 2, 3)),
            globals={
                "unknown_00467214": random_.choice((0, 0, 0, 0x80010002, NONE)),
                "unknown_0054e8e0": bytes(controllers),
                "online_tasks": tasks,
            },
            externals={
                "online_task_find": lambda harness, arguments, found=random_.choice((NONE, NONE, 0x10003, 5)): (found, tuple(arguments)),
                "online_task_close": lambda harness, arguments: (0, tuple(arguments)),
                "user_interface_controller_close_tasks": lambda harness, arguments: (0, tuple(arguments)),
                "online_logoff": lambda harness, arguments: (0, ()),
                "online_task_new_signed_in": lambda harness, arguments, number=number: (number, ()),
                "online_service_unregister_address": lambda harness, arguments: (0, ()),
                "XOnlineChangeLogonUsers": changed,
            },
        )

    # ---------- the two texts of each kind of task

    for n in range(6000):
        tasks, number = table()
        data = bytearray(tasks.data)
        for index in range(COUNT):
            struct.pack_into("<I", data, TASKS + index * TASK + 4, random_.choice((random_.randrange(0x30), random_.randrange(0x30), 0x28, 0x2D, 0x2E, NONE, random_.getrandbits(32))))
        tasks = dataclasses.replace(tasks, data=bytes(data))
        t.check("online_task_get_string", number, globals={"online_tasks": tasks})
        t.check("online_task_get_other_string", number, globals={"online_tasks": tasks})
