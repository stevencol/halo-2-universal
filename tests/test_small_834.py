# two small functions of the way to the main menu: something of Havok as it
# starts, and seven reals of the height of the viewport of the text

import random
import struct

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(834)

    for n in range(400):
        time = bytearray(random_.randbytes(0x24))
        struct.pack_into("<h", time, 2, random_.choice((30, 60, 7)))
        t.check("havok_unknown_001cf950", Buffer(random_.randbytes(0xA0)), random_.getrandbits(32), globals={"game_time_globals": Buffer(bytes(time))})

        bounds = bytearray(random_.randbytes(16))
        top = random_.choice((0, 10))
        struct.pack_into("<hhh", bounds, 0, top, random_.getrandbits(15), top + random_.choice((0, 480, 720, 1)))
        flags = bytearray(random_.randbytes(8))
        flags[5] = random_.choice((0, 1))
        t.check(
            "rasterizer_unknown_0001beb0",
            globals={
                "rasterizer_text_bounds": bytes(bounds),
                "unknown_00485b28": random_.randbytes(28),
                "unknown_00510c50": random_.choice((0, Buffer(bytes(flags)))),
            },
        )
