# the threads and the nodes of the scripts that do not stay are deleted

import random

from difftest import Buffer

from test_data import array

THREADS, NODES = 10, 60


def run(t) -> None:
    random_ = random.Random(703)

    for n in range(200):
        threads = array(random_, THREADS, 0x418, {index: random_.randint(-0x8000, 0x7FFF) or 1 for index in random_.sample(range(THREADS), random_.choice((0, 1, 5, THREADS)))})
        data = bytearray(threads.data)
        for index in range(THREADS):
            data[0x4C + index * 0x418 + 2] = random_.choice((0, 1, 2, 2, 4))
        nodes = array(random_, NODES, 0x14, {index: random_.randint(-0x8000, 0x7FFF) or 1 for index in random_.sample(range(NODES), random_.choice((0, 3, 30, NODES)))})
        node_bytes = bytearray(nodes.data)
        for index in range(NODES):
            node_bytes[0x4C + index * 0x14 + 6] = random_.choice((0, 8, 8, 0xF7, 0xFF))
        t.check(
            "hs_delete_temporaries",
            globals={
                "unknown_004f9388": random_.choice((b"\1", b"\1", b"\0", b"\x80")),
                "hs_thread_data": Buffer(bytes(data), pointers=threads.pointers),
                "script_node_data": Buffer(bytes(node_bytes), pointers=nodes.pointers),
            },
        )
