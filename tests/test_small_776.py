# five small functions that ask the caches for the bitmaps and the geometry
# of the lightmap of the structure bsp, and for the geometry of a part of a
# model

import random
import struct

from difftest import Buffer, Pointer

NONE = 0xFFFFFFFF
# where each thing is in one buffer, so that they can point to each other
LIGHTMAP, GROUP, BLOCK2C, BLOCK4C, BLOCK54, BLOCK64, GEOMETRY = 0x0, 0x100, 0x200, 0x300, 0x400, 0x500, 0x600
BSP, TAGS, BITMAP_GROUP, BITMAPS, MODEL, PARTS, SECTIONS, SIZE = 0xA00, 0xA80, 0xB00, 0xB80, 0x1300, 0x1480, 0x1600, 0x1920
PLACES, TAG_COUNT, MODEL_PARTS = 16, 4, 4


def run(t) -> None:
    random_ = random.Random(776)

    for n in range(600):
        data = bytearray(random_.randbytes(SIZE))
        pointers = []

        def point(at: int, to: int) -> None:
            struct.pack_into("<I", data, at, to)
            pointers.append(at)

        struct.pack_into("<i", data, LIGHTMAP + 0x80, random_.choice((1, 1, 1, 3, 0, -1)))
        point(LIGHTMAP + 0x84, GROUP)
        checksum = random_.getrandbits(32)
        struct.pack_into("<I", data, BSP + 8, checksum)
        struct.pack_into("<I", data, GROUP + 4, checksum if random_.random() < 0.8 else random_.getrandbits(32))
        bitmap_tag = random_.getrandbits(16) << 16 | random_.randrange(TAG_COUNT)
        struct.pack_into("<I", data, GROUP + 0x1C, bitmap_tag if random_.random() < 0.85 else NONE)
        for offset, block in ((0x2C, BLOCK2C), (0x4C, BLOCK4C), (0x54, BLOCK54), (0x64, BLOCK64), (0x44, GEOMETRY)):
            point(GROUP + offset, block)
        for place in range(PLACES):
            for block in (BLOCK2C, BLOCK4C):
                struct.pack_into("<h", data, block + place * 4, random_.choice((-1, 0, 1, 15, random_.randrange(16))))
            for block in (BLOCK54, BLOCK64):
                struct.pack_into("<H", data, block + place * 12 + 2, random_.randrange(16))
        model_tag = random_.getrandbits(16) << 16 | (bitmap_tag + 1) % TAG_COUNT
        for index in range(TAG_COUNT):
            point(TAGS + index * 16 + 8, MODEL if index == model_tag & 0xFFFF else BITMAP_GROUP)
        point(BITMAP_GROUP + 0x48, BITMAPS)
        point(MODEL + 0x144, PARTS)
        point(MODEL + 0x13C, SECTIONS)
        for part in range(MODEL_PARTS):
            struct.pack_into("<h", data, PARTS + part * 0x58 + 0x34, random_.randrange(4))
        world = Buffer(bytes(data), pointers=tuple(pointers))

        def predicted(harness, arguments, said=random_.choice((0, 1))):
            return said, (arguments[0] - world.address,)

        externals = {"texture_cache_bitmap_predict": predicted, "geometry_cache_block_predict": predicted}
        globals_ = {
            "global_structure_lightmap": random_.choice((Pointer(world, LIGHTMAP),) * 5 + (0,)),
            "global_structure_bsp": random_.choice((Pointer(world, BSP),) * 5 + (0,)),
            "global_tag_instances": Pointer(world, TAGS),
        }
        place = random_.randrange(PLACES)

        for name in ("structure_lightmap_predict_bitmap_2c", "structure_lightmap_predict_bitmap_4c"):
            t.check(name, place, extra=[world], globals=globals_, externals=externals)
        for name in ("structure_lightmap_predict_geometry_64", "structure_lightmap_predict_geometry_54"):
            t.check(name, place, extra=[world], globals=globals_, externals=externals)
        t.check(
            "render_model_predict_geometry",
            model_tag,
            random_.randrange(MODEL_PARTS),
            extra=[world],
            globals=globals_,
            externals=externals,
        )
