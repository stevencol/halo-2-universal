# ten small functions: a list of four arrays of the game engine (adding,
# finding, swapping), three bytes of unknown_004e9bc8, six bits moved down,
# and finding things by name in the blocks of a kind of tag

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
NAMES = (0, 0x1234, 0x5678, 0x9ABC)


def run(t) -> None:
    random_ = random.Random(796)

    def word(value: int) -> int:
        return value & 0xFFFFFFFF

    def a_list() -> Buffer:
        """the list (0x18 bytes) and its arrays of eight"""
        data = bytearray(random_.randbytes(0x18 + 8 * 2 + 8 * 4 + 8 * 4 + 8 * 2))
        struct.pack_into("<iH", data, 0, random_.choice((0, 1, 4, 8)), random_.choice((0, 1, 3, 6, 7)))
        struct.pack_into("<IIII", data, 8, 0x18, 0x28, 0x48, 0x68)
        for index in range(8):
            struct.pack_into("<I", data, 0x28 + index * 4, random_.choice((1, 2, 3)))
        return Buffer(bytes(data), pointers=(8, 0xC, 0x10, 0x14))

    def tags() -> Buffer:
        """a tag with blocks at 0x50 (0x38 bytes), 0x70 (0x10 bytes, each with a block of 8 at 0x08) and 0x78 (0x5C)"""
        tag = 0x40
        first, second, third, inner = 0x100, 0x100 + 4 * 0x38, 0x100 + 4 * 0x38 + 4 * 0x10, 0x100 + 4 * 0x38 + 4 * 0x10 + 4 * 0x5C
        data = bytearray(random_.randbytes(inner + 4 * 4 * 8))
        struct.pack_into("<I", data, 8, tag)
        struct.pack_into("<iI", data, tag + 0x50, random_.choice((0, 1, 4, -1)), first)
        struct.pack_into("<iI", data, tag + 0x70, random_.choice((0, 1, 4, -1)), second)
        struct.pack_into("<iI", data, tag + 0x78, random_.choice((0, 1, 4, -1)), third)
        pointers = [8, tag + 0x54, tag + 0x74, tag + 0x7C]
        for index in range(4):
            struct.pack_into("<I", data, first + index * 0x38, random_.choice(NAMES))
            struct.pack_into("<I", data, second + index * 0x10, random_.choice(NAMES))
            data[second + index * 0x10 + 5] = random_.choice((0, 1, 0xFF, 7))
            struct.pack_into("<iI", data, second + index * 0x10 + 8, random_.choice((0, 1, 4, -1)), inner + index * 32)
            pointers.append(second + index * 0x10 + 0x0C)
            struct.pack_into("<I", data, third + index * 0x5C, random_.choice(NAMES))
            for entry in range(4):
                struct.pack_into("<I", data, inner + index * 32 + entry * 8, random_.choice(NAMES))
        return Buffer(bytes(data), pointers=tuple(pointers))

    for n in range(400):
        t.check(
            "game_engine_list_add",
            a_list(),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
        )
        t.check("game_engine_list_find", a_list(), random_.choice((1, 2, 3, 4)))
        t.check(
            "game_engine_list_swap",
            random_.getrandbits(16) << 16 | random_.randrange(8),
            random_.getrandbits(16) << 16 | random_.randrange(8),
            a_list(),
        )
        t.check("unknown_004e9bc8_clear", random_.randrange(2), random_.randrange(2), globals={"unknown_004e9bc8": Buffer(random_.randbytes(2 * 0x20CC))})
        t.check("game_engine_unknown_00168ae0", random_.getrandbits(32))

        tag_index = random_.choice((NONE, random_.getrandbits(16) << 16))
        t.check("tag_unknown_0016d180", tag_index, random_.choice(NAMES), globals={"global_tag_instances": tags()})
        t.check("tag_unknown_0016d1d0", tag_index, random_.choice(NAMES), globals={"global_tag_instances": tags()})
        t.check(
            "tag_unknown_0016d220",
            random_.choice((NONE, 0, 1, 3)),
            tag_index,
            random_.choice(NAMES),
            globals={"global_tag_instances": tags()},
        )
        t.check("tag_unknown_0016d890", random_.getrandbits(16) << 16, word(random_.choice((0, 1, -1, 7, 0xFF))), globals={"global_tag_instances": tags()})
        t.check("tag_unknown_0016da90", tag_index, random_.choice(NAMES), globals={"global_tag_instances": tags()})
