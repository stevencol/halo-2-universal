# the sound goes down before a game ends, and comes up in a new map

import random
import struct

from difftest import Buffer

KINDS = 8


def run(t) -> None:
    random_ = random.Random(702)

    def set_(harness, arguments):
        # the name is not at the same address in the game and in source/
        name = bytes(harness.mu.mem_read(arguments[0], 0x20)).split(b"\0")[0]
        return 0, (name, arguments[1], arguments[2])

    for n in range(60):
        engine = bytearray(random_.randbytes(0xC9C))
        struct.pack_into("<I", engine, 0xC14, random_.randrange(KINDS))
        kinds = b"".join(struct.pack("<I", random_.choice((0, random_.getrandbits(32)))) for _ in range(KINDS))
        for name in ("sound_fade_for_game_end", "sound_fade_in_for_new_map"):
            t.check(
                name,
                globals={"unknown_004e9ae8": Buffer(bytes(engine)), "unknown_0055e4d0": kinds},
                externals={"sound_classes_set_decibels": set_},
            )
