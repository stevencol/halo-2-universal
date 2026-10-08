# where the game starts and ends (source/shell/shell.c)
#
# What the game sets up and takes down is not run: the test stands in for
# each function, and the order of the calls is what is compared

import random
import struct

from difftest import PHYSICAL_ADDRESS, Buffer, Memory, Pointer, StopCall

NONE = 0xFFFFFFFF
DEVICE = PHYSICAL_ADDRESS + 0x50000
DEVICE_SIZE = 0x24A0
REFERENCE_COUNT = 0x938
SHELL_MEMORY = 0x00D00000

INITIALIZE = (
    "unknown_002d5d0a async_initialize global_preferences_initialize fonts_initialize cache_files_initialize animation_cache_new "
    "game_state_initialize input_initialize sound_initialize"
).split()
DISPOSE = (
    "game_state_close_file sound_dispose input_dispose unknown_000141c0 animation_cache_delete font_cache_dispose "
    "font_characters_dispose"
).split()


def nothing(harness, arguments):
    return 0


def stop(harness, arguments):
    raise StopCall


def initialize(harness, initialized: int) -> tuple:
    # how the memory is shared out when the game is set up: the function clears it before it ends
    address = harness.global_addresses["physical_memory_globals"]
    return initialized, (bytes(harness.mu.mem_read(address, 0x34)),)


