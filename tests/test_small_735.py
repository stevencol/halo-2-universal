# twenty small functions that call nothing

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
TAGS, TAG = 4, 0xA0
USER = 0x10
NODES, NODE = 5, 0x10


def run(t) -> None:
    random_ = random.Random(735)

    for n in range(300):
        # the tags, and the thing that takes one
        tags = bytearray(random_.randbytes(TAGS * 16 + TAGS * TAG))
        pointers = []
        for index in range(TAGS):
            struct.pack_into("<I", tags, index * 16 + 8, TAGS * 16 + index * TAG)
            pointers.append(index * 16 + 8)
        t.check(
            "tag_user_set_tag",
            Buffer(random_.randbytes(USER)),
            random_.getrandbits(16) << 16 | random_.randrange(TAGS),
            globals={"global_tag_instances": Buffer(bytes(tags), pointers=tuple(pointers))},
        )

        # the thing with its tag after it, and the things of the tag after both
        user = bytearray(random_.randbytes(USER + TAG + 4 * 0x38))
        struct.pack_into("<I", user, 8, USER)
        struct.pack_into("<I", user, USER + 0x88, USER + TAG)
        a_user = lambda: Buffer(bytes(user), pointers=(8, USER + 0x88))
        t.check("tag_user_get_unknown84", a_user())
        t.check("tag_user_get_unknown88", a_user(), random_.randrange(4))
        t.check("tag_user_get_unknown90", a_user())
        t.check("tag_user_get_flag7", a_user())
        t.check("tag_user_get_flag8", a_user())

        ring = bytearray(random_.randbytes(8 + 0x100 * 2))
        struct.pack_into("<I", ring, 4, random_.choice((0, 1, 0xFE, 0xFF, random_.randrange(0x100))))
        t.check("unknown_ring_add_short", Buffer(bytes(ring)), random_.getrandbits(32))

        unknown = lambda: Buffer(random_.randbytes(0xB0))
        t.check("unknown_get_unknown16_bit2", unknown())
        t.check("unknown_get_unknownC_bit10", unknown())
        t.check("unknown_get_unknown9C", unknown())
        t.check("unknown_get_unknownA8", unknown())
        t.check("unknown_get_unknown16_bit7", unknown())
        t.check("unknown_get_unknownC_bit9", unknown())

        # some things, each of the one after it or of none
        nodes = bytearray(random_.randbytes(NODES * NODE))
        pointers = []
        for index in range(NODES):
            if index + 1 < NODES and random_.random() < 0.7:
                struct.pack_into("<I", nodes, index * NODE + 0xC, (index + 1) * NODE)
                pointers.append(index * NODE + 0xC)
            else:
                struct.pack_into("<I", nodes, index * NODE + 0xC, 0)
        some_nodes = lambda: Buffer(bytes(nodes), pointers=tuple(pointers))
        t.check("unknown_node_get_parent", some_nodes())
        t.check("unknown_node_get_root", some_nodes())

        flag = lambda: Buffer(random_.choice((bytes(4), b"\0" + random_.randbytes(3), random_.randbytes(4))))
        t.check("unknown_flag_get", flag())
        t.check("unknown_flag_set", flag(), random_.getrandbits(24) << 8 | random_.choice((0, 1, 0xFF)))

        thing = bytearray(random_.randbytes(0x78))
        struct.pack_into("<I", thing, 0, random_.choice((NONE, 0, random_.getrandbits(32))))
        struct.pack_into("<H", thing, 6, random_.choice((0xFFFF, 0, random_.getrandbits(16))))
        struct.pack_into("<I", thing, 0x68, random_.choice((NONE, 0, random_.getrandbits(32))))
        thing[0x18] = random_.choice((0, 1, 5, 0x7F, 0x80, 0xFF))
        struct.pack_into("<i", thing, 0x74, random_.choice((0, 1, 2, 5, 6, 0x7F, 0x80, 0x1000, -1)))
        a_thing = lambda: Buffer(bytes(thing))
        t.check("unknown_has_unknown0_and_unknown6", a_thing())
        t.check("unknown_has_unknown0_unknown6_and_unknown68", a_thing())
        t.check("unknown_get_unknown18_below_unknown74", a_thing())
