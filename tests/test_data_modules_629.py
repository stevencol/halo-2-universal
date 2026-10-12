# four modules with the same three functions: liquid, antenna, cloth, recorded animations

import random

from difftest import Buffer

from test_effects_initialize import tables

# prefix, global, how much memory the table asks for
MODULES = (
    ("liquid", "liquid_data", 0x4454),
    ("antenna", "antenna_data", 0x2120),
    ("cloth", "cloth_data", 0x9310),
    ("recorded_animations", "recorded_animation_data", 0xF0),
)


def run(t) -> None:
    random_ = random.Random(629)

    def empties(harness, arguments):
        return 0, (harness.normalize(arguments[0]), harness.mu.mem_read(arguments[0] + 0x29, 1)[0])

    for n in range(60):
        for prefix, global_, size in MODULES:
            allocation, buffers, mocks = tables(random_, (size,))
            t.check(
                f"{prefix}_initialize",
                globals={"game_state_allocation": allocation, global_: random_.getrandbits(32)},
                extra=buffers,
                mocks=mocks,
            )
            t.check(
                f"{prefix}_initialize_for_new_map",
                globals={global_: Buffer(random_.randbytes(0x4C))},
                externals={"data_delete_all": empties},
            )
            t.check(f"{prefix}_dispose_from_old_map", globals={global_: Buffer(random_.randbytes(0x4C))})
