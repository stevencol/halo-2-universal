# seven accessors: five of what an object keeps, two of the structure bsp

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
COUNT, OBJECT = 4, 0x400


def objects(random_) -> Buffer:
    """The table of the objects: its entries, of 12 bytes, and the objects after them."""
    headers = bytearray(random_.randbytes(0x4C + COUNT * 12))
    struct.pack_into("<I", headers, 0x44, 0x4C)
    pointers = [0x44]
    data = bytearray()
    for index in range(COUNT):
        struct.pack_into("<I", headers, 0x4C + index * 12 + 8, len(headers) + len(data))
        pointers.append(0x4C + index * 12 + 8)
        data += random_.randbytes(OBJECT)
    return Buffer(bytes(headers) + bytes(data), pointers=tuple(pointers))


def run(t) -> None:
    random_ = random.Random(709)

    for n in range(200):
        index = random_.getrandbits(16) << 16 | random_.randrange(COUNT)
        for name in ("unit_get_unknown240", "unit_get_unknown134_bit0", "unit_get_unknown238", "unit_get_unknown23C", "unit_get_unknown3DC"):
            t.check(name, index, globals={"object_header_data": objects(random_)})

        leaves = random_.randint(1, 12)
        bsp = bytearray(random_.randbytes(0x100 + leaves * 8 + 0x20))
        struct.pack_into("<I", bsp, 0x30, 0x100)
        struct.pack_into("<iI", bsp, 0xC4, random_.choice((0, -1, 1, 7)), 0x100 + leaves * 8)
        structure_bsp = {"global_structure_bsp": Buffer(bytes(bsp), pointers=(0x30, 0xC8))}
        t.check("structure_bsp_leaf_get_cluster_index", random_.choice((NONE, random_.randrange(leaves))), globals=structure_bsp)
        t.check("structure_bsp_get_unknownC4", globals=structure_bsp)
