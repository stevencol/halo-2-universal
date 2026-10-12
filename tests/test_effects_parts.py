# source/effects: the effects, the contrails and the decals start and stop

import random

from difftest import Buffer

EFFECTS = ("effect_data", "effect_location_data", "unknown_00510c74", "unknown_0051ec84", "unknown_0051ec88", "unknown_0051ec8c")
CONTRAILS = ("contrail_data", "contrail_point_data")


def run(t) -> None:
    random_ = random.Random(611)

    def empties(harness, arguments):
        # the table is emptied after it is said that it can be used
        return 0, (harness.normalize(arguments[0]), harness.mu.mem_read(arguments[0] + 0x29, 1)[0])

    for n in range(150):
        for names, prefix in ((EFFECTS, "effects"), (CONTRAILS, "contrails")):
            t.check(f"{prefix}_dispose", globals={name: random_.choice((0, 0x1234, random_.getrandbits(32))) for name in names})
            tables = lambda: {name: Buffer(random_.randbytes(0x4C)) for name in names}
            t.check(f"{prefix}_initialize_for_new_map", globals=tables(), externals={"data_delete_all": empties})
            t.check(f"{prefix}_dispose_from_old_map", globals=tables())
        t.check(
            "decals_initialize_for_new_map",
            globals={"decal_data": Buffer(random_.randbytes(0x4C)), "decal_globals": Buffer(random_.randbytes(0x3C3C))},
            externals={"data_delete_all": empties},
        )
