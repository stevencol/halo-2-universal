# five small functions of the texts that the screens of the interface ask
# for, and whether the player of a gamepad cannot be heard

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF


def wide(random_: random.Random, length: int) -> bytes:
    """A text of two-byte letters with its end, of no more than a length."""
    count = random_.randrange(length)
    return b"".join(struct.pack("<H", random_.randrange(1, 0x3000)) for _ in range(count)) + b"\0\0"


def read_wide(harness, address: int) -> bytes:
    """A text of two-byte letters in the emulator, to its end (no more than 300)."""
    data = b""
    for at in range(0, 600, 2):
        letter = bytes(harness.mu.mem_read(address + at, 2))
        if letter == b"\0\0":
            break
        data += letter
    return data


def read_ascii(harness, address: int) -> bytes:
    data = b""
    for at in range(0x60):
        letter = bytes(harness.mu.mem_read(address + at, 1))
        if letter == b"\0":
            break
        data += letter
    return data


def run(t) -> None:
    random_ = random.Random(768)

    def says(*values):
        return lambda harness, arguments, said=random_.choice(values): (said, tuple(arguments))

    for n in range(600):
        text = Buffer(random_.randbytes(0x200))

        def copied(harness, arguments, text=text):
            return 0, (arguments[0] == text.address, read_wide(harness, arguments[1]))

        for name, getter, length in (
            ("user_interface_text_hopper_name", "network_hoppers_get_name", 16),
            ("user_interface_text_hopper_description", "network_hoppers_get_description", 128),
        ):
            def got(harness, arguments, there=random_.choice((0, 1, 1)), written=wide(random_, length)):
                harness.mu.mem_write(arguments[1], written)
                return there, (arguments[0],)

            t.check(
                name,
                random_.getrandbits(32),
                text,
                globals={"unknown_0055c2e0": random_.choice((0, 3, 9, NONE))},
                externals={
                    "network_life_cycle_session_get_unknown49C8": says(NONE, NONE, 0, 2, 15),
                    getter: got,
                    "network_motd_text_copy": copied,
                },
            )

        t.check(
            "user_interface_text_unknown_0022d67c",
            random_.getrandbits(32),
            text,
            externals={"unknown_0023620d": lambda harness, arguments, text=text: (0, (arguments[0], arguments[1] == text.address))},
        )

        def profile_of(harness, arguments, written=wide(random_, 16), saved_game=random_.getrandbits(32)):
            harness.mu.mem_write(arguments[1] + 8, written)
            harness.mu.mem_write(arguments[2], struct.pack("<I", saved_game))
            return 0, (arguments[0],)

        other = bytearray(random_.randbytes(0x1EC))
        at = random_.randrange(0, 0x1EA, 2)
        other[at : at + 2] = b"\0\0"
        t.check(
            "user_interface_text_profile_name",
            random_.choice((0xE40E, 0xE40F, 0xE410, 0xE411, 0xE412, 0xE413, 0, NONE)),
            text,
            globals={"unknown_0054e5e0": bytes(other)},
            externals={
                "user_interface_controller_get_profile": profile_of,
                "network_motd_text_copy": lambda harness, arguments, text=text: (
                    0,
                    (arguments[0] == text.address, read_wide(harness, arguments[1])),
                ),
            },
        )

        def user_of(harness, arguments, written=random_.randbytes(random_.randrange(0x5F)).replace(b"\0", b"a") + b"\0"):
            harness.mu.mem_write(arguments[1] + 0xC, written)
            return 0, (arguments[0],)

        t.check(
            "user_interface_text_online_name",
            random_.choice((0xE428, 0xE429, 0xE42A, 0xE42B, 0xE42C, 0, NONE)),
            text,
            externals={
                "user_interface_controller_get_online_user": user_of,
                "ascii_string_to_wchar_string": lambda harness, arguments, text=text: (
                    0,
                    (read_ascii(harness, arguments[0]), arguments[1] == text.address, arguments[2]),
                ),
            },
        )

        t.check(
            "user_interface_controller_voice_unavailable",
            random_.randrange(4),
            externals={
                "user_interface_controller_is_guest": says(0, 0, 1),
                "user_interface_controller_voice_allowed": says(0, 1),
                "online_signed_in": says(0, 1),
            },
        )
