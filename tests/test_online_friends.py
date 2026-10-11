# source/networking/online_friends.c and source/networking/network_motd.c

import random
import struct

from difftest import Buffer

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
