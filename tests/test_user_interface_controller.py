# source/interface/user_interface_controller.c

import random
import struct

from difftest import Buffer, GlobalData

NONE = 0xFFFFFFFF
COUNT, SIZE = 4, 0xC70
FLAGS, VOICE, PROFILE, SAVED_GAME, USER, TASKS = 0, 4, 0x18, 0x1F8, 0x470, 0xC14
BITS = (0, 0x10, 0x10, 0x20, 0x30, 0x30, 0x50, 0x70, 0xFFFFFFEF, 0xFFFFFFDF, 0xFFFFFFFF)


def controllers(random_) -> bytearray:
    """What the game keeps of the four gamepads' players."""
    data = bytearray(random_.randbytes(COUNT * SIZE))
    for index in range(COUNT):
        base = index * SIZE
        struct.pack_into("<I", data, base + FLAGS, random_.choice(BITS) & (random_.getrandbits(32) | 0x70))
        struct.pack_into("<I", data, base + VOICE, random_.choice((NONE, 0, 1, 2, 3)))
        struct.pack_into("<I", data, base + SAVED_GAME, random_.choice((NONE, 0, 5, 5, 0x200005, 0x10007, random_.getrandbits(32))))
        if random_.random() < 0.3:
            data[base + USER:base + USER + 8] = bytes(8)
        elif random_.random() < 0.4 and index:
            data[base + USER:base + USER + 8] = data[USER:USER + 8]
        data[base + USER + 8] = random_.choice((0, 0, 0, 0, 4, 1, 2, 3))
        if random_.random() < 0.3:
            data[base + 0x4E0:base + 0x4E8] = bytes(8)
        for task in range(2):
            struct.pack_into("<I", data, base + TASKS + task * 4, random_.choice((NONE, NONE, 0x10002, random_.getrandbits(32))))
    return data


