# six small functions of the way to the main menu: a real without its sign,
# where something of Havok is, two numbers by a kind, four things made
# empty, and a number kept with what Havok gives

import random
import struct

from difftest import GlobalData, f32

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(817)

    for n in range(400):
        t.check("real_math_unknown_0001c1a0", f32(random_.uniform(-1000, 1000)))
        t.check("havok_unknown_001473b0", globals={"unknown_004798b0": GlobalData(random_.randbytes(0x10))})
        t.check("havok_unknown_002365e0", random_.choice((0, 1, 2, NONE, random_.getrandbits(32))))
        t.check("game_unknown_0013ac42", random_.randrange(4), globals={"unknown_004e69d0": random_.randbytes(4 * 0x270)})
        t.check("game_unknown_0013ac30", globals={"unknown_004e69d0": random_.randbytes(4 * 0x270)})
        value = random_.getrandbits(32)
        t.check(
            "unknown_00278e60",
            random_.getrandbits(32),
            globals={"unknown_0055e504": random_.getrandbits(32), "unknown_0055e508": random_.getrandbits(32)},
            externals={"havok_unknown_00147090": lambda harness, arguments, value=value: (value, tuple(arguments))},
        )
