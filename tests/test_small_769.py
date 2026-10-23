# six small functions of the lists of the interface: a list that goes both
# ways and the list that has it, a tree followed down its first children,
# and two small questions of a thing. The lists never go round

import random
import struct

from difftest import Buffer, Pointer

NONE = 0xFFFFFFFF
NODE = 0x20
LINK = 0xC


class Nodes:
    """Things in one buffer, with pointers between them."""

    def __init__(self, random_: random.Random, size: int) -> None:
        self.data = bytearray(random_.randbytes(size))
        self.pointers: list[int] = []

    def link(self, at: int, to: int | None) -> None:
        if to is None:
            struct.pack_into("<I", self.data, at, 0)
            if at in self.pointers:
                self.pointers.remove(at)
        else:
            struct.pack_into("<I", self.data, at, to)
            if at not in self.pointers:
                self.pointers.append(at)

    def buffer(self) -> Buffer:
        return Buffer(bytes(self.data), pointers=tuple(self.pointers))


def run(t) -> None:
    random_ = random.Random(769)

    for n in range(600):
        thing = bytearray(random_.randbytes(0x44))
        struct.pack_into("<I", thing, 4, random_.choice((0, 1, 4, 5, 6, NONE, 0x80000000, random_.getrandbits(32))))
        thing[0x42] = random_.choice((0, 0, 1, 2, 3, 4, 0xFC, 0xFE, 0xFF))
        t.check("user_interface_unknown4_valid", Buffer(bytes(thing)))
        t.check("user_interface_flags42_unknown_0022f0ff", Buffer(bytes(thing)))

        # a list at 0, and eight things after it; some of them in the list
        count = 8
        nodes = Nodes(random_, NODE * (count + 1))
        at = lambda index: NODE * (index + 1)
        listed = random_.sample(range(count), random_.randrange(count + 1))
        nodes.link(0x14, at(listed[0]) if listed else None)
        for place, index in enumerate(listed):
            nodes.link(at(index) + 0x18, at(listed[place + 1]) if place + 1 < len(listed) else None)
            nodes.link(at(index) + 0x1C, at(listed[place - 1]) if place > 0 else None)
        for index in range(count):
            if index not in listed:
                nodes.link(at(index) + 0x18, random_.choice((None, at(random_.randrange(count)))))
                nodes.link(at(index) + 0x1C, random_.choice((None, at(random_.randrange(count)))))
        buffer = nodes.buffer()
        chosen = random_.choice(listed) if listed and random_.random() < 0.8 else random_.randrange(count)
        t.check("user_interface_list_remove", Pointer(buffer, at(chosen)), Pointer(buffer, 0), extra=[buffer])

        # a tree: each first child is a later thing, or none
        tree = Nodes(random_, NODE * count)
        for index in range(count):
            later = random_.choice((None, None, *range(index + 1, count))) if index + 1 < count else None
            tree.link(index * NODE + 0x10, later * NODE if later is not None else None)
            struct.pack_into("<I", tree.data, index * NODE + 4, random_.choice((0, 0, 1, NONE)))
        buffer = tree.buffer()
        t.check("user_interface_tree_deepest_first_child", Pointer(buffer, NODE * random_.randrange(count)), extra=[buffer])

        # links: the head of a list at 0, and eight links after it
        links = Nodes(random_, 4 + LINK * count)
        place = lambda index: 4 + LINK * index
        listed = random_.sample(range(count), random_.randrange(count))
        links.link(0, place(listed[0]) if listed else None)
        for order, index in enumerate(listed):
            links.link(place(index), place(listed[order - 1]) if order > 0 else None)
            links.link(place(index) + 4, place(listed[order + 1]) if order + 1 < len(listed) else None)
            links.link(place(index) + 8, 0)
        others = [index for index in range(count) if index not in listed]
        buffer = links.buffer()
        t.check("user_interface_link_append", Pointer(buffer, 0), Pointer(buffer, place(random_.choice(others))), extra=[buffer])
        if listed:
            t.check("user_interface_link_remove", Pointer(buffer, place(random_.choice(listed))), Pointer(buffer, 0), extra=[buffer])
