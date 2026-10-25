# three functions of the way to the main menu (guided by halo2-decompiled):
# how the geometry of something is, whether an object belongs to a player,
# and the matrices of the nodes of a render model

import random
import struct

from difftest import Buffer

from test_object_types_more import after

NONE = 0xFFFFFFFF
OBJECTS = 6
OBJECT = 0x140


def objects_table(random_) -> tuple[bytearray, list[int]]:
    """A table of objects as a data array (12 bytes an entry), each with its children, next and parents among the later ones."""
    first = 0x4C
    objects = first + OBJECTS * 12
    table = bytearray(random_.randbytes(objects + OBJECTS * OBJECT))
    struct.pack_into("<i", table, 0x24, 12)
    struct.pack_into("<i", table, 0x38, random_.choice((OBJECTS, OBJECTS, 4)))
    struct.pack_into("<I", table, 0x44, first)
    pointers = [0x44]
    salts = [random_.choice((1, 0x22, 0x7FFF, 0)) for _ in range(OBJECTS)]
    for index in range(OBJECTS):
        struct.pack_into("<hBB", table, first + index * 12, salts[index], random_.getrandbits(8), random_.choice((0, 1, 2, 3, 4, 33)))
        struct.pack_into("<I", table, first + index * 12 + 8, objects + index * OBJECT)
        pointers.append(first + index * 12 + 8)
    for index in range(OBJECTS):
        at = objects + index * OBJECT
        later = list(range(index + 1, OBJECTS))

        def some(chance):
            if not later or random_.random() > chance:
                return NONE
            other = random_.choice(later)
            salt = salts[other] if random_.random() < 0.85 else salts[other] ^ 1
            return (salt & 0xFFFF) << 16 | other

        struct.pack_into("<III", table, at + 0x0C, some(0.5), some(0.5), some(0.5))
        struct.pack_into("<I", table, at + 0x13C, random_.choice((NONE, NONE, NONE, 0, 3)))
        table[at + 0xAA] = random_.choice((0, 1, 2, 3, 4, 5))
        table[at + 0x12C] = random_.choice((0, 0, 8, 0xF7))
    return table, pointers, salts


def run(t) -> None:
    random_ = random.Random(867)

    for n in range(400):
        # the lightmap of the structure bsp, and its tables, in one buffer
        lightmaps = bytearray(random_.randbytes(0x400))
        struct.pack_into("<iI", lightmaps, 0x80, random_.choice((0, 1, 1, 1, -1)), 0x90)
        lightmap = 0x90
        tag = random_.getrandbits(32)
        struct.pack_into("<I", lightmaps, lightmap + 0x04, tag)
        struct.pack_into("<I", lightmaps, lightmap + 0x1C, random_.choice((NONE, 0, 0, 0)))
        tables = {0x44: 0x100, 0x54: 0x200, 0x5C: 0x260, 0x64: 0x2C0}
        for offset, place in tables.items():
            struct.pack_into("<I", lightmaps, lightmap + offset, place)
        for instance in range(4):
            for table in (0x200, 0x2C0):
                struct.pack_into("<H", lightmaps, table + instance * 12 + 2, random_.randrange(4))
            struct.pack_into("<I", lightmaps, 0x260 + instance * 12 + 8, random_.choice((0x1234, 0x5678)))
        pointers = (0x84,) + tuple(lightmap + offset for offset in tables)
        bsp = bytearray(random_.randbytes(0x150 + 4 * 0x58))
        struct.pack_into("<I", bsp, 0x08, random_.choice((tag, tag, tag ^ 1)))
        struct.pack_into("<I", bsp, 0x144, 0x150)
        for instance in range(4):
            struct.pack_into("<h", bsp, 0x150 + instance * 0x58 + 0x56, random_.choice((0, 0, 1, -1)))
        tags = bytearray(random_.randbytes(0x20 + 2 * 0x10))
        for index in range(2):
            struct.pack_into("<I", tags, index * 16 + 8, 0x20 + index * 0x10)
            struct.pack_into("<I", tags, 0x20 + index * 0x10 + 8, random_.choice((0x1234, 0x5678)))
        lightmaps = Buffer(bytes(lightmaps), pointers=pointers) if random_.random() < 0.9 else None
        bsp = Buffer(bytes(bsp), pointers=(0x144,)) if random_.random() < 0.95 else None
        tags = Buffer(bytes(tags), pointers=(8, 0x18))
        object_ = Buffer(random_.randbytes(0x10))
        place = after(*(buffer for buffer in (lightmaps, bsp, tags) if buffer))
        t.check(
            "unknown_0002d000",
            random_.choice((NONE, 0, 7)),
            random_.choice((NONE, 0, 1, 0x12340001)),
            random_.choice((0x7FF, 0, 1, 2, 3)),
            globals={
                "global_structure_lightmap": lightmaps if lightmaps else 0,
                "global_structure_bsp": bsp if bsp else 0,
                "global_tag_instances": tags,
            },
            extra=[object_],
            externals={
                "geometry_cache_block_load": lambda harness, arguments, loaded=random_.choice((0, 1, 1)): (loaded, tuple(arguments)),
                "object_try_and_get_and_verify_type": lambda harness, arguments, found=random_.choice((0, place)): (found, tuple(arguments)),
            },
        )

        # whether an object belongs to a player
        table, table_pointers, salts = objects_table(random_)
        index = random_.randrange(OBJECTS)
        t.check(
            "object_unknown_000beb30",
            (salts[index] & 0xFFFF) << 16 | index,
            globals={"object_header_data": Buffer(bytes(table), pointers=tuple(table_pointers))},
        )

        # the matrices of the nodes of a render model
        count = random_.choice((0, 1, 3))
        model = bytearray(random_.randbytes(0x50 + 3 * 0x60))
        struct.pack_into("<iI", model, 0x48, count, 0x50)
        for node in range(3):
            struct.pack_into("<13f", model, 0x50 + node * 0x60 + 0x28, *(random_.uniform(-2, 2) for _ in range(13)))
        models = bytearray(random_.randbytes(0x10)) + model
        struct.pack_into("<I", models, 8, 0x10)
        t.check(
            "unknown_001376c0",
            random_.getrandbits(16) << 16,
            Buffer(random_.randbytes(3 * 0x30)),
            Buffer(struct.pack("<39f", *(random_.uniform(-2, 2) for _ in range(39)))),
            globals={"global_tag_instances": Buffer(bytes(models), pointers=(8, 0x10 + 0x4C))},
        )
