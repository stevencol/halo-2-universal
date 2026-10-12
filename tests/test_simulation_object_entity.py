# source/networking/simulation_gamestate_entities.c: the object of a thing of the simulation is done away with

import random
import struct

from difftest import Buffer, Mock, mock_address


def run(t) -> None:
    random_ = random.Random(597)

    for n in range(2000):
        # a kind of thing: where its functions are, and the functions; the thirty-first is the one that the test stands in for
        kind = bytearray(random_.randbytes(4 + 31 * 4))
        struct.pack_into("<I", kind, 0, 4)
        struct.pack_into("<I", kind, 4 + 30 * 4, mock_address(0))
        entity = bytearray(random_.randbytes(0x20))
        struct.pack_into("<I", entity, 8, random_.choice((0xFFFFFFFF, 0x12340005, 0x12340005)))
        t.check(
            "simulation_object_entity_delete_object",
            Buffer(bytes(kind), pointers=(0,)),
            Buffer(bytes(entity)),
            mocks=[Mock(lambda call, value=random_.choice((0, 0, 1, 0x100)): value, stack_arguments=1, registers=("ecx",))],
            externals={
                "object_detach_simulation_entity": lambda harness, arguments: (0, (arguments[0],)),
                "unknown_000b8540": lambda harness, arguments: (0, (arguments[0],)),
            },
        )
