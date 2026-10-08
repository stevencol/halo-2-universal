# source/networking/voice_mute_list.c

import random
import struct

from difftest import Buffer, GlobalData, GlobalRange

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
                                    "voice_mute_list_request": lambda harness, arguments: 0,
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

    # ---------- what the library is asked

    TASK_SIZE = 0x14

    def task_externals(new_task: int, datum, **results: int) -> dict:
        """The things that the library is asked are the test's: one is in a buffer, or there is none."""

        def where(harness, arguments):
            if datum is None or arguments[0] == NONE:
                return 0
            return datum.address

        return {
            "unknown_0006b730": lambda harness, arguments: new_task,
            "unknown_0006b910": where,
            "unknown_0006b640": lambda harness, arguments: 0,
            **{name: (lambda harness, arguments, result=result: result) for name, result in results.items()},
        }

    for now in (NONE, 0x00010002):
        for new_task in (0x00050003, NONE):
            for there in (True, False):
                for result in (0, 1, 0x80004005):
                    datum = Buffer(random_.randbytes(TASK_SIZE)) if there else None

                    def startup(harness, arguments, result=result):
                        return result, tuple(arguments)

                    t.check(
                        "voice_mute_list_startup",
                        random_.getrandbits(32),
                        globals={"unknown_004c99c0": now},
                        externals={**task_externals(new_task, datum), "unknown_003a0841": startup},
                        extra=[datum] if datum else [],
                    )

    for controller in range(4):
        for startup_task in (NONE, 0x00010002):
            for task in (NONE, 0x00070004):
                for asked in (1, 0, 0x100):
                    tasks = [random_.choice((NONE, 0x00090005)) for _ in range(4)]
                    tasks[controller] = task
                    t.check(
                        "voice_mute_list_stop",
                        controller,
                        globals={
                            "unknown_004c99c0": startup_task,
                            "voice_mute_list_tasks": struct.pack("<4I", *tasks),
                            "game_voice_muted_players": random_.randbytes(4 * LIST_SIZE),
                            "game_voice_muted_player_counts": struct.pack("<4I", 5, 6, 7, 8),
                        },
                        externals={
                            "unknown_0006b640": lambda harness, arguments: 0,
                            "unknown_0006b890": lambda harness, arguments, asked=asked: asked,
                        },
                    )

        for task in (NONE, 0x00070004):
            for still_there in (True, False):
                for new_task in (0x00050003, NONE):
                    for there in (True, False):
                        for result in (0, 1, 0x80004005):
                            datum = Buffer(random_.randbytes(TASK_SIZE)) if there else None
                            old = Buffer(random_.randbytes(TASK_SIZE))

                            def where(harness, arguments, task=task, still_there=still_there, datum=datum, old=old):
                                if arguments[0] == NONE:
                                    return 0
                                if arguments[0] == task:
                                    return old.address if still_there else 0
                                return datum.address if datum else 0

                            def get(harness, arguments, result=result):
                                # the list and the place for how many are the game's and those of
                                # source/: they are compared as the places they point to
                                controller_, maximum, unknown, handle, players, count = arguments
                                return result, (
                                    controller_,
                                    maximum,
                                    unknown,
                                    handle,
                                    harness.normalize(players),
                                    harness.normalize(count),
                                )

                            tasks = [random_.choice((NONE, 0x00090005)) for _ in range(4)]
                            tasks[controller] = task
                            t.check(
                                "voice_mute_list_request",
                                controller,
                                globals={
                                    "voice_mute_list_tasks": struct.pack("<4I", *tasks),
                                    "game_voice_muted_players": GlobalRange(4 * LIST_SIZE),
                                    "game_voice_muted_player_counts": GlobalData(struct.pack("<4I", 5, 6, 7, 8)),
                                },
                                externals={
                                    "unknown_0006b730": lambda harness, arguments, new_task=new_task: new_task,
                                    "unknown_0006b910": where,
                                    "unknown_0006b640": lambda harness, arguments: 0,
                                    "unknown_003a0832": get,
                                },
                                extra=[old] + ([datum] if datum else []),
                            )
