# source/networking/replication_event_view.c

import random
import struct

from difftest import Buffer, Mock, Pointer, mock_address

NONE = 0xFFFFFFFF
EVENT = 0x2C
# in one buffer: the heap of the network and what gives its memory, with its functions; what the things that happen
# are of, with its functions; what keeps them; six of them; the view; three packets; and six of what it kept of them
HEAP, ALLOCATOR, TABLE, CLIENT, CLIENT_TABLE, MANAGER, EVENTS, VIEW, PACKETS, ITEMS, END = (
    0x0,
    0x8,
    0xC,
    0x38,
    0x3C,
    0x4C,
    0x9C,
    0x1A4,
    0x1CC,
    0x1F0,
    0x220,
)
EVENT_COUNT = 6


def run(t) -> None:
    random_ = random.Random(431)

    def world() -> Buffer:
        data = bytearray(random_.randbytes(END))
        pointers = [HEAP, ALLOCATOR, CLIENT, MANAGER + 4, VIEW + 0x18]
        struct.pack_into("<Ii", data, HEAP, ALLOCATOR, random_.choice((0, 7, 100)))
        struct.pack_into("<I", data, ALLOCATOR, TABLE)
        struct.pack_into("<11I", data, TABLE, mock_address(0), mock_address(1), *(0x11 * (n + 3) for n in range(9)))
        struct.pack_into("<I", data, CLIENT, CLIENT_TABLE)
        struct.pack_into("<4I", data, CLIENT_TABLE, 0x11, 0x22, 0x33, mock_address(2))
        struct.pack_into("<I", data, MANAGER + 4, CLIENT)
        index = random_.choice((0, 3, 14, 31, 33))
        bit = 1 << (index & 31)
        struct.pack_into("<I", data, VIEW + 0xC, index)
        struct.pack_into("<I", data, VIEW + 0x18, MANAGER)
        # the things that happen: in the list, some; each still to be told by this view only, by others too, or not by it
        listed = random_.choice((0, 1, 3, EVENT_COUNT))
        events = [EVENTS + event * EVENT for event in range(EVENT_COUNT)]
        struct.pack_into("<iI", data, MANAGER + 0x48, random_.choice((listed, listed, 9)), events[0] if listed else 0)
        if listed:
            pointers.append(MANAGER + 0x4C)
        for place, event in enumerate(events):
            mask = random_.choice((bit, bit, bit | 0x40, bit | random_.getrandbits(32), random_.getrandbits(32) & ~bit, 0))
            struct.pack_into("<I", data, event + 0x18, random_.choice((0, 0, 0x12345678)))
            struct.pack_into("<I", data, event + 0x20, mask)
            last = place >= listed - 1
            struct.pack_into("<I", data, event + 0x28, 0 if last else events[place + 1])
            if not last:
                pointers.append(event + 0x28)
        # the packets that the view waits to hear of, and the things that it said in each
        waiting = random_.choice((0, 1, 2, 3))
        struct.pack_into("<I", data, VIEW + 0x10, PACKETS if waiting else 0)
        if waiting:
            pointers.append(VIEW + 0x10)
        free = [ITEMS + item * 8 for item in range(6)]
        for packet in range(waiting):
            mine = [free.pop() for item in range(random_.choice((0, 1, 2)))]
            struct.pack_into("<3I", data, PACKETS + packet * 0xC, random_.getrandbits(32), mine[0] if mine else 0, PACKETS + (packet + 1) * 0xC if packet + 1 < waiting else 0)
            if mine:
                pointers.append(PACKETS + packet * 0xC + 4)
            if packet + 1 < waiting:
                pointers.append(PACKETS + packet * 0xC + 8)
            for place, item in enumerate(mine):
                after = mine[place + 1] if place + 1 < len(mine) else 0
                struct.pack_into("<II", data, item, random_.choice(events), after)
                pointers.append(item)
                if after:
                    pointers.append(item + 4)
        return Buffer(bytes(data), pointers=tuple(pointers))

    for n in range(600):
        data = world()
        t.check(
            "replication_event_view_reset",
            Pointer(data, VIEW),
            extra=[data],
            globals={"unknown_004d87f8": Pointer(data, HEAP)},
            mocks=[
                Mock(lambda call: 0, stack_arguments=2, registers=("ecx",)),
                # where the size goes is in the stack
                Mock(lambda call, known=random_.choice((0, 1)): known, stack_arguments=2, registers=("ecx",), record=lambda call: (call.registers["ecx"], call.stack[0])),
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
            ],
        )
