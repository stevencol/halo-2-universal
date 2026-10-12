# the lights of an object are taken out of the clusters, or updated

import random
import struct

from difftest import Buffer

from test_lights_walks import lights

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(695)

    for n in range(300):
        some = lights(random_)
        data = bytearray(some.data)
        maximum_count = struct.unpack_from("<i", data, 0x20)[0]
        objects = [random_.getrandbits(32) for _ in range(2)]
        for index in range(maximum_count):
            struct.pack_into("<I", data, 0x4C + index * 0x110 + 0x58, random_.choice([NONE] + objects + objects))
        t.check(
            "lights_unknown_000c1720",
            random_.choice(objects),
            random_.choice((0, 1, 1, 0x100)),
            random_.choice((0, 1, 1, 0x100)),
            globals={
                "light_data": Buffer(bytes(data), pointers=some.pointers),
                "light_cluster_partition": random_.randbytes(12),
            },
            externals={
                "cluster_partition_remove": lambda harness, arguments: (
                    0,
                    (arguments[0] == harness.global_addresses["light_cluster_partition"], *arguments[1:]),
                ),
                "unknown_000c2d00": lambda harness, arguments: 0,
            },
        )
