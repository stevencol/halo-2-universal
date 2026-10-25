# two functions of the way to the main menu (guided by halo2-decompiled): a
# channel of animations going to a frame, and a color between two tinted

import random
import struct

from difftest import Buffer, f32

from test_object_types_more import after


def r32(value: float) -> float:
    return struct.unpack("<f", struct.pack("<f", value))[0]


def run(t) -> None:
    random_ = random.Random(880)

    for n in range(400):
        # the animation and the events are the test's
        channel = bytearray(random_.randbytes(0x20))
        struct.pack_into("<IHH", channel, 0, random_.randrange(2), random_.getrandbits(16), random_.choice((0, 1, 2)))
        channel[0x11] = random_.choice((0, 0, 1, 2, 8, 0xF6))
        channel[0x12] = random_.choice((0, 1, 1, 0xFE))
        struct.pack_into("<f", channel, 0x18, random_.choice((1.0, 1.0, 0.5, -1.0, 2.0)))
        frames = random_.choice((1, 5, 10, 30))
        # a frame whose next tick (one frame at a rate of 1) gets just to the end
        just = r32(r32(frames - r32(0.0001)) - 1.0)
        animation = bytearray(random_.randbytes(0x20))
        struct.pack_into("<h", animation, 0x14, frames)
        times = bytearray(random_.randbytes(0x24))
        struct.pack_into("<f", times, 4, random_.choice((1 / 30, 1 / 30, 1 / 60)))
        channel = Buffer(bytes(channel))
        tags = Buffer(random_.randbytes(0x20))
        times = Buffer(bytes(times))
        place = after(channel, tags, times)
        t.check(
            "havok_unknown_001c6560",
            channel,
            f32(random_.choice((just, just, 0.0, -1.0, 2.0, frames - 1.0, frames - 0.00005, float(frames), frames + 3.0, random_.uniform(0, frames)))),
            globals={"global_tag_instances": tags, "game_time_globals": times},
            extra=[Buffer(bytes(animation))],
            externals={
                "havok_unknown_001daea0": lambda harness, arguments, place=place: (place, tuple(arguments)),
                "havok_unknown_001c69b0": lambda harness, arguments: (0, tuple(arguments)),
            },
        )

        # colors of an alpha and three reals; the color between them is t's
        def color():
            return struct.pack("<4f", random_.choice((0.0, 0.0, 0.00005, 0.5, 1.0)), *(random_.uniform(0, 1) for _ in range(3)))

        tint = random_.choice((None, Buffer(struct.pack("<3f", *(random_.uniform(0, 1) for _ in range(3))))))
        t.check(
            "real_math_unknown_00131d60",
            Buffer(random_.randbytes(12)),
            Buffer(color()),
            Buffer(color()),
            tint if tint else 0,
            random_.choice((0, 1, 2, 3)),
            f32(random_.uniform(0.001, 0.999)),
        )
