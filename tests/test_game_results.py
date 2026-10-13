# source/game/game_results.c

import random

from difftest import Buffer, GlobalData, GlobalRange

NONE = 0xFFFFFFFF
PLAYERS, TEAMS, PLAYER, TEAM = 16, 16, 0xA4, 0x18


def run(t) -> None:
    random_ = random.Random(733)

    for n in range(300):
        def everything() -> dict:
            players = bytearray(random_.randbytes(PLAYERS * PLAYER))
            for index in range(PLAYERS):
                players[index * PLAYER] = random_.choice((0, 1, 1, 1, 0xFF))
                players[index * PLAYER + 0x8C] = random_.choice((0xFF, 0, 1, 7, 8, 15, 16, 0x80))
            teams = bytearray(random_.randbytes(TEAMS * TEAM))
            for index in range(TEAMS):
                teams[index * TEAM] = random_.choice((0, 1, 1, 1))
            return {
                # the flags are one byte each, and two of them are next to each other in the game
                "unknown_00510ca0": bytes((random_.choice((1, 1, 1, 0)),)),
                "unknown_00510cb0": bytes((random_.choice((1, 1, 1, 0)),)),
                "unknown_00510cb1": bytes((random_.choice((1, 1, 0)),)),
                "unknown_00510e2c": random_.getrandbits(32),
                "unknown_00510e30": bytes((random_.choice((0, 1, 0xFE, 0xFF)),)),
                "unknown_00511034": GlobalData(bytes(players)),
                "unknown_00511a74": GlobalData(bytes(teams)),
                "unknown_00511bf4": GlobalData(random_.randbytes(PLAYERS * 0x1B5 * 2)),
                "unknown_00515294": GlobalData(random_.randbytes(PLAYERS * PLAYERS * 4)),
                "unknown_00515694": GlobalData(random_.randbytes(TEAMS * 0x2D * 2)),
                "unknown_00450aac": GlobalRange(4),
            }

        player = random_.randrange(PLAYERS)
        a_player = lambda: random_.choice((NONE, player, player, player))
        a_team = lambda: random_.choice((NONE, 0xFFFFFFF0, 0, 3, 7, 8, 15, 16))
        low = lambda value: random_.getrandbits(24) << 8 | value

        t.check(
            "game_results_get_statistic",
            a_player(),
            random_.choice((NONE, NONE, random_.randrange(TEAMS))),
            random_.randrange(0x2D),
            globals=everything(),
        )
        t.check("game_results_get_player_unknown4E", player, random_.randrange(8), random_.randrange(8), globals=everything())
        t.check("game_results_get_player_unknown2D", player, random_.randrange(0x20), globals=everything())
        t.check("game_results_get_player_versus_player", player, random_.randrange(PLAYERS), random_.randrange(2), globals=everything())
        t.check("game_results_get_unknown_00510e2c", globals=everything())
        t.check(
            "game_results_set_player_unknownA2",
            a_player(),
            random_.getrandbits(32),
            low(random_.randrange(0x100)),
            globals=everything(),
        )
        t.check(
            "game_results_set_team_unknown2",
            a_team(),
            random_.choice((0, 5, 0x7FFF, 0x8000, 0xFFFF8000, 0xFFFF7FFF, 0x12345678, 0x87654321, random_.getrandbits(32))),
            low(random_.randrange(0x100)),
            globals=everything(),
        )
        for name in (
            "game_results_get_player_unknownA2",
            "game_results_get_player_unknownA0",
            "game_results_get_player_unknown10",
            "game_results_get_player_unknown2",
            "game_results_get_player_team",
            "game_results_get_player_unknown98",
        ):
            t.check(name, a_player(), globals=everything())
        for name in ("game_results_get_team_unknown2", "game_results_get_team_unknown1"):
            t.check(name, a_team(), globals=everything())
        t.check("game_results_get_player_unknown50", a_player(), Buffer(random_.randbytes(16)), globals=everything())
