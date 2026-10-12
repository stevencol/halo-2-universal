# source/networking/simulation_world.c: the thing that the simulation has going changes its part

import random
import struct

from difftest import Buffer

WORLD = 0x1220


def run(t) -> None:
    random_ = random.Random(584)

    def note(harness, arguments):
        game = struct.unpack("<I", harness.mu.mem_read(harness.global_addresses["main_game_globals"], 4))[0]
        there = bytes(harness.mu.mem_read(harness.global_addresses["unknown_004cf770"], 12))
        flags = bytes(harness.mu.mem_read(harness.global_addresses["unknown_00510ca0"], 1))
        return 0, (tuple(harness.normalize(argument) for argument in arguments), harness.mu.mem_read(game + 0xC, 1)[0], there, flags)

    def scrambles(harness, arguments):
        # what ends the thing and what sets it up again leave it as another: what is kept has to be put back
        world = arguments[-1]
        record = note(harness, arguments)
        harness.mu.mem_write(world + 0xC, bytes(0x0C))
        harness.mu.mem_write(world + 0x2F, b"\x00")
        return record

    for n in range(1500):
        world = bytearray(random_.randbytes(WORLD))
        struct.pack_into("<i", world, 8, random_.choice((2, 3, 3, 4, 5)))
        struct.pack_into("<i", world, 0x1200, random_.choice((-1, -1, -1, 7)))
        world[0x2F] = random_.choice((0, 1, 0x80))
        voice = bytearray(0xC18)
        struct.pack_into("<i", voice, 0xC14, random_.randrange(16))
        globals_ = {
            "main_game_globals": Buffer(random_.randbytes(0x20)),
            "unknown_004cf770": random_.randbytes(16),
            "unknown_004cf784": random_.getrandbits(32),
            "unknown_00510ca0": bytes((random_.choice((0, 1, 0x80)),)),
            "unknown_00510ca1": bytes((random_.choice((0, 1, 0x80)),)),
            "unknown_004e9ae8": Buffer(bytes(voice)),
            "unknown_0055e4d0": b"".join(struct.pack("<I", random_.choice((0, 1, 0x100))) for _ in range(16)),
        }
        externals = {
            "simulation_world_dispose": scrambles,
            "simulation_world_initialize": scrambles,
            **{
                name: note
                for name in (
                    "simulation_world_unknown_0006a770",
                    "simulation_world_unknown_0006b040",
                    "replication_entity_manager_adopt_entities",
                    "unknown_000d5560",
                    "unknown_000d5640",
                    "unknown_00185a30",
                    "unknown_00185630",
                    "unknown_00162060",
                    "unknown_00196470",
                    "unknown_00196780",
                    "simulation_world_unknown_0006b1f0",
                    "simulation_world_unknown_0006b2a0",
                )
            },
        }
        for name in ("simulation_world_become_client", "simulation_world_become_authority"):
            t.check(name, Buffer(bytes(world)), globals=globals_, externals=externals)