def run(t) -> None:
    random_ = random.Random(23)

    # ---------- setting up and taking down

    # shell_platform_initialize is always true: the compiler of source/ knows it,
    # and does not look at what it returns
    for platform in (1,):
        for device in (DEVICE, 0):

            def rasterizer_initialize(harness, arguments, device=device):
                # what makes the device, or cannot
                harness.mu.mem_write(harness.global_addresses["global_d3d_device"], struct.pack("<I", device))
                return 0

            t.check(
                "shell_initialize",
                globals={"shell_memory_globals": random_.randbytes(16), "global_d3d_device": 0},
                externals={
                    **{name: nothing for name in INITIALIZE},
                    "VirtualAlloc": lambda harness, arguments: SHELL_MEMORY,
                    "shell_platform_initialize": lambda harness, arguments, platform=platform: platform,
                    "_control87": nothing,
                    "unknown_00012560": rasterizer_initialize,
                },
            )

    for memory in (SHELL_MEMORY, 0):
        async_globals = random_.randbytes(0x25A0)
        t.check(
            "shell_dispose",
            globals={
                "shell_memory_globals": struct.pack("<4I", 5, memory, 6, 7),
                "async_globals": async_globals,
                # game_state_dispose is in the game's function itself
                "game_state_globals": bytes([memory != 0]) + random_.randbytes(0x253F),
                "xbox_game_state_globals": random_.randbytes(0x28),
                "font_slots": random_.randbytes(0x1220),
            },
            externals={
                **{name: nothing for name in DISPOSE},
                "CloseHandle": lambda harness, arguments: 1,
                "VirtualFree": lambda harness, arguments: 1,
            },
        )

    for initialized in (1, 0):
        for stage in (0, 3):
            lows = [0x01000000 + n for n in range(5)]
            highs = [0x02000000 + n for n in range(5)]
            t.check(
                "shell_main",
                globals={
                    "physical_memory_globals": struct.pack("<13I", stage, 0x61000, 0x3145000, *lows, *highs),
                },
                externals={
                    "shell_test_device": lambda harness, arguments: 1,
                    "physical_memory_initialize": nothing,
                    "shell_initialize": lambda harness, arguments, initialized=initialized: initialize(
                        harness, initialized
                    ),
                    "main_loop": nothing,
                    "shell_dispose": nothing,
                    # the function has no return after this
                    "XPhysicalFree": stop,
                },
            )

    # ---------- how the game was started

    for result in (0, 0x490):
        for launch_type in (0, 1, 2):
            for data in (b"XDEMOS\0rest", b"XDEMOS", b"XDEMOSX", b"xdemos\0", b"\0", b"XDEMO\0S"):
                for name in (b"", b"ab", b"abcdefg", b"abcdefgh", b"abc\0xyz"):

                    def launch_info(harness, arguments, result=result, launch_type=launch_type, data=data):
                        harness.mu.mem_write(arguments[0], struct.pack("<I", launch_type))
                        harness.mu.mem_write(arguments[1], data.ljust(0xC00, b"\x55"))
                        return result, ()

                    t.check(
                        "shell_platform_initialize",
                        globals={"shell_launch_name": name.ljust(8, b"\0")},
                        externals={"XGetLaunchInfo": launch_info},
                    )

    # ---------- the picture

    for standard in (0, 1, 2, 3, 4):
        for flags in (0, 0x01, 0x08, 0x10, 0x40, 0x59, 0xFFFFFFFF, 0x100, 0x20):
            t.check(
                "shell_get_video_settings",
                globals={"xbox_video_settings": random_.randbytes(8)},
                externals={
                    "XGetVideoStandard": lambda harness, arguments, standard=standard: standard,
                    "XGetVideoFlags": lambda harness, arguments, flags=flags: flags,
                },
            )

    for widescreen in (0, 1):
        for progressive in (0, 1):
            for refresh_rate in (60, 50, -1):
                for made in (DEVICE, 0):

                    def create_device(harness, arguments, made=made):
                        flags, device, parameters = arguments
                        harness.mu.mem_write(device, struct.pack("<I", made))
                        to_global = device == harness.global_addresses["global_d3d_device"]
                        record = (flags, to_global, bytes(harness.mu.mem_read(parameters, 0x44)))
                        return 0, record

                    settings = struct.pack("<h6B", refresh_rate, widescreen, 7, 8, 9, progressive, 10)
                    t.check(
                        "shell_create_device",
                        globals={
                            "xbox_video_settings": settings,
                            "global_d3d_device": 0x1234,
                            "shell_device_requested": 0,
                            "unknown_0047ffd8": random_.randbytes(0xD4),
                            "unknown_00484ce8": random_.randbytes(0xD4),
                        },
                        externals={"d3d_create_device": create_device},
                    )

    for created in (1, 0):
        for device, d3d in ((DEVICE, DEVICE), (0, DEVICE)):
            for reference_count in (1, 2, 5):
                data = bytearray(random_.randbytes(DEVICE_SIZE))
                struct.pack_into("<i", data, REFERENCE_COUNT, reference_count)
                t.check(
                    "shell_test_device",
                    globals={"global_d3d_device": device, "d3d_device": d3d},
                    memory={DEVICE: Memory(bytes(data))},
                    externals={
                        "shell_get_video_settings": nothing,
                        "shell_create_device": lambda harness, arguments, created=created: created,
                        "unknown_003f9d00": nothing,
                        "d3d_device_destroy": nothing,
                    },
                )

    # ---------- the memory that the console's libraries ask for

    import random as random_module

    random_x = random_module.Random(108)
    allocators = (0x82, 0x81, 0x83, 0x88, 0x89, 0x8A, 0x8B, 0x00, 0xFF, 0x182)
    for allocator in allocators:
        for kind in (1, 0, 2, 0x2001, 0x1001, 0x1FFF):
            for zero in (0, 0x40000000):
                attributes = ((allocator << 16) & 0x7FFFFFFF) | kind | zero | (0x80000000 if random_x.getrandbits(1) else 0)
                for name in ("XMemAlloc", "xmem_static_allocate"):
                    buffer = Buffer(random_x.randbytes(0xD0))
                    t.check(
                        name,
                        random_x.choice((0, 0x10, 0x1000)),
                        attributes,
                        globals={"xmem_static_buffer+0": buffer, "xmem_static_buffer_used": random_x.randbytes(1)},
                        externals={
                            "XMemAllocDefault": lambda harness, arguments: 0x80123000,
                            "game_voice_allocate": lambda harness, arguments: 0x80456000,
                        },
                    )
                buffer = Buffer(random_x.randbytes(0xD0))
                for address in (0x80999000, 0x80123000, 0):
                    t.check(
                        "XMemFree",
                        address,
                        attributes,
                        globals={"xmem_static_buffer": 0x80999000, "xmem_static_buffer_used": random_x.choice((b"\x01", b"\x00", b"\x80"))},
                        extra=[buffer],
                        externals={
                            "XMemFreeDefault": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
                            "game_voice_free": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
                        },
                    )

    # ---------- the texture of a bitmap

    for n in range(200):
        bitmap = bytearray(random_x.randbytes(0x74))
        frame = random_x.choice((5, 100, -3, 0x7FFFFFF0))
        struct.pack_into("<i", bitmap, 0x70, random_x.choice((frame, frame - 1, frame + 1, frame + 1)))
        struct.pack_into("<I", bitmap, 0x50, random_x.choice((0, 0x80123400, 0x80123400)))
        for name in ("shell_bitmap_get_texture_blocking", "shell_bitmap_get_texture"):
            t.check(
                name,
                Buffer(bytes(bitmap)),
                struct.unpack("<I", struct.pack("<f", random_x.choice((0.0, 1.5, -0.5))))[0],
                globals={"texture_cache_globals+52": frame & 0xFFFFFFFF},
                externals={
                    "texture_cache_bitmap_get_loaded": lambda harness, arguments, loaded=random_x.choice((0, 0, 0x80555000)): loaded,
                    "texture_cache_bitmap_get_hardware_format": lambda harness, arguments, made=random_x.choice((0, 0x80777000)): made,
                },
            )
