# source/networking/online_friends.c and source/networking/network_motd.c

import random
import struct

from difftest import Buffer
from test_online_storage import tasks_table

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(548)

    def same(harness, arguments, so=None):
        return (random.Random(arguments[0]).choice((0, 1, 1, 0x100)), tuple(arguments))

    for n in range(1500):
        t.check("online_title_is_same_title", random_.getrandbits(32), externals={"XOnlineTitleIdIsSameTitle": same})
        t.check("online_friend_is_same_title", Buffer(random_.randbytes(0x40)), externals={"XOnlineTitleIdIsSameTitle": same})
        t.check("online_unknown_0008cc00", Buffer(random_.randbytes(4 + 32 * 0x200)))
        length = random_.choice((0, 1, 20, 254, 255, 256, 300))
        text = "".join(random_.choice("Halo 2 clan") for index in range(length)).encode("utf-16-le") + bytes(2)
        t.check("online_unknown_0008cc20", Buffer(random_.randbytes(0x200)), Buffer(text + random_.randbytes(8)))
        friend = bytearray(random_.randbytes(0x40))
        name = bytes(random_.choice(b"Chief117x") for index in range(random_.choice((0, 1, 8, 15, 16, 20))))
        if len(name) < 16:
            friend[0xC:0xC + len(name) + 1] = name + bytes(1)
        else:
            friend[0xC:0xC + len(name)] = name
        friend = bytes(friend[:0x40]) if len(friend) >= 0x40 else bytes(friend)
        t.check("online_friend_get_identity", Buffer(friend + random_.randbytes(8)), Buffer(random_.randbytes(0x56 + 6)))

        t.check(
            "network_motd_get_file",
            Buffer(random_.randbytes(0x110)),
            globals={"network_motd_file_name": Buffer(b"message_of_the_day.dat" + bytes(2))},
        )
        blob = bytearray(random_.randbytes(0x30))
        struct.pack_into("<HHH", blob, 4, random_.choice((2, 2, 3, 0, 0xFFFD, 0xFFFF)), random_.getrandbits(16), random_.choice((0, 1, 4, 0xFFFF)))
        t.check("network_motd_available", globals={"unknown_00479780": bytes(blob)})

    # ---------- what a friend is at, and what is done with one

    BITS = (0x1, 0x2, 0x8, 0x10, 0x4000000, 0x8000000, 0x20000000, 0x40000000, 0x80000000)

    def state_bits() -> int:
        value = 0
        for bit in BITS:
            if random_.random() < 0.4:
                value |= bit
        return value | (random_.getrandbits(32) if random_.random() < 0.2 else 0)

    def a_friend() -> bytes:
        data = bytearray(random_.randbytes(0x56))
        struct.pack_into("<I", data, 0x1C, state_bits())
        return bytes(data)

    for n in range(2500):
        signed_in = {
            "online_tasks_get_logon_state": lambda harness, arguments, state=random_.choice((1, 1, 1, 0, 2)): (state, tuple(arguments)),
        }
        task = {"unknown_00467214": random_.choice((0, 0, 0, 0x80010002, NONE))}
        given = lambda harness, arguments: (0, (arguments[0], bytes(harness.mu.mem_read(arguments[1], 0x56)), *arguments[2:]))

        t.check("online_friend_state_from_library", state_bits(), random_.getrandbits(32), externals={"XOnlineTitleIdIsSameTitle": same})
        t.check("online_friend_state_to_library", random_.choice((random_.getrandbits(11), random_.getrandbits(32))))
        t.check("online_friend_get_state", Buffer(a_friend()), externals={"XOnlineTitleIdIsSameTitle": same})
        t.check("online_friend_get_summary", Buffer(a_friend()), Buffer(random_.randbytes(0x20)), externals={"XOnlineTitleIdIsSameTitle": same})
        t.check(
            "online_friend_remove",
            Buffer(a_friend()),
            random_.randrange(4),
            globals=task,
            externals={**signed_in, "XOnlineFriendsRemove": given},
        )
        t.check(
            "online_friend_answer_request",
            random_.choice((0, 1, 2, 3, NONE)),
            Buffer(a_friend()),
            random_.randrange(4),
            globals=task,
            externals={**signed_in, "XOnlineFriendsAnswerRequest": given},
        )
        t.check(
            "online_friend_answer_game_invite",
            Buffer(a_friend()),
            random_.randrange(4),
            random_.randrange(3),
            globals=task,
            externals={**signed_in, "XOnlineFriendsAnswerGameInvite": given},
        )

        def named(harness, arguments, result=random_.choice((0, 0, 1, 0x80004005)), text="Halo 2"):
            if result < 0x80000000:
                harness.mu.mem_write(arguments[3], (text + "\0").encode("utf-16-le"))
            return result, (*arguments[:3], harness.normalize(arguments[3]) if False else 0)

        t.check(
            "online_friend_get_title_name",
            Buffer(random_.randbytes(0x40)),
            Buffer(a_friend()),
            random_.choice((0x20, 0x10, 1, 0x7FFF)),
            globals=task,
            externals={
                **signed_in,
                "XOnlineTitleIdIsSameTitle": same,
                "XGetLanguage": lambda harness, arguments, language=random_.randrange(10): (language, ()),
                "XOnlineFriendsGetTitleName": named,
            },
        )

    # ---------- the list of the friends of a player

    FAILED = (0x80004005, 0x80151000, 0xFFFFFFFF)
    for n in range(2500):
        tasks, number = tasks_table(random_)

        def starts(harness, arguments, result=random_.choice((0, 0, 0, 1) + FAILED), handle=random_.getrandbits(32)):
            if result < 0x80000000:
                harness.mu.mem_write(arguments[2], struct.pack("<I", handle))
            return result, tuple(arguments[:2])

        t.check(
            "online_friends_enumerate_start",
            random_.randrange(4),
            globals={"online_tasks": tasks},
            externals={
                "online_task_new_signed_in": lambda harness, arguments, index=random_.choice((number, number, number, NONE, number ^ 0x10000)): (index, ()),
                "XOnlineFriendsEnumerate": starts,
                "online_task_close": lambda harness, arguments: (0, tuple(arguments)),
            },
        )

        tasks, number = tasks_table(random_)

        def latest(harness, arguments, count=random_.choice((0, 1, 3, 100, 0x10005))):
            harness.mu.mem_write(arguments[2], bytes((count & 0xFF,)) * 8)
            return count, tuple(arguments[:2])

        t.check(
            "online_friends_get_latest",
            random_.choice((number, number, number, number, NONE, number ^ 0x10000, number | 0xFFF8)),
            Buffer(random_.randbytes(0x20)),
            globals={"online_tasks": tasks, "unknown_00467214": random_.choice((0, 0, 0, 0x80010002, NONE))},
            externals={
                "online_tasks_get_logon_state": lambda harness, arguments, state=random_.choice((1, 1, 1, 1, 0, 2)): (state, tuple(arguments)),
                "online_task_get_status": lambda harness, arguments, status=random_.choice((0, 1, 2, 2, 3, 4, 5)): (status, tuple(arguments)),
                "XOnlineFriendsGetLatest": latest,
            },
        )

    # ---------- the friends start; new content is asked about; a player's action is kept

    for n in range(2500):
        tasks, number = tasks_table(random_)

        def starts(at: int):
            def stand_in(harness, arguments, result=random_.choice((0, 0, 0, 1) + FAILED), handle=random_.getrandbits(32)):
                if result < 0x80000000:
                    harness.mu.mem_write(arguments[at], struct.pack("<I", handle))
                return result, tuple(arguments[:at])

            return stand_in

        closed = lambda harness, arguments: (0, tuple(arguments))
        t.check(
            "online_friends_startup",
            globals={"online_tasks": tasks},
            externals={
                "online_task_find": lambda harness, arguments, found=random_.choice((number, number, NONE)): (found, tuple(arguments)),
                "online_task_restart": closed,
                "online_task_new_signed_in": lambda harness, arguments, index=random_.choice((number, number, NONE)): (index, ()),
                "XOnlineFriendsStartup": starts(1),
                "online_task_close": closed,
            },
        )

        tasks, number = tasks_table(random_)
        t.check(
            "online_new_content_check_start",
            globals={"online_tasks": tasks, "unknown_0051055d": bytes((random_.choice((0, 1)),))},
            externals={
                "online_task_new_signed_in": lambda harness, arguments, index=random_.choice((number, number, number, NONE, number ^ 0x10000)): (index, ()),
                "XOnlineOfferingIsNewContentAvailable": starts(2),
                "online_task_close": closed,
            },
        )

        update = bytearray(random_.randbytes(0x50 + 0x34 + 0x40))
        struct.pack_into("<I", update, 4, random_.randrange(16))
        pointers = []
        for offset, target in ((0x44, 0x50), (0x4C, 0x84)):
            if random_.random() < 0.7:
                struct.pack_into("<I", update, offset, target)
                pointers.append(offset)
            else:
                struct.pack_into("<I", update, offset, 0)
        t.check(
            "simulation_player_actions_add",
            Buffer(random_.randbytes(0x720 + 16 * 0x40)),
            Buffer(bytes(update), pointers=tuple(pointers)),
        )

    # ---------- what a player says of another

    for n in range(2500):
        tasks, number = tasks_table(random_)

        def said(harness, arguments, result=random_.choice((0, 0, 0, 1) + FAILED), handle=random_.getrandbits(32)):
            nickname = bytes(harness.mu.mem_read(arguments[5], 4))
            if result < 0x80000000:
                harness.mu.mem_write(arguments[7], struct.pack("<I", handle))
            return result, (*arguments[:5], nickname, arguments[6])

        t.check(
            "online_feedback_send",
            Buffer(random_.randbytes(12)),
            random_.randrange(4),
            random_.randrange(10),
            globals={"online_tasks": tasks},
            externals={
                "online_task_new_signed_in": lambda harness, arguments, index=random_.choice((number, number, number, NONE, number ^ 0x10000)): (index, ()),
                "XOnlineFeedbackSend": said,
                "online_task_close": lambda harness, arguments: (0, tuple(arguments)),
            },
        )

    # ---------- what a friend is at in this game

    for n in range(8000):
        friend = bytearray(a_friend())
        struct.pack_into("<I", friend, 0x1C, state_bits() | random_.choice((1, 1, 1, 0)))
        friend[0x34] = random_.choice((4, 4, 4, 5, 3, 0, 0xFF))
        friend[0x35:0x37] = random_.choice((b"\xf1\xfc", b"\xf1\xfc", b"\xf1\xfc", b"\xf1\xfc", b"\xf1\xfd", random_.randbytes(2)))
        friend[0x37] = random_.choice((0, 59, 60, 83, 84, 125, 126, 127, 186, 187, 210, 211, 252, 253, 255, random_.randrange(256)))
        friend[0x38] = random_.getrandbits(8)
        t.check(
            "online_friend_get_online_state",
            Buffer(bytes(friend)),
            Buffer(random_.randbytes(0x10)),
            globals={"unknown_00467214": random_.choice((0, 0, 0, 0, 0x80010002, NONE))},
            externals={
                "online_tasks_get_logon_state": lambda harness, arguments, state=random_.choice((1, 1, 1, 1, 0)): (state, tuple(arguments)),
                "XOnlineTitleIdIsSameTitle": same,
                "XOnlineTitleIdIsSamePublisher": lambda harness, arguments, so=random_.choice((1, 1, 1, 0, 0x100)): (so, tuple(arguments)),
            },
        )

    # ---------- the message of the day is written

    for n in range(1500):
        message = Buffer(random_.randbytes(0x40))
        state = {}

        def created(harness, arguments, so=random_.choice((1, 1, 1, 1, 0))):
            state["file"] = arguments[0]
            return so, (bytes(harness.mu.mem_read(arguments[0], 0x30)),)

        def opened(harness, arguments, so=random_.choice((1, 1, 1, 1, 0)), handle=random_.getrandbits(32)):
            # the file is open: it has what the console knows it by
            if so:
                harness.mu.mem_write(arguments[0] + 0x108, struct.pack("<I", handle))
            harness.mu.mem_write(arguments[2], struct.pack("<I", 0))
            return so, (arguments[0] == state.get("file"), arguments[1])

        def wrote(harness, arguments, results=[random_.choice((1, 1, 1, 0)) for index in range(2)]):
            # where in the message, not where the message is
            calls = sum(1 for call in harness._calls if call[0] == "file_write")
            return results[calls % 2], (arguments[0] == state.get("file"), arguments[1], arguments[2] - message.address)

        t.check(
            "network_motd_write_file",
            message,
            globals={
                "network_motd_file_name": Buffer(b"message_of_the_day.dat" + bytes(2)),
                "global_language": random_.choice((NONE, 0, 1, 3, 8)),
            },
            externals={
                "file_create": created,
                "file_open": opened,
                "XGetLanguage": lambda harness, arguments, language=random_.randrange(1, 10): (language, ()),
                "file_write": wrote,
                "CloseHandle": lambda harness, arguments, so=random_.choice((1, 1, 0)): (so, tuple(arguments)),
                "GetLastError": lambda harness, arguments: (5, ()),
                "SetLastError": lambda harness, arguments: (0, tuple(arguments)),
                "file_delete": lambda harness, arguments: (1, (arguments[0] == state.get("file"),)),
            },
        )
