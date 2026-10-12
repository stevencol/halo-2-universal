# a table of references is made; a thing is taken out of its clusters

import random
import struct

from difftest import SCRATCH_ADDRESS, Buffer, Mock

from test_data import array
from test_effects_initialize import an_allocation
from test_object_types_more import after

NONE = 0xFFFFFFFF
REFERENCES = 48


def run(t) -> None:
    random_ = random.Random(676)

    for n in range(100):
        # whatever size is asked for, the memory is the same buffer, or there is none
        allocation = an_allocation()
        memory = Buffer(random_.randbytes(0x2000))
        name = Buffer(random_.choice((b"list object", b"cluster light", b"", bytes(random_.randint(0x41, 0x7A) for _ in range(40)))) + bytes(9))
        given = random_.choice((True, True, True, False))
        t.check(
            "reference_list_new",
            name,
            random_.choice((1, 5, 0x40)),
            random_.choice((0, 0, 4, 0x14, 0x14, 8)),
            globals={"game_state_allocation": allocation},
            extra=[memory],
            mocks=[
                Mock(lambda call, memory=memory, given=given: memory.address if given else 0, stack_arguments=1, registers=("ecx",)),
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
            ],
        )

    for n in range(300):
        thing = random_.getrandbits(32)
        clusters = random_.sample(range(0x200), random_.randint(0, 6))

        # the lists of some clusters, in the first table; in some of them the thing is
        size4 = random_.choice((0xC, 0x10))
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
                    "<2I",
                    data4,
                    0x4C + (entry & 0xFFFF) * size4 + 4,
                    what,
                    entries[at + 1] if at + 1 < len(entries) else NONE,
                )
            if entries:
                struct.pack_into("<I", firsts, cluster * 4, entries[0])

        # where the thing is, in the second table
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

        first = Buffer(struct.pack("<I", places[0] if places else NONE) + random_.randbytes(12))
        heads = Buffer(bytes(firsts))
        four = Buffer(bytes(data4), pointers=table4.pointers)
        eight = Buffer(bytes(data8), pointers=table8.pointers)
        partition = Buffer(bytes(12))
        partition = Buffer(
            struct.pack(
                "<3I",
                after(partition, first),
                after(partition, first, heads),
                after(partition, first, heads, four),
            )
        )
        t.check("cluster_partition_remove", partition, thing, first, extra=[heads, four, eight])
