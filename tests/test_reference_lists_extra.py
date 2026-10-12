# what is kept with a thing in the lists of its clusters is changed

import random
import struct

from difftest import Buffer

from test_data import array
from test_object_types_more import after

NONE = 0xFFFFFFFF
REFERENCES = 48


def lists(random_, thing: int, clusters: list, size4: int) -> tuple:
    """The lists of some clusters, in the first table (in most of them the thing is); and where the thing is, in the second."""
    firsts = bytearray(b"\xff" * 0x800)
    free4 = list(range(REFERENCES))
    random_.shuffle(free4)
    table4 = array(random_, REFERENCES, size4, {index: random_.randint(1, 0x7FFF) for index in range(REFERENCES)})
    data4 = bytearray(table4.data)
    for cluster in clusters + random_.sample(range(0x200), 3):
        things = [random_.getrandbits(32) for _ in range(random_.randint(0, 4))]
        if cluster in clusters and random_.random() < 0.85:
            things.insert(random_.randint(0, len(things)), thing)
        entries = [random_.getrandbits(16) << 16 | free4.pop() for _ in things if free4]
        for at, (entry, what) in enumerate(zip(entries, things)):
            struct.pack_into(
                "<2I", data4, 0x4C + (entry & 0xFFFF) * size4 + 4, what, entries[at + 1] if at + 1 < len(entries) else NONE
            )
        if entries:
            struct.pack_into("<I", firsts, cluster * 4, entries[0])

    table8 = array(random_, REFERENCES, 0xC, {index: random_.randint(1, 0x7FFF) for index in range(REFERENCES)})
    data8 = bytearray(table8.data)
    places = [random_.getrandbits(16) << 16 | index for index in random_.sample(range(REFERENCES), len(clusters))]
    for at, (place, cluster) in enumerate(zip(places, clusters)):
        struct.pack_into(
            "<hHI",
            data8,
            0x4C + (place & 0xFFFF) * 0xC + 4,
            cluster,
            random_.getrandbits(16),
            places[at + 1] if at + 1 < len(places) else NONE,
        )
    return (
        Buffer(bytes(firsts)),
        Buffer(bytes(data4), pointers=table4.pointers),
        Buffer(bytes(data8), pointers=table8.pointers),
        places[0] if places else NONE,
    )


def run(t) -> None:
    random_ = random.Random(678)

    for n in range(300):
        thing = random_.getrandbits(32)
        extra_size = random_.choice((0, 3, 4, 0x14))
        clusters = random_.sample(range(0x200), random_.randint(0, 6))
        heads, four, eight, first = lists(random_, thing, clusters, 0xC + extra_size)

        # the list of one cluster
        cluster = random_.choice(clusters + [random_.randrange(0x200)])
        t.check(
            "reference_list_set_extra",
            Buffer(heads.data[cluster * 4 : cluster * 4 + 4] + random_.randbytes(4)),
            thing,
            four,
            extra_size,
            Buffer(random_.randbytes(extra_size + 8)),
        )

        heads, four, eight, first = lists(random_, thing, clusters, 0xC + extra_size)
        extra = Buffer(random_.randbytes(extra_size + 8))
        partition = Buffer(bytes(12))
        partition = Buffer(
            struct.pack(
                "<3I",
                after(partition, extra),
                after(partition, extra, heads),
                after(partition, extra, heads, four),
            )
        )
        t.check(
            "cluster_partition_set_extra",
            first,
            partition,
            extra,
            thing,
            extra_size,
            extra=[heads, four, eight],
        )
