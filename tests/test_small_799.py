# seven small functions of the sound: a tag and the kind of an object, two
# bits of the elements of unknown_004ed28c, ten longs compared, tags found in
# a block and by name, and a point near the game's sound entries

import random
import struct

from difftest import Buffer, f32

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(799)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    def data_array(count: int, size: int) -> Buffer:
        data = bytearray(random_.randbytes(0x50 + count * size))
        struct.pack_into("<I", data, 0x44, 0x50)
        return Buffer(bytes(data), pointers=(0x44,))

    for n in range(400):
        tags = bytearray(random_.randbytes(0x40 + 4 * 0x10))
        for tag in range(4):
            struct.pack_into("<I", tags, tag * 0x10 + 8, 0x40 + tag * 0x10)
            tags[0x40 + tag * 0x10] = random_.choice((0, 0x40, 0xBF, 0xFF))
        table = some_objects(random_)
        headers = bytearray(table.data)
        for index in range(COUNT):
            headers[0x4C + index * 12 + 3] = random_.choice((0, 1, 2, 2, 34, 0xFF))
        t.check(
            "sound_unknown_00189010",
            random_.choice((NONE, salted(random_.randrange(COUNT)))),
            random_.choice((NONE, salted(random_.randrange(4)))),
            globals={
                "global_tag_instances": Buffer(bytes(tags), pointers=tuple(tag * 0x10 + 8 for tag in range(4))),
                "object_header_data": Buffer(bytes(headers), pointers=table.pointers),
            },
        )

        t.check("unknown_004ed28c_set_bit1", salted(random_.randrange(4)), globals={"unknown_004ed28c": data_array(4, 0x18)})
        t.check(
            "unknown_004ed28c_set_bit4",
            salted(random_.randrange(4)),
            random_.choice((0, 1, 0x100)),
            globals={"unknown_004ed28c": data_array(4, 0x18)},
        )

        first = random_.randbytes(40)
        second = bytearray(first)
        if n % 2:
            second[random_.randrange(40)] ^= 1 << random_.randrange(8)
        t.check("sound_unknown_0018c960", Buffer(first), Buffer(bytes(second)))

        # a tag with elements (0x58) at 0x1C naming other tags, and elements (0x38) at 0x00
        data = bytearray(random_.randbytes(0x40 + 0x30 + 4 * 0x58 + 4 * 0x38 + 4 * 0x10))
        pointers = []
        main = 0x40
        others = 0x70 + 4 * 0x58 + 4 * 0x38
        struct.pack_into("<I", data, 8, main)
        pointers.append(8)
        struct.pack_into("<iI", data, main, random_.choice((0, 1, 4, -1)), 0x70 + 4 * 0x58)
        struct.pack_into("<iI", data, main + 0x1C, random_.choice((0, 1, 4, -1)), 0x70)
        pointers += [main + 4, main + 0x20]
        for element in range(4):
            struct.pack_into("<I", data, 0x70 + element * 0x58 + 0x20, random_.choice((NONE, salted(1), salted(2), salted(3))))
            struct.pack_into("<I", data, 0x70 + 4 * 0x58 + element * 0x38, random_.choice((1, 2, 3)))
        for tag in (1, 2, 3):
            struct.pack_into("<I", data, tag * 0x10 + 8, others + tag * 0x10)
            pointers.append(tag * 0x10 + 8)
            data[others + tag * 0x10 + 2] = random_.choice((0x20, 0x26, 0x21, 0xA6))
        instances = lambda: {"global_tag_instances": Buffer(bytes(data), pointers=tuple(pointers))}
        t.check("sound_unknown_0018d360", salted(0), globals=instances())
        t.check("sound_unknown_0018d630", random_.choice((NONE, salted(0))), random_.choice((1, 2, 3, 4)), globals=instances())

        sounds = bytearray(random_.randbytes(0xC0 + 4 * 0x48))
        for entry in range(4):
            at = 0x78 + entry * 0x48
            sounds[at + 0x16] = random_.choice((0, 1))
            struct.pack_into("<3I", sounds, at + 0x40, *(f32(random_.uniform(-3, 3)) for _ in range(3)))
        # sometimes the point of an entry itself with no radius: on the edge
        point = bytes(sounds[0x78 + 0x40 : 0x78 + 0x4C]) if n % 5 == 0 else struct.pack("<3I", *(f32(random_.uniform(-3, 3)) for _ in range(3)))
        if n % 5 == 0:
            sounds[0x78 + 0x16] = 1
        t.check(
            "sound_unknown_0018d670",
            Buffer(point),
            f32(0.0) if n % 5 == 0 else f32(random_.choice((0.0, 1.0, 2.0, 4.0, random_.uniform(0, 5)))),
            globals={"game_sound_globals": Buffer(bytes(sounds))},
        )
