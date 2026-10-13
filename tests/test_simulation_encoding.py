# source/networking/simulation_encoding.c

import random
import struct

from difftest import Buffer

HEADER = 0x40


def stream(random_, size: int, position: int, error: int = 0, blank: bool = False) -> Buffer:
    """A stream over some bytes that follow it, at a position in bits."""
    data = bytearray(random_.randbytes(size))
    if blank:
        for bit in range(position, size * 8):
            data[bit >> 3] &= ~(1 << (bit & 7))
    header = bytearray(HEADER)
    struct.pack_into("<I4i", header, 0, HEADER, size, random_.getrandbits(31), random_.getrandbits(31), position)
    header[0x14] = error
    return Buffer(bytes(header) + bytes(data), pointers=(0,))


def run(t) -> None:
    random_ = random.Random(731)

    for n in range(300):
        size = random_.choice((4, 8, 16))
        position = random_.choice((0, 0, 3, 8, 13, size * 8 - 20, size * 8 - 4, size * 8, size * 8 + 5))
        position = max(position, 0)
        bits = random_.randrange(1, 17)
        a_point = lambda: Buffer(struct.pack("<3f", *(random_.choice((random_.uniform(-12.0, 12.0), -10.0, 10.0, 0.0)) for _ in range(3))))
        box = []
        for axis in range(3):
            lower = random_.uniform(-50.0, 0.0)
            box += [lower, lower + random_.uniform(1.0, 60.0)]
        bsp = lambda: {"global_structure_bsp": Buffer(random_.randbytes(0x34) + struct.pack("<6f", *box))}

        t.check("simulation_read_world_point", stream(random_, size, position), Buffer(random_.randbytes(12)), bits, globals=bsp())
        t.check("simulation_read_relative_point", bits, stream(random_, size, position), Buffer(random_.randbytes(12)))
        t.check("simulation_write_relative_point", stream(random_, 16, random_.choice((0, 3, 8, 40)), blank=True), bits, a_point())
        first = [random_.uniform(-60.0, 60.0) for _ in range(3)]
        second = [one + random_.choice((0.0, 0.0, 0.01, -0.01, 0.5, 5.0)) for one in first]
        t.check(
            "simulation_world_points_are_close",
            bits,
            Buffer(struct.pack("<3f", *first)),
            Buffer(struct.pack("<3f", *second)),
            globals=bsp(),
        )

        for name in ("0009ae80", "0009f240", "0009fa90", "0009ec30", "0009fd90", "0009ceb0"):
            t.check(
                "simulation_decode_unknown_" + name,
                random_.getrandbits(32),
                Buffer(random_.randbytes(4)),
                stream(random_, size, position),
            )
        ones = bytearray(b"\xff" * 4)
        t.check("simulation_decode_unknown_0009ceb0", random_.getrandbits(32), Buffer(random_.randbytes(4)), Buffer(bytes(stream(random_, 4, 0).data[:HEADER]) + bytes(ones), pointers=(0,)))
        t.check(
            "simulation_decode_unknown_0009cfa0",
            random_.getrandbits(32),
            Buffer(random_.randbytes(4)),
            random_.getrandbits(32),
            Buffer(random_.randbytes(4)),
            stream(random_, size, position),
        )
        t.check(
            "simulation_decode_unknown_000b29a0",
            stream(random_, size, position, error=random_.choice((0, 0, 0, 1))),
            random_.getrandbits(32),
            Buffer(random_.randbytes(4)),
        )

        for name in ("damage_section_response", "game_engine", "game_engine_request_boot_player"):
            length = random_.choice((0x40, 0x40, 0x20, 12, 5, 1))
            t.check(
                "simulation_describe_" + name,
                random_.getrandbits(32),
                random_.getrandbits(32),
                random_.getrandbits(32),
                length,
                Buffer(random_.randbytes(0x40)),
            )
