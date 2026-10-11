# source/networking/simulation_world.c: the thing that the simulation has going is set up, made ready again and ended

import random
import struct

from difftest import Buffer, GlobalRange

WORLD = 0x1220
MEMORY = 0xA0AC + 0x18


def run(t) -> None:
    random_ = random.Random(577)

    def note(harness, arguments):
        return 0, tuple(harness.normalize(argument) for argument in arguments)

    for n in range(500):
        kept = random_.choice((0, 1, 0x80))
        now = random_.getrandbits(32)
        time = {
            "unknown_00510548": bytes((kept,)),
            "unknown_0051054c": now if kept else random_.getrandbits(32),
        }
        game = bytearray(random_.randbytes(0x20))
        game[0xC] = random_.choice((1, 2, 3, 4, 5))
        memory = Buffer(random_.randbytes(MEMORY))
        t.check(
            "simulation_world_initialize",
            random_.getrandbits(32),
            memory,
            random_.getrandbits(32),
            Buffer(random_.randbytes(WORLD)),
            globals={**time, "main_game_globals": Buffer(bytes(game))},
            externals={
                "GetTickCount": lambda harness, arguments, now=now: now,
                "replication_entity_manager_new": note,
                "simulation_entity_database_reset": note,
                "simulation_world_clear_updates": note,
                "simulation_world_set_unknown18": note,
            },
        )

        world = bytearray(random_.randbytes(WORLD))
        struct.pack_into("<i", world, 8, random_.choice((0, 1, 2, 3, 4, 5, 6)))
        struct.pack_into("<I", world, 4, WORLD)
        t.check(
            "simulation_world_restart",
            Buffer(bytes(world) + random_.randbytes(0x10), pointers=(4,)),
            externals={
                "replication_entity_manager_reset": note,
                "replication_event_manager_reset_views": note,
                "simulation_entity_database_reset": note,
                "simulation_world_others_clear": note,
                "simulation_world_clear_updates": note,
            },
        )

        # the thing, then its views, then its memory
        count = random_.choice((0, 1, 2, 5, 15))
        views = WORLD
        memory_at = views + count * 0x40
        data = bytearray(random_.randbytes(memory_at + MEMORY))
        struct.pack_into("<i", data, 8, random_.choice((0, 1, 2, 3, 4, 5, 6)))
        struct.pack_into("<I", data, 4, memory_at)
        struct.pack_into("<i", data, 0x3C, count)
        pointers = [4]
        places = []
        for index in range(15):
            struct.pack_into("<I", data, 0x40 + index * 4, 0)
        for index in range(count):
            at = views + index * 0x40
            struct.pack_into("<I", data, 0x40 + index * 4, at)
            pointers.append(0x40 + index * 4)
            struct.pack_into("<H", data, at + 2, random_.choice((1, 2, 0x101)))
            struct.pack_into("<i", data, at + 0x24, random_.choice((-1, 0, 3)))
            # the thing that the view is of: the stand-in takes the view away from it
            struct.pack_into("<I", data, at + 0xC, 0)
            pointers.append(at + 0xC)
            # what it has at 0x8, or nothing: in the memory, past what the test looks at
            if random_.choice((False, True)):
                struct.pack_into("<I", data, at + 8, 0)
            else:
                struct.pack_into("<I", data, at + 8, at + 0x30)
                pointers.append(at + 8)
                places += [at + 0x30 + offset for offset in (0x4, 0x30, 0x5070, 0x5098)]
        struct.pack_into("<I", data, memory_at + 0xA0AC + 0xC, memory_at + 0x2048)
        struct.pack_into("<I", data, memory_at + 0x2098 + 0xC, memory_at)
        pointers += [memory_at + 0xA0AC + 0xC, memory_at + 0x2098 + 0xC]

        def detaches(harness, arguments):
            world_at = struct.unpack("<I", harness.mu.mem_read(arguments[0] + 0xC, 4))[0]
            left = struct.unpack("<i", harness.mu.mem_read(world_at + 0x3C, 4))[0]
            harness.mu.mem_write(world_at + 0x3C, struct.pack("<i", left - 1))
            return 0, (harness.normalize(arguments[0]), left)

        t.check(
            "simulation_world_dispose",
            Buffer(bytes(data), pointers=tuple(pointers), places=tuple(places)),
            globals={
                "unknown_004d87ec": 0x1111,
                "unknown_004d87f0": 0x2222,
                **{name: GlobalRange(4) for name in ("unknown_00450c94", "unknown_00450cd0", "unknown_00450cf4", "unknown_00450d1c")},
            },
            externals={
                "simulation_world_unknown_0006b040": note,
                "simulation_view_release_channel": note,
                "simulation_view_detach": detaches,
                "datum_delete": note,
                "simulation_world_players_clear": note,
                "simulation_world_others_clear": note,
                "simulation_entity_database_reset": note,
            },
        )
