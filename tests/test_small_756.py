# three small functions of a session that this console offers to the
# matchmaking of the service of games: eight bytes of a player, and the
# attributes that are said of it

import random
import struct

from difftest import Buffer, GlobalData

NONE = 0xFFFFFFFF
GAMEPADS = 4
UNKNOWN_0054E8E0 = 0xC70


def run(t) -> None:
    random_ = random.Random(756)

    for n in range(400):
        table = bytearray(random_.randbytes(GAMEPADS * UNKNOWN_0054E8E0))
        gamepad = random_.randrange(GAMEPADS)
        if random_.random() < 0.2:
            table[gamepad * UNKNOWN_0054E8E0 + 0x4E0 : gamepad * UNKNOWN_0054E8E0 + 0x4E8] = bytes(8)
        elif random_.random() < 0.2:
            table[gamepad * UNKNOWN_0054E8E0 + 0x4E0 : gamepad * UNKNOWN_0054E8E0 + 0x4E4] = bytes(4)
        data = bytearray(random_.randbytes(8))
        struct.pack_into("<i", data, 4, random_.choice((2, 2, 1, 3, 0)))
        t.check(
            "online_matchmaking_unknown_0008f390",
            gamepad,
            Buffer(bytes(data)),
            globals={"unknown_0054e8e0": GlobalData(bytes(table))},
        )

        kind = random_.choice((0, 1, 2, 3, 4, 5, 6, 7, -1, 0x80000000))
        value = random_.choice((0, 1, 2, 3, 4, 6, 8, 9, 0x80000000, random_.getrandbits(32)))
        t.check("online_matchmaking_attribute_identifier", kind & 0xFFFFFFFF)
        t.check("online_matchmaking_attribute_valid", kind & 0xFFFFFFFF, value)
