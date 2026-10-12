# what tells which things are in each cluster is made

import random

from difftest import PHYSICAL_ADDRESS, Buffer, Memory

from test_game_state import game_state_globals

BASE = PHYSICAL_ADDRESS + 0x60000


def run(t) -> None:
    random_ = random.Random(675)

    for n in range(100):
        made = [random_.choice((0, random_.getrandbits(32))) for _ in range(2)]

        def table(harness, arguments, made=made):
            # the name is on the stack of the function: what is compared is its text
            name = bytes(harness.mu.mem_read(arguments[0], 0x100)).split(b"\0")[0]
            calls = sum(1 for call in harness._calls if call[0] == "reference_list_new")
            return made[min(calls, 1)], (name, arguments[1], arguments[2])

        allocated = random_.choice((0, 0x40, 0x1288))
        name = random_.choice((b"light", b"", b"object", b"a longer name than most", bytes(random_.randint(0x41, 0x7A) for _ in range(40))))
        t.check(
            "cluster_partition_initialize",
            Buffer(random_.randbytes(0xC + 0x10)),
            Buffer(name + bytes(1) + random_.randbytes(8)),
            random_.choice((0, 4, 0x14, random_.getrandbits(32))),
            globals={"game_state_globals": game_state_globals(random_, base=BASE, cpu_allocation_size=allocated)},
            memory={BASE + allocated: Memory(random_.randbytes(0x800 + 0x10))},
            externals={"reference_list_new": table},
        )
