# source/networking/voice_mute_list.c

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
PROFILE_SIZE = 0xC70
LIST_SIZE = 4000


def run(t) -> None:
    random_ = random.Random(288)

    for controller in range(4):
        t.check(
            "voice_mute_list_reset",
            controller,
            globals={
                "unknown_004c99c0": 0x1234,
                "voice_mute_list_tasks": random_.randbytes(0x10),
                "game_voice_muted_players": random_.randbytes(4 * LIST_SIZE),
                "game_voice_muted_player_counts": random_.randbytes(0x10),
                "unknown_004cd904": random_.randbytes(8),
            },
        )

        for session in (0x12340005, NONE):
            for doing in (1, 0, 2):
                for count in (0, 5, NONE):
                    for result in (0, 1, 0x80004005):
                        for name, library in (
                            ("voice_mute_list_add", "unknown_003a084c"),
                            ("voice_mute_list_remove", "unknown_003a086e"),
                        ):
                            counts = [random_.choice((3, NONE)) for _ in range(4)]
                            counts[controller] = count
                            t.check(
                                name,
                                controller,
                                Buffer(random_.randbytes(0xC)),
                                globals={
                                    "unknown_00467214": session,
                                    "game_voice_muted_player_counts": struct.pack("<4I", *counts),
                                },
                                externals={
                                    "unknown_0006cd50": lambda harness, arguments, doing=doing: doing,
                                    "unknown_003a084c": lambda harness, arguments, result=result: result,
                                    "unknown_003a086e": lambda harness, arguments, result=result: result ^ 0x80000000,
                                    "unknown_00058b70": lambda harness, arguments: 0,
                                },
                            )

        for flags in (0x20, 0, 0xFFFFFFDF, 0xFFFFFFFF):
            for count in (0, 1, 7, 250, NONE):
                for where in ("first", "last", "past the count", "nearly", "nowhere"):
                    player = random_.randbytes(0xC)
                    lists = bytearray(random_.randbytes(4 * LIST_SIZE))
                    counts = [random_.choice((3, NONE)) for _ in range(4)]
                    counts[controller] = count
                    # the same player in the list of another gamepad does not count
                    lists[((controller + 1) % 4) * LIST_SIZE : ((controller + 1) % 4) * LIST_SIZE + 0xC] = player
                    listed = 0 if count in (0, NONE) else count
                    index = {"first": 0, "last": max(listed - 1, 0), "past the count": min(listed, 249), "nearly": 0}.get(where)
                    if index is not None:
                        at = controller * LIST_SIZE + index * 0x10
                        lists[at : at + 0xC] = player
                        if where == "nearly":
                            lists[at + random_.randrange(0xC)] ^= 1 << random_.randrange(8)
                    profiles = bytearray(random_.randbytes(4 * PROFILE_SIZE))
                    struct.pack_into("<I", profiles, controller * PROFILE_SIZE, flags)
                    t.check(
                        "voice_mute_list_has_player",
                        controller,
                        Buffer(player),
                        globals={
                            "game_voice_muted_players": bytes(lists),
                            "game_voice_muted_player_counts": struct.pack("<4I", *counts),
                            "unknown_0054e8e0": bytes(profiles),
                        },
                    )
