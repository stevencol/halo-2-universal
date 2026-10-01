# source/physics/bsp3d.c

import math
import random
import struct

from difftest import Buffer

LEAF_FLAG = 0x800000
NO_CHILD = 0xFFFFFF


def bsp3d(random_: random.Random, node_count: int) -> Buffer:
    """A tree of nodes with any planes; the children of a node are later nodes, leaves or nothing."""
    nodes = bytearray()
    planes = bytearray()
    for index in range(node_count):
        children = []
        for _ in range(2):
            kind = random_.random()
            if kind < 0.5 and index + 1 < node_count:
                children.append(random_.randint(index + 1, node_count - 1))
            elif kind < 0.9:
                children.append(LEAF_FLAG | random_.randint(0, 0x7FFFFE))
            else:
                children.append(NO_CHILD)
        nodes += struct.pack("<h", index) + children[0].to_bytes(3, "little") + children[1].to_bytes(3, "little")

        normal = [random_.uniform(-1, 1) for _ in range(3)]
        length = math.sqrt(sum(x * x for x in normal)) or 1.0
        planes += struct.pack("<4f", *(x / length for x in normal), random_.uniform(-10, 10))

    # the bsp, then its nodes, then its planes
    header = struct.pack("<4I", node_count, 16, node_count, 16 + len(nodes))
    return Buffer(header + bytes(nodes) + bytes(planes), pointers=(4, 12))


def run(t) -> None:
    random_ = random.Random(6)

    for _ in range(60):
        node_count = random_.choice((1, 2, 5, 20, 100))
        bsp = bsp3d(random_, node_count)
        # each plane, as it is and facing the other way; the tree has a plane per node
        for plane_index in random_.sample(range(node_count), min(node_count, 6)):
            for flags in (0, 0x8000, 0x10000, 0x7FFF8000):
                t.check(
                    "bsp3d_get_plane_from_designator",
                    bsp,
                    plane_index | flags,
                    Buffer(struct.pack("<4f", 9, 9, 9, 9), format="<4f"),
                )
        for _ in range(25):
            point = Buffer(struct.pack("<3f", *(random_.uniform(-15, 15) for _ in range(3))))
            t.check("bsp3d_test_point", bsp, 0, point)
            # from any node, and with the bits over the index that the game ignores
            node_index = random_.randint(0, node_count - 1)
            t.check("bsp3d_test_point", bsp, node_index | random_.choice((0, 0x1000000)), point)
