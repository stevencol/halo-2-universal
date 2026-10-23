# five small functions of a part of the game that uses the tasks of the
# service of games and keeps a thing of 0x6A2 bytes

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
SIZE = 0x6A2


def run(t) -> None:
    random_ = random.Random(767)

    def says(*values):
        return lambda harness, arguments, said=random_.choice(values): (said, tuple(arguments))

    for n in range(600):
        data = bytearray(random_.randbytes(0x624))
        struct.pack_into("<I", data, 0x61C, random_.choice((NONE, NONE, 0, 3, 0x10002, random_.getrandbits(32))))
        task = Buffer(random_.randbytes(0x40))
        t.check(
            "online_unknown_001a2f70_end_task",
            Buffer(bytes(data)),
            extra=[task],
            externals={
                "online_task_get": lambda harness, arguments, there=random_.choice((0, 1, 1)): (task.address if there else 0, tuple(arguments)),
                "online_task_continue": lambda harness, arguments, said=random_.choice((0, 1, 0x80004005, 0x80155100, random_.getrandbits(32))): (
                    said,
                    (arguments[0] == task.address,),
                ),
                "online_task_close": says(0),
            },
        )

        t.check(
            "online_unknown_001a3230_task_going_on",
            globals={
                "unknown_0046e7d4": random_.choice((NONE, 0, 1, 0x10003)),
                "unknown_0046e7bc": random_.choice((0, 0, 1, 0x100, NONE)),
            },
            externals={"online_task_get_status": says(0, 1, 2, 3, 4, NONE)},
        )

        kept = {
            "unknown_0046e814": random_.randbytes(SIZE),
            "unknown_0046eeb6": bytes((random_.choice((0, 0, 1, 0x80)), random_.choice((0, 1, 2, 0xFF)))),
        }
        t.check("online_unknown_001a325a_get_byte", globals=kept)
        t.check("online_unknown_001a3269_get", Buffer(random_.randbytes(SIZE + 2)), globals=kept)
        t.check("online_unknown_001a3294_clear", globals=kept)
