# source/networking/simulation.c: the changes that come with a tick are done

import random
import struct

from difftest import SCRATCH_ADDRESS, Buffer, Mock, mock_address

UPDATE = 0x4048
KINDS = (
    "simulation_players_update_remove_player",
    "simulation_players_update_unknown_00084fb0",
    "simulation_players_update_unknown_000850c0",
    "unknown_00085140",
    "simulation_players_update_set_properties",
)


def run(t) -> None:
    random_ = random.Random(587)

    for n in range(1500):
        update = bytearray(random_.randbytes(UPDATE))
        struct.pack_into("<I", update, 0x5CC, random_.choice((0, 1, 0x8001, 0xFFFF, random_.getrandbits(16), random_.getrandbits(32))))
        update[0xDD0] = random_.choice((0, 1, 0x80))
        count = random_.choice((0, 1, 2, 5, 12))
        struct.pack_into("<i", update, 0xE38, count)
        for index in range(count):
            struct.pack_into("<i", update, 0xE3C + index * 0xC8 + 0x10, random_.choice((0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 5, -1)))
        update[0x403C] = random_.choice((0, 1, 1, 0x80))
        world = bytearray(random_.randbytes(0x40))
        struct.pack_into("<i", world, 8, random_.choice((1, 2, 3, 4, 5)))

        def an_object(harness, arguments, there=random_.choice((0, 0x1234, 0x1234))):
            return there if arguments[0] & 1 else 0x1234, (arguments[0], arguments[1])

        def kind(harness, arguments, results=tuple(random_.choice((1, 1, 1, 1, 1, 1, 0, 0x100)) for _ in range(16))):
            # each of the things in its turn: whether it could be done is by its place
            place = (arguments[0] - SCRATCH_ADDRESS - 0xE3C) // 0xC8 % 16
            return results[place], (harness.normalize(arguments[0]),)

        t.check(
            "simulation_apply_update_changes",
            Buffer(bytes(update)),
            globals={
                "unknown_004cf770": random_.randbytes(12) + bytes(4),
                "unknown_004cf770+0xc": Buffer(bytes(world)),
                "game_state_before_save_procs": struct.pack("<4I", *(mock_address(index) for index in range(4))),
                "game_state_after_save_procs": struct.pack("<I", mock_address(4)),
            },
            mocks=[Mock(lambda call: 0, stack_arguments=1) for index in range(5)],
            externals={
                "object_try_and_get_and_verify_type": an_object,
                "unknown_000c6de0": lambda harness, arguments: (0, (arguments[0], harness.normalize(arguments[1]))),
                "unknown_0014c630": lambda harness, arguments: (0, (arguments[0], bytes(harness.mu.mem_read(arguments[1], 0x60)))),
                **{name: kind for name in KINDS},
            },
        )
