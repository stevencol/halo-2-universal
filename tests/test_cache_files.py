# source/cache/cache_files.c

import struct

from difftest import Buffer

BUILD = b"02.09.27.09809"


def header(
    signature: bytes = b"daeh",
    footer: bytes = b"toof",
    version: int = 8,
    length: int = 0x01C5BC00,
    scenario_type: int = 1,
    name: bytes = b"ascension",
) -> Buffer:
    """A cache file header with the fields that the game examines."""
    data = bytearray(0x800)
    data[0:4] = signature
    struct.pack_into("<2i", data, 4, version, length)
    data[0x120 : 0x120 + len(BUILD)] = BUILD
    struct.pack_into("<h", data, 0x140, scenario_type)
    data[0x198 : 0x198 + len(name)] = name
    data[0x7FC:0x800] = footer
    return Buffer(bytes(data))


def run(t) -> None:
    for scenario_type in range(-3, 9):
        t.check("cache_file_maximum_length", scenario_type)
    for scenario_type in (0x100, 0x10000, 0x7FFFFFFF, -0x80000000):
        t.check("cache_file_maximum_length", scenario_type)

    for build in (BUILD, b"", b"02.09.27.0980", BUILD + b"0", b"02.09.27.09808", b"01.01.14.2342", b"\xff" * 20):
        t.check("cache_file_build_verify", Buffer(build + b"\0"))

    t.check("cache_file_header_verify", header())
    t.check("cache_file_header_verify", header(signature=b"head"))
    t.check("cache_file_header_verify", header(footer=b"foot"))
    for version in (0, 5, 7, 8, 9, -1):
        t.check("cache_file_header_verify", header(version=version))
    for scenario_type in range(-2, 8):
        # at the greatest length of each type, one less and one more
        for length in (0x11800000, 0x05000000, 0x0B400000, 0x20800000):
            for difference in (-1, 0, 1):
                t.check("cache_file_header_verify", header(scenario_type=scenario_type, length=length + difference))
    for length in (0, 1, -1, 0x7FFFFFFF, -0x80000000):
        t.check("cache_file_header_verify", header(length=length))
    for name in (b"", b"a", b"a" * 30, b"a" * 31, b"a" * 32):
        t.check("cache_file_header_verify", header(name=name))
