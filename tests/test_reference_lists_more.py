# three more of the lists of references

import random
import struct

from difftest import Buffer

from test_reference_lists_extra import lists


def run(t) -> None:
    random_ = random.Random(679)

    for n in range(300):
        thing = random_.getrandbits(32)
        size = random_.choice((0xC, 0x10, 0x20))
        clusters = random_.sample(range(0x200), random_.randint(1, 6))

        def one() -> tuple:
            """The first of the list of a cluster, and the table."""
            heads, four, eight, first = lists(random_, thing, clusters, size)
            cluster = random_.choice(clusters + [random_.randrange(0x200)])
            return heads.data[cluster * 4 : cluster * 4 + 4], four

        first, table = one()
        t.check("reference_list_remove", Buffer(first + random_.randbytes(4)), thing, table)
        first, table = one()
        t.check("reference_list_contains", Buffer(first + random_.randbytes(4)), table, thing)
        first, table = one()
        t.check("reference_list_delete_all", struct.unpack("<I", first)[0], table)
