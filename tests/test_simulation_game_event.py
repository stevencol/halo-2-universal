# source/networking/simulation_gamestate_entities.c: something that happened in the game, from a message

import hashlib
import random
import struct

from difftest import Buffer

DATUM = 8
COUNT = 6


def run(t) -> None:
    random_ = random.Random(596)

    for n in range(3000):
        # what is read: each read takes the next of the values of this case, by a count kept in the stream
        values = [random_.choice((0, 1, 9, 5, 0xF, random_.getrandbits(32))) for _ in range(16)]
        flags = [random_.choice((0, 0, 1, 0x100)) for _ in range(16)]

        def take(harness, stream):
            count = struct.unpack("<I", harness.mu.mem_read(stream, 4))[0]
            harness.mu.mem_write(stream, struct.pack("<I", count + 1))
            return count % 16

        t.check(
            "simulation_game_event_decode",
            random_.getrandbits(32),
            Buffer(random_.randbytes(0x24)),
            Buffer(bytes(0x20)),
            externals={
                "bitstream_read_integer": lambda harness, arguments, values=values: (
                    values[take(harness, arguments[0])],
                    (arguments[1],),
                ),
                "bitstream_read_bool": lambda harness, arguments, flags=flags: (flags[take(harness, arguments[0])], ()),
            },
        )

        array = bytearray(random_.randbytes(0x4C))
        struct.pack_into("<i", array, 0x24, DATUM)
        struct.pack_into("<i", array, 0x38, random_.choice((COUNT, COUNT, 3)))
        struct.pack_into("<I", array, 0x44, 0x4C)
        datums = bytearray(random_.randbytes(COUNT * DATUM))
        for index in range(COUNT):
            struct.pack_into("<h", datums, index * DATUM, random_.choice((0, 5, -3, 0x7FFF)))
        event = [random_.getrandbits(32) for _ in range(9)]
        for place in (2, 3, 5):
            event[place] = random_.choice((0xFFFFFFFF, 0xFFFFFFFF, 0, 1, 2, 4, 5, 6, 0x80000000))
        t.check(
            "simulation_game_event_apply",
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            Buffer(struct.pack("<9I", *event)),
            globals={"player_data": Buffer(bytes(array) + bytes(datums), pointers=(0x44,))},
            externals={"unknown_0019eb30": lambda harness, arguments: (0, (bytes(harness.mu.mem_read(arguments[0], 0x24)),))},
        )
