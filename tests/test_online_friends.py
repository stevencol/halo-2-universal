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
