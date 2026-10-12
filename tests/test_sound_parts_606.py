# three parts of the game in the sound: sound_classes.c and two blocks of sound_manager.c

import random

from difftest import PHYSICAL_ADDRESS, Buffer, Memory

from test_game_state import game_state_globals

BASE = PHYSICAL_ADDRESS + 0x60000


def run(t) -> None:
    random_ = random.Random(606)

    for n in range(100):
        for name, pointer, size in (
            ("sound_classes_initialize", "sound_class_gains", 0x360),
            ("sound_unknown_002229d0", "unknown_00502120", 0x22C),
            ("sound_unknown_00225a40", "unknown_0051ebf4", 0x28),
        ):
            allocated = random_.choice((0, 0x40, 0x1288))
            t.check(
                name,
                globals={
                    "game_state_globals": game_state_globals(random_, base=BASE, cpu_allocation_size=allocated),
                    pointer: random_.getrandbits(32),
                },
                memory={BASE + allocated: Memory(random_.randbytes(size + 0x10))},
            )
        flag = {"unknown_0050211c": bytes((random_.choice((0, 1, 0x80)),))}
        t.check("sound_classes_dispose", globals={"sound_class_gains": random_.getrandbits(32)})
        t.check("sound_classes_initialize_for_new_map", globals={"sound_class_gains": Buffer(random_.randbytes(0x370)), **flag})
        t.check("sound_classes_dispose_from_old_map", globals=flag)
        t.check("sound_unknown_00222a10", globals={"unknown_00502120": Buffer(random_.randbytes(0x23C))})
        t.check("sound_unknown_00222a60", externals={"unknown_00222d80": lambda harness, arguments: (0, ())})
        t.check("sound_unknown_00225a80", globals={"unknown_0051ebf4": Buffer(random_.randbytes(0x38))})
