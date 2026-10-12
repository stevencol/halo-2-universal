# four more functions of parts of the game: the levels, the looping sounds, the decals

import random
import struct

from difftest import Buffer, Mock, mock_address


def run(t) -> None:
    random_ = random.Random(613)

    def note(harness, arguments):
        return 0, tuple(harness.normalize(argument) for argument in arguments)

    def table(harness, arguments):
        return 0, (*(harness.normalize(argument) for argument in arguments), harness.mu.mem_read(arguments[0] + 0x29, 1)[0])

    for n in range(300):
        t.check(
            "levels_initialize_for_new_map",
            globals={"unknown_004ee4e4": Buffer(random_.randbytes(0x4C)), "unknown_004ee4e8": Buffer(random_.randbytes(0x4C))},
            externals={
                "data_delete_all": table,
                "unknown_0019c120": note,
                "unknown_0019c6d0": table,
                "unknown_0019c7e0": table,
            },
        )

        sounds = bytearray(random_.randbytes(0x4C))
        sounds[0x29] = random_.choice((0, 1, 1, 0x80))
        t.check(
            "object_looping_sounds_dispose_from_old_map",
            globals={
                "unknown_004ed28c": random_.choice((0, 1, 1, 1)) and Buffer(bytes(sounds)),
                "unknown_004ed288": Buffer(random_.randbytes(0x40)),
            },
            externals={"unknown_001889d0": note},
        )

        # the cache, with what its memory came from: that has its functions, of which the second gives memory back
        cache = bytearray(random_.randbytes(0x70 + 8 + 8))
        struct.pack_into("<I", cache, 0x6C, 0x70)
        struct.pack_into("<I", cache, 0x70, 0x78)
        struct.pack_into("<I", cache, 0x7C, mock_address(0))
        t.check(
            "decals_dispose",
            globals={
                "decal_data": random_.getrandbits(32),
                "unknown_00509444": random_.choice((0, 0x1234, 0x1234)),
                "unknown_00509448": Buffer(bytes(cache), pointers=(0x6C, 0x70)),
            },
            externals={"D3DResource_Release": note},
            mocks=[Mock(lambda call: 0, stack_arguments=1, registers=("ecx",))],
        )
        t.check(
            "decals_dispose_from_old_map",
            globals={"decal_data": Buffer(random_.randbytes(0x4C)), "unknown_00509448": Buffer(random_.randbytes(0x80))},
            externals={"unknown_0017d5f0": note, "lruv_flush": note},
        )
