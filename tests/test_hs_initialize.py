# source/hs/hs.c: the scripts start, and are set up for a map

import random
import struct

from difftest import Buffer, Mock

from test_effects_initialize import an_allocation
from test_object_types_more import after


def run(t) -> None:
    random_ = random.Random(621)

    def note(harness, arguments):
        return 0, tuple(harness.normalize(argument) for argument in arguments)

    def table(harness, arguments):
        return 0, (*(harness.normalize(argument) for argument in arguments), harness.mu.mem_read(arguments[0] + 0x29, 1)[0])

    def a_map():
        scenario = bytearray(random_.randbytes(0x240))
        struct.pack_into("<i", scenario, 0x1B8, random_.choice((0, 0, 1)))
        struct.pack_into("<i", scenario, 0x1D0, random_.choice((0, 1, 1, -1)))
        return {
            "global_scenario_index": random_.choice((0xFFFFFFFF, 0x12340000, 0x12340000)),
            "global_scenario": random_.choice((0, 1, 1, 1)) and Buffer(bytes(scenario)),
            "object_list_header_data": Buffer(random_.randbytes(0x4C)),
            "list_object_data": Buffer(random_.randbytes(0x4C)),
        }

    others = {
        "data_connect": note,
        "unknown_002a09c0": note,
        "data_delete_all": table,
        "unknown_002090a0": note,
    }

    for n in range(300):
        nodes = bytearray(random_.randbytes(0x4C))
        struct.pack_into("<H", nodes, 0x2A, random_.choice((0, 2, 2, 6, 0xFFFD)))
        t.check(
            "hs_initialize_for_new_map",
            globals={**a_map(), "script_node_data": Buffer(bytes(nodes))},
            externals=others,
        )

    for n in range(40):
        # the table is made first: what is done for a map comes after, and needs it
        # with no memory the game goes on to look at a table that is not there: the test always gives it
        allocation = an_allocation()
        globals_ = {"heap_allocation": allocation, "script_node_data": random_.getrandbits(32), **a_map()}
        memory = after(*(value for value in globals_.values() if isinstance(value, Buffer)))
        t.check(
            "hs_initialize",
            globals=globals_,
            extra=[Buffer(random_.randbytes(0x124C))],
            mocks=[
                Mock(lambda call, memory=memory: memory, stack_arguments=1, registers=("ecx",)),
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
            ],
            externals={**others, "object_lists_initialize": note, "unknown_00208ff0": note},
        )
