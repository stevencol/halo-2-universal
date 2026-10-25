# two functions of the way to the main menu (guided by halo2-decompiled): the
# resources of the animations of a state asked for, and whether one of ten
# buttons is down

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(882)

    for n in range(400):
        state = bytearray(random_.randbytes(0x80))
        struct.pack_into("<I", state, 0x68, random_.choice((NONE, 0, 1)))
        names = random_.randbytes(16)

        def resolve(harness, arguments, names=names):
            harness.mu.mem_write(arguments[3], names)
            return 0, (arguments[0], arguments[1], arguments[2], arguments[4], arguments[5])

        t.check(
            "havok_unknown_001ce180",
            Buffer(bytes(state)),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.choice((0, 1, 0x100)),
            random_.choice((0, 1, 0x100)),
            globals={"global_tag_instances": Buffer(random_.randbytes(2 * 16))},
            externals={
                "havok_unknown_001cc2f0": resolve,
                "havok_unknown_001ddd00": lambda harness, arguments: (0, tuple(arguments[:4]) + (arguments[4] & 0xFF, arguments[5] & 0xFF)),
            },
        )

        down = {button: random_.random() < 0.08 for button in range(16)}
        t.check(
            "input_unknown_00156ab0",
            externals={"input_unknown_0023e400": lambda harness, arguments, down=down: (int(down.get(arguments[0], False)), tuple(arguments))},
        )
