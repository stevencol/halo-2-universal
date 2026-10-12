# a thing is put in its clusters

import random
import struct

from difftest import Buffer

from test_data import array
from test_object_types_more import after

NONE = 0xFFFFFFFF
REFERENCES = 24
CLUSTERS = 0x200


def bits_of(clusters: list) -> bytes:
    bits = bytearray(CLUSTERS // 8)
    for cluster in clusters:
        bits[cluster >> 3] |= 1 << (cluster & 7)
    return bytes(bits)


def run(t) -> None:
    random_ = random.Random(677)

    for n in range(300):
        count = random_.choice((0, 1, 40, CLUSTERS, random_.randrange(CLUSTERS)))
        bsp = bytearray(random_.randbytes(0x100))
        struct.pack_into("<i", bsp, 0x9C, count)
        maximum = random_.choice((0, 1, 3, 0x100))
        t.check(
            "structure_bsp_clusters_from_bit_vector",
            Buffer(random_.choice((random_.randbytes(CLUSTERS // 8), bits_of(random_.sample(range(CLUSTERS), 5))))),
            Buffer(random_.randbytes(2 + 6)),
            maximum,
            Buffer(random_.randbytes(0x200 + 8)),
            globals={"global_structure_bsp": Buffer(bytes(bsp))},
        )

    for n in range(400):
        how = random_.choice(("bits", "bits", "sphere", "sphere", "one", "none"))
        some = random_.sample(range(CLUSTERS), random_.choice((0, 1, 2, 5, 9)))
        cluster_bits = Buffer(bits_of(some)) if how == "bits" else 0
        bsp = bytearray(random_.randbytes(0x100))
        struct.pack_into("<i", bsp, 0x9C, random_.choice((CLUSTERS, CLUSTERS, 100)))
        radius = {"sphere": random_.choice((0.5, 3.0, 1e9)), "one": random_.choice((0.0, -1.0, float("nan")))}.get(how, 1.0)
        location = bytearray(random_.randbytes(8))
        struct.pack_into("<h", location, 4, -1 if how == "none" else random_.randrange(CLUSTERS))
        # more clusters than there is room for, now and then
        found = random_.choice((0, 1, len(some), len(some), 0x100, 0x101, 0x140))
        touched = (some + [random_.randrange(CLUSTERS) for _ in range(0x100)])[:0x100]

        def around(harness, arguments, found=found, touched=touched):
            written = min(found, arguments[3] & 0xFFFF, 0x100)
            harness.mu.mem_write(arguments[4], struct.pack(f"<{written}h", *touched[:written]))
            # the list is on the stack of the function: what is compared is the rest
            return found, (arguments[0] & 0xFFFF, harness.normalize(arguments[1]), arguments[2], arguments[3])

        extra_size = random_.choice((0, 4, 0x14))
        extra = Buffer(random_.randbytes(extra_size + 8)) if random_.random() < 0.6 else 0
        size4 = 0xC + extra_size
        heads = Buffer(b"".join(struct.pack("<I", random_.choice((NONE, NONE, random_.getrandbits(32)))) for _ in range(CLUSTERS)))
        tables = []
        for size in (size4, 0xC):
            in_use = {index: random_.randint(1, 0x7FFF) for index in random_.sample(range(REFERENCES), random_.choice((0, 5, REFERENCES - 2, REFERENCES)))}
            tables.append(array(random_, REFERENCES, size, in_use))
        four, eight = tables

        first = Buffer(struct.pack("<I", random_.choice((NONE, random_.getrandbits(32)))) + random_.randbytes(12))
        center = Buffer(random_.randbytes(12 + 4))
        where = Buffer(bytes(location))
        overflow = Buffer(random_.randbytes(1 + 7))
        structure_bsp = Buffer(bytes(bsp))
        partition = Buffer(bytes(12))
        before = [buffer for buffer in (cluster_bits, partition, first, center, where, extra, overflow) if buffer] + [structure_bsp]
        partition = Buffer(
            struct.pack("<3I", after(*before), after(*before, heads), after(*before, heads, four))
        )
        t.check(
            "cluster_partition_add",
            cluster_bits,
            partition,
            random_.getrandbits(32),
            first,
            center,
            struct.unpack("<I", struct.pack("<f", radius))[0],
            where,
            extra_size,
            extra,
            overflow,
            globals={
                "global_structure_bsp": structure_bsp,
                "unknown_004e7414": random_.getrandbits(32),
                "unknown_004e7411": random_.randbytes(1),
            },
            extra=[heads, four, eight],
            externals={"unknown_0014a5b0": around},
        )
