# source/sound/sound_manager.c: two short functions, apart from the tests of the rest

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
SOUND_TAG = struct.unpack("<I", b"!dns")[0]


def run(t) -> None:
    random_ = random.Random(566)

    for n in range(2000):
        numbers = bytearray(random_.randbytes(0x28 + 64 * 2))
        for index in range(64):
            struct.pack_into("<H", numbers, 0x28 + index * 2, random_.choice((0, 1, 0x7FFF, 0x8000, 0xFFFE, 0xFFFF, random_.getrandbits(16))))
        t.check("sound_unknown_0018cac0", random_.randrange(64), globals={"unknown_004ed288": Buffer(bytes(numbers))})

        # eight tags, and after them what each has
        count, sound_size = 8, 0x20
        tags = bytearray(random_.randbytes(count * 16 + count * sound_size))
        pointers = []
        for index in range(count):
            group = random_.choice((SOUND_TAG, SOUND_TAG, SOUND_TAG, struct.unpack("<I", b"dnsl")[0]))
            struct.pack_into("<III", tags, index * 16, group, random_.getrandbits(32), count * 16 + index * sound_size)
            pointers.append(index * 16 + 8)
            tags[count * 16 + index * sound_size + 0xD] = random_.choice((0, 1, 5, 0x7F, 0xFF, 0xFF, 0x80))
        gestalt = bytearray(random_.randbytes(0x48))
        struct.pack_into("<II", gestalt, 0x30, 8, 0x1000)
        t.check(
            "sound_get_gestalt_unknown30",
            random_.choice((NONE, NONE, *range(count), *range(count), 0x12340003)),
            globals={
                "global_tag_instances": Buffer(bytes(tags), pointers=tuple(pointers)),
                "global_sound_cache_file_gestalt": Buffer(bytes(gestalt), pointers=(0x34,)),
            },
        )
