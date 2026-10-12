# a light is deleted

import random
import struct

from difftest import Buffer

from test_lights_walks import lights
from test_units import COUNT, some_objects

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(694)

    for n in range(400):
        some = lights(random_)
        data = bytearray(some.data)
        count = (len(data) - 0x4C) // 0x110
        maximum_count = struct.unpack_from("<i", data, 0x20)[0]
        objects = [random_.getrandbits(16) << 16 | random_.randrange(COUNT) for _ in range(3)]
        for index in range(min(count, maximum_count)):
            struct.pack_into("<I", data, 0x4C + index * 0x110 + 0x58, random_.choice([NONE, NONE] + objects))
        # the light: one that is in use, when there is one
        in_use = [index for index in range(maximum_count) if data[0x4C + maximum_count * 0x110 + (index >> 3)] & (1 << (index & 7))]
        absolute = random_.choice(in_use) if in_use and random_.random() < 0.9 else random_.randrange(maximum_count)
        identifier = struct.unpack_from("<H", data, 0x4C + absolute * 0x110)[0]
        t.check(
            "light_delete",
            identifier << 16 | absolute,
            random_.choice((0, 1, 1, 0x100)),
            globals={
                "light_data": Buffer(bytes(data), pointers=some.pointers),
                "object_header_data": some_objects(random_),
                "light_cluster_partition": random_.randbytes(12),
            },
            externals={
                "cluster_partition_remove": lambda harness, arguments: (
                    0,
                    (arguments[0] == harness.global_addresses["light_cluster_partition"], *arguments[1:]),
                )
            },
        )
