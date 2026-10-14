# twenty-two small functions of objects, lights, the network and others

import random
import struct

from difftest import Buffer

from test_lights_walks import lights
from test_small_709 import COUNT, OBJECT, objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12


def f32(value: float) -> int:
    return struct.unpack("<I", struct.pack("<f", value))[0]


def run(t) -> None:
    random_ = random.Random(737)

    for n in range(300):
        maybe = lambda size, at, values: random_.choice(
            (0, Buffer(bytes(at) + bytes((random_.choice(values),)) + random_.randbytes(size - at - 1)))
        )
        t.check("unknown_004de2f4_is_set", globals={"unknown_004de2f4": maybe(4, 0, (0, 0, 1, 0xFF))})

        t.check(
            "unknown_004de2d0_get",
            random_.getrandbits(16) << 16 | random_.choice((0, 1, 0x27F, 0x280, 0x7FFF, 0x8000, 0xFFFF, random_.randrange(0x280))),
            globals={"unknown_004de2d0": Buffer(random_.randbytes(0x280 * 4))},
        )

        def some_objects() -> Buffer:
            table = objects(random_)
            data = bytearray(table.data)
            for one in range(COUNT):
                struct.pack_into("<h", data, FIRST + one * OBJECT + 0x342, random_.choice((0, 0x10, 0x200, 0x3F0)))
            return Buffer(bytes(data), pointers=table.pointers)

        object_index = lambda: random_.getrandbits(16) << 16 | random_.randrange(COUNT)
        t.check("object_get_unknown12C_bit1", object_index(), globals={"object_header_data": some_objects()})
        t.check("object_get_unknown342_first", object_index(), globals={"object_header_data": some_objects()})

        t.check(
            "unknown_004ba014_unknown_00030e00",
            random_.getrandbits(24) << 8 | random_.choice((0, 1, 1, 0xFF)),
            globals={"unknown_004ba014": random_.choice((0, 1, 2, 3, 3, 3, 0xFFFFFFFF, 0xFFFFFFFC, random_.getrandbits(32)))},
        )

        t.check("antenna_dispose", globals={"antenna_data": random_.choice((0, Buffer(random_.randbytes(0x4C))))})

        table = lights(random_)
        maximum = struct.unpack_from("<i", table.data, 0x20)[0]
        light_index = random_.randrange(maximum)
        now = random_.choice((struct.unpack_from("<I", table.data, 0x4C + light_index * 0x110 + 0xC)[0], random_.getrandbits(32)))
        t.check(
            "light_set_unknownC",
            random_.getrandbits(16) << 16 | light_index,
            globals={"light_data": table, "unknown_004e0308": now},
        )

        t.check(
            "unknown_004e9194_and_unknown_004e9188",
            globals={
                "unknown_004e9194": random_.choice((0, 1, 0x100, random_.getrandbits(32))),
                "unknown_004e9188": bytes((random_.choice((0, 1, 0xFF)),)),
            },
        )

        t.check("unknown_byte_is_from_none_to_3", Buffer(bytes((random_.choice((0, 1, 3, 4, 5, 0x7F, 0x80, 0xFE, 0xFF)),)) + random_.randbytes(3)))

        t.check("unknown_00510c50_get_unknown5", globals={"unknown_00510c50": maybe(0x24, 5, (0, 1, 0xFF, 7))})

        low, high = sorted((random_.uniform(-10.0, 10.0), random_.uniform(-10.0, 10.0)))
        ranges = lambda: {
            "unknown_0054e858": struct.pack("<f", low),
            "unknown_0054e85c": struct.pack("<f", high),
            "unknown_0054e860": struct.pack("<f", high),
        }
        real = lambda: f32(
            random_.choice(
                (
                    0.0,
                    -0.0,
                    low,
                    high,
                    3600.0,
                    3600.001,
                    -1e-30,
                    float("inf"),
                    float("-inf"),
                    float("nan"),
                    random_.uniform(-12.0, 12.0),
                    random_.uniform(0.0, 4000.0),
                )
            )
        )
        t.check("real_is_in_range_0054e858", real(), globals=ranges())
        t.check("real_is_in_range_0054e860", real(), globals=ranges())
        t.check("real_is_in_range_3600", real())

        t.check("unknown_00485898_is_from_5_to_7", globals={"unknown_00485898": random_.choice((0, 4, 5, 6, 7, 8, NONE, 0x80000005))})

        t.check(
            "real_floor",
            f32(
                random_.choice(
                    (
                        0.0,
                        -0.0,
                        1.0,
                        -1.0,
                        0.5,
                        -0.5,
                        2.9999998,
                        -2.9999998,
                        1e9,
                        -1e9,
                        2147483520.0,
                        -2147483648.0,
                        random_.uniform(-100.0, 100.0),
                        float(random_.randrange(-50, 50)),
                    )
                )
            ),
        )

        byte = lambda: random_.getrandbits(24) << 8 | random_.choice((0, 0, 1, 0xFF))
        t.check("unknown_00040e10", byte(), byte())

        t.check(
            "unknown8_lacks_bit",
            random_.getrandbits(24) << 8 | random_.randrange(32),
            Buffer(random_.randbytes(8) + random_.choice((bytes(4), b"\xff" * 4, random_.randbytes(4), struct.pack("<I", 1 << random_.randrange(32))))),
        )

        t.check(
            "unknown_004cd8f8_get",
            random_.choice((0, Buffer(random_.randbytes(4)))),
            globals={"unknown_004cd8f8": random_.getrandbits(32), "unknown_004cd8fc": random_.getrandbits(32)},
        )

        t.check("game_engine_unknown_0009bb50", *(random_.getrandbits(32) for _ in range(3)), Buffer(random_.randbytes(0x1B4)))
        t.check("simulation_unknown68_set", random_.getrandbits(32), Buffer(random_.randbytes(0x94)), Buffer(random_.randbytes(0x2C)))
        t.check("simulation_unknown68_get", random_.getrandbits(32), Buffer(random_.randbytes(0x94)), Buffer(random_.randbytes(0x2C)))

        t.check("unknown_005093e0_clear_unknownA0", globals={"unknown_005093e0": random_.choice((0, Buffer(random_.randbytes(0xA4))))})