def run(t) -> None:
    random_ = random.Random(526)

    for n in range(40):
        t.check(
            "user_interface_controller_get",
            random_.choice((0, 1, 2, 3, NONE)),
            globals={"unknown_0054e8e0": GlobalData(bytes(controllers(random_)))},
        )
        t.check("user_interface_controller_next", random_.choice((NONE, 0, 1, 2, 3, 4, 5, 0xFFFFFFFE, 0x80000000, random_.getrandbits(32))))

    for n in range(1500):
        table = {"unknown_0054e8e0": bytes(controllers(random_))}
        t.check("user_interface_controller_is_guest", random_.randrange(COUNT), globals=table)
        t.check("user_interface_controller_find_voice_index", random_.choice((NONE, 0, 1, 2, 3, 4)), globals=table)
        t.check("user_interface_controllers_count_profiles", globals=table)
        t.check("user_interface_controllers_count_profiles_not_guests", globals=table)
        t.check("user_interface_controllers_first_profile", random_.choice((0, 1, 1, 2, NONE)), globals=table)
        t.check("user_interface_controllers_get_only_profile", globals=table)
        t.check("user_interface_controllers_get_only_saved_game", globals=table)

        users = bytearray(random_.randbytes(COUNT * 0x70))
        for index in range(COUNT):
            if random_.random() < 0.3:
                users[index * 0x70:index * 0x70 + 8] = bytes(8)
            users[index * 0x70 + 0xC] = random_.choice((0, 0x41, 0x41, 0x41))
            struct.pack_into("<I", users, index * 0x70 + 0x6C, random_.choice((0, 0, 0x1512F0, 0x80151200)))
        t.check("user_interface_controllers_first_online_user", Buffer(bytes(users)), globals=table)

    # ---------- what is asked of a gamepad's player

    for n in range(1500):
        data = bytes(controllers(random_))
        table = {"unknown_0054e8e0": data}
        index = random_.randrange(COUNT)
        t.check(
            "user_interface_controller_remove_profile",
            index,
            globals=table,
            externals={"cached_saved_game_file_delete": lambda harness, arguments: (0, tuple(arguments))},
        )
        t.check(
            "user_interface_controller_get_profile",
            random_.choice((0, 1, 2, 3, NONE)),
            Buffer(random_.randbytes(0x1E0)),
            Buffer(random_.randbytes(4)),
            globals=table,
        )
        saved_games = [struct.unpack_from("<I", data, other * SIZE + SAVED_GAME)[0] for other in range(COUNT)]
        t.check("user_interface_controllers_have_saved_game", random_.choice(saved_games + [5, NONE]), globals=table)
        t.check("user_interface_controller_get_unknown1FC", index, globals=table)
        t.check("user_interface_controller_get_online_user", index, Buffer(random_.randbytes(0x70)), globals=table)
        t.check("user_interface_controller_has_tasks", index, globals=table)
        t.check("user_interface_controller_get_unknown4E0", index, Buffer(random_.randbytes(0x6A2)), globals=table)
        t.check("user_interface_controller_get_unknownB82", index, Buffer(random_.randbytes(0x92)), globals=table)
        t.check("user_interface_controllers_count_online_not_guests", globals=table)
        t.check("user_interface_controller_count_guests", index, globals=table)
        t.check("user_interface_controllers_get_profile_mask", globals=table)
        t.check("user_interface_controllers_get_online_mask", globals=table)
        t.check("user_interface_controllers_any_online", globals=table)
        t.check("user_interface_controller_get_name", index, globals={"unknown_0054e8e0": GlobalData(data)})
        t.check("user_interface_controller_get_profile_unknownE4", index, globals=table)
        t.check(
            "user_interface_unknown_001907bf",
            random_.choice((0, 1, 1, 2, 0xFF)),
            random_.choice((0, 249, 250, 251, 0xFFFF, random_.getrandbits(16))),
        )

    # ---------- a gamepad's player signs out, and what is done with all of them

    def told(harness, arguments):
        return 0, tuple(harness.normalize(value) for value in arguments)

    for n in range(1000):
        data = bytes(controllers(random_))
        table = {"unknown_0054e8e0": GlobalData(data)}
        index = random_.randrange(COUNT)
        t.check("game_voice_unknown_004cd904_clear", index, globals={"unknown_004cd904": random_.randbytes(4 * 0xD0)})
        t.check("user_interface_controller_close_tasks", index, globals=table, externals={"online_task_close": told})
        t.check("user_interface_controller_sign_out", index, globals=table, externals={"online_task_close": told})
        t.check(
            "user_interface_controller_reset",
            index,
            globals={**table, "unknown_004cd904": random_.randbytes(4 * 0xD0)},
            externals={"online_task_close": told, "cached_saved_game_file_delete": told},
        )
        t.check(
            "user_interface_controller_remove_guests",
            index,
            globals=table,
            externals={"user_interface_controller_reset": told},
        )
        t.check(
            "user_interface_controller_set_online",
            index,
            random_.choice((0, 1)),
            globals=table,
            externals={"unknown_00190eb3": told},
        )
        t.check(
            "user_interface_controller_restart_tasks",
            index,
            globals=table,
            externals={
                "online_task_close": told,
                "unknown_000abc70": lambda harness, arguments, task=random_.choice((NONE, 0x30001, 0x12340005)): (
                    task,
                    (harness.normalize(arguments[0]), arguments[1]),
                ),
            },
        )
        t.check(
            "user_interface_controllers_write_changed_profiles",
            globals=table,
            externals={"player_profile_write": lambda harness, arguments: (random_.choice((0, 1)), told(harness, arguments)[1])},
        )
        t.check(
            "user_interface_controllers_unknown_00190e71",
            random_.choice((0, 1, random_.getrandbits(32))),
            globals=table,
            externals={"unknown_002153dd": told},
        )
        current = {}

        def now(harness, arguments):
            # the same answer in the game and in source/, whatever the order
            return current.setdefault(arguments[0], (arguments[0] * 7 + 3) & 0xFFFFFFFF), (arguments[0],)

        t.check(
            "user_interface_controllers_update_saved_games",
            globals={**table, "saved_game_files_cache_valid": bytes((random_.choice((0, 1)),))},
            externals={"saved_game_files_update": told, "saved_game_file_get_current_index": now},
        )

    for n in range(300):
        t.check(
            "user_interface_controllers_logoff",
            globals={"unknown_0054e8e0": GlobalData(bytes(controllers(random_)))},
            externals={"online_logoff": told, "user_interface_controller_reset": told, "unknown_00199e2e": told},
        )
