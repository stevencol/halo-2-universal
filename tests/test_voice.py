# source/networking/voice.c

import random
import struct

from difftest import Buffer

VOICE_SIZE = 0x90
VOICE_KIND = 8
PARAMETERS_SIZE = 0x24


def run(t) -> None:
    random_ = random.Random(263)

    def voice(kind: int | None = None) -> Buffer:
        data = bytearray(random_.randbytes(VOICE_SIZE))
        if kind is not None:
            struct.pack_into("<I", data, VOICE_KIND, kind & 0xFFFFFFFF)
        return Buffer(bytes(data))

    for _ in range(4):
        t.check("voice_reset_controller_levels", voice())

    for kind in (0, 1, 2, 3, -1, 0x101):
        t.check("voice_reset_controllers", voice(kind))

    for kind in (0, 1, 2, 3, -1, 0x102):
        for unknown in (0x80345000, 0, 1):
            t.check(
                "voice_engine_parameters_new",
                Buffer(random_.randbytes(PARAMETERS_SIZE)),
                voice(kind),
                globals={"unknown_00510c90": Buffer(struct.pack("<I", unknown))},
            )
