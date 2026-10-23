# four small functions of the levels and the gamepads: whether something of
# 0x2D3FC bytes is right, the lowest number of the multiplayer levels,
# putting two of them in order, and how many gamepads have a profile

import random
import struct

from difftest import Buffer
from test_user_interface_controller import controllers

NONE = 0xFFFFFFFF
SIZE = 0x2D3FC
GLOBALS = 0x180
LEVELS = 0x18
LEVEL = 0xC64


def run(t) -> None:
    random_ = random.Random(766)

    for n in range(300):
        data = bytearray(random_.randbytes(SIZE))
        struct.pack_into("<I", data, 0, random_.choice((2, 2, 2, 0, 1, 3, 0x102, NONE)))
        checksum = struct.unpack_from("<I", data, 4)[0]
        data = Buffer(bytes(data))
        said = random_.choice((checksum, checksum, checksum ^ 1, checksum ^ 0x80000000, NONE, 0))

        def crc(harness, arguments, data=data, said=said):
            # what it is given: the place, the size, the checksum as it is
            # then and the number that the result goes to
            seen = (
                arguments[0] == data.address,
                arguments[1],
                struct.unpack("<I", harness.mu.mem_read(arguments[0] + 4, 4))[0],
                struct.unpack("<I", harness.mu.mem_read(arguments[2], 4))[0],
            )
            harness.mu.mem_write(arguments[2], struct.pack("<I", said))
            return 0, seen

        t.check("levels_unknown_0019bdc0_valid", data, externals={"crc_checksum_buffer": crc})

    for n in range(600):
        # the globals tag, what it says of the levels, and the block of the
        # multiplayer levels
        count = random_.choice((0, 1, 2, 3, 5, NONE, 0x80000000))
        stored = count if count < 8 else 0
        multiplayer_at = GLOBALS + LEVELS
        data = bytearray(random_.randbytes(multiplayer_at + stored * LEVEL))
        has = random_.choice((0, 1, 1, 1, 1))
        address = random_.choice((multiplayer_at - LEVELS, multiplayer_at - LEVELS, multiplayer_at - LEVELS, 0))
        struct.pack_into("<2I", data, 0x178, has, address)
        struct.pack_into("<2I", data, GLOBALS + 0x10, count, multiplayer_at)
        for index in range(stored):
            struct.pack_into("<I", data, multiplayer_at + index * LEVEL, random_.choice((NONE, NONE, 0, 1, 5, 7, 0x80000000, 0x7FFFFFFF, random_.getrandbits(32))))
        pointers = (0x17C, GLOBALS + 0x14) if address else (GLOBALS + 0x14,)
        t.check("levels_get_lowest_multiplayer_level_number", globals={"global_game_globals": Buffer(bytes(data), pointers=pointers)})

        levels = [bytearray(random_.randbytes(LEVEL)) for _ in range(2)]
        same = random_.random() < 0.3
        for level in levels:
            struct.pack_into("<I", level, 0xC4C, random_.choice((0, 1, 2, NONE, 0x80000000, 0x7FFFFFFF)))
        if same:
            levels[1][0xC4C : 0xC50] = levels[0][0xC4C : 0xC50]
        levels = [Buffer(bytes(level)) for level in levels]
        numbers = [Buffer(struct.pack("<I", random_.getrandbits(32))) for _ in range(2)]

        def level_of(harness, arguments, levels=levels, numbers=numbers):
            first = struct.unpack("<I", bytes(numbers[0].data))[0]
            return (levels[0].address if arguments[0] == first else levels[1].address), tuple(arguments)

        t.check(
            "levels_multiplayer_level_compare",
            numbers[0],
            numbers[1],
            extra=levels,
            externals={"levels_get_multiplayer_level": level_of},
        )

        t.check("user_interface_controllers_count_profiles_long", globals={"unknown_0054e8e0": bytes(controllers(random_))})
