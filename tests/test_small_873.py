# three functions of the way to the main menu (guided by halo2-decompiled):
# the rotation between two matrices of a forward and an up, the difference
# of two states, and a query made ready

import random
import struct

from difftest import Buffer, Pointer, f32

DATA = 0x2AD4
FRUSTUM = 0x1BC


def some_vector(random_) -> tuple[float, float, float]:
    return tuple(random_.choice((0.0, 1.0, -1.0, random_.uniform(-1, 1))) for _ in range(3))


def run(t) -> None:
    random_ = random.Random(873)

    for n in range(400):
        t.check(
            "real_math_unknown_00170c00",
            *(Buffer(struct.pack("<3f", *some_vector(random_))) for _ in range(4)),
            Buffer(random_.randbytes(12)),
        )

        t.check(
            "real_math_unknown_00170bb0",
            Buffer(struct.pack("<16f", *(random_.uniform(-2, 2) for _ in range(16)))),
            Buffer(struct.pack("<16f", *(random_.uniform(-2, 2) for _ in range(16)))),
            Buffer(random_.randbytes(13 * 4)),
        )

        # a query: what 0x00 of it points to is after it
        count = random_.choice((0, 1, 2, 3))
        data = bytearray(random_.randbytes(DATA + 4 + 3 * FRUSTUM))
        struct.pack_into("<I", data, 0, DATA)
        data = Buffer(bytes(data), pointers=(0,))
        frusta = random_.choice((Buffer(random_.randbytes(3 * FRUSTUM)), Buffer(random_.randbytes(3 * FRUSTUM)), Pointer(data, DATA + 4)))

        def locate(harness, arguments, location=random_.randbytes(8)):
            harness.mu.mem_write(arguments[0], location)
            return 0, (bytes(harness.mu.mem_read(arguments[1], 12)),)

        t.check(
            "unknown_00132b80",
            random_.getrandbits(32) & ~0x381 | random_.choice((0, 0x80, 0x100, 0x180, 0x200)),
            count,
            data,
            random_.choice((0, 0, 1, 2)),
            frusta,
            random_.choice((0xFFFFFFFF, 0, 5)),
            random_.choice((0, 0x80123450)),
            Buffer(struct.pack("<3f", *(random_.uniform(-50, 50) for _ in range(3)))),
            f32(random_.choice((0.0, 1.0, 3.0, 2.999, 10.0, float("nan")))),
            random_.choice((0, 1, 0x100)),
            f32(random_.uniform(-2, 2)),
            globals={"global_structure_bsp_index": struct.pack("<h", random_.choice((0, 1, -1)))},
            externals={"scenario_location_from_point": locate},
        )
