#!/usr/bin/env python3

# prints what the XBE header and the cache file headers say: the build, the
# sections, the SDK libraries linked in, and each map's version

import hashlib
import re
import struct
import sys
import time
from pathlib import Path

ENTRY_KEYS = {"retail": 0xA8FC57AB, "debug": 0x94859D4B}
THUNK_KEYS = {"retail": 0x5B6D40B6, "debug": 0xEFB1F152}


def cstring(data: bytes, offset: int) -> str:
    end = data.index(b"\0", offset)
    return data[offset:end].decode("latin-1")


def inspect_xbe(path: Path) -> None:
    data = path.read_bytes()
    if data[:4] != b"XBEH":
        sys.exit(f"{path} is not an XBE")

    (base, headers_size, image_size, _, timestamp, certificate, section_count, sections, _, entry) = struct.unpack_from(
        "<10I", data, 0x104
    )
    pe_timestamp, debug_path, debug_file = struct.unpack_from("<3I", data, 0x148)
    thunk, _, library_count, libraries = struct.unpack_from("<4I", data, 0x158)

    print(f"file           {path}")
    print(f"size           {len(data):,} bytes")
    print(f"sha256         {hashlib.sha256(data).hexdigest()}")
    print(f"base address   {base:#010x}")
    print(f"image size     {image_size:#x} ({image_size:,} bytes)")
    print(f"built          {time.strftime('%Y-%m-%d %H:%M:%S', time.gmtime(timestamp))} UTC")
    print(f"PE built       {time.strftime('%Y-%m-%d %H:%M:%S', time.gmtime(pe_timestamp))} UTC")
    print(f"debug path     {cstring(data, debug_path - base)}")
    print(f"debug file     {cstring(data, debug_file - base)}")

    # the entry point and the kernel thunk table are XORed with a key that
    # differs between retail and debug executables; the right key gives an
    # address inside the image
    for kind in ENTRY_KEYS:
        address = entry ^ ENTRY_KEYS[kind]
        if base <= address < base + image_size:
            print(f"entry point    {address:#010x} ({kind})")
            print(f"kernel thunks  {thunk ^ THUNK_KEYS[kind]:#010x}")

    c = certificate - base
    title_id, = struct.unpack_from("<I", data, c + 0x8)
    title = data[c + 0xC : c + 0xC + 80].decode("utf-16-le").rstrip("\0")
    media, region, _, disc, version = struct.unpack_from("<5I", data, c + 0x9C)
    print(f"title          {title!r}  id {title_id:#010x}")
    print(f"region flags   {region:#x}   media {media:#x}   disc {disc}   version {version:#x}")

    print(f"\nsections ({section_count})")
    print(f"  {'name':<12} {'address':>10} {'virtual':>10} {'raw':>10}  flags")
    code = 0
    for i in range(section_count):
        flags, address, virtual_size, _, raw_size, name = struct.unpack_from("<6I", data, sections - base + i * 0x38)
        described = [text for bit, text in ((1, "write"), (2, "preload"), (4, "exec"), (8, "inserted")) if flags & bit]
        if flags & 4:
            code += virtual_size
        print(
            f"  {cstring(data, name - base):<12} {address:#10x} {virtual_size:>10,} {raw_size:>10,}  "
            + " ".join(described)
        )
    print(f"  executable bytes: {code:,}")

    print(f"\nlibraries ({library_count})")
    for i in range(library_count):
        name, major, minor, build, flags = struct.unpack_from("<8s4H", data, libraries - base + i * 16)
        print(f"  {name.rstrip(b'\0').decode('latin-1'):<10} {major}.{minor}.{build}   flags {flags:#06x}")


def inspect_map(path: Path) -> None:
    with path.open("rb") as f:
        header = f.read(0x800)
    signature, version, length = struct.unpack_from("<4s2i", header, 0)
    footer = header[-4:]
    # the header's layout is not known yet: show its strings instead
    strings = [m.group().decode("latin-1") for m in re.finditer(rb"[\x20-\x7e]{4,}", header[12:-4])]
    print(
        f"  {path.name:<28} {signature[::-1].decode('latin-1')}/{footer[::-1].decode('latin-1')}"
        f"  v{version}  {length:>11,}  {' | '.join(strings)}"
    )


def main() -> None:
    disc = Path(sys.argv[1])
    inspect_xbe(disc / "default.xbe")
    print("\nmaps")
    for path in sorted((disc / "maps").glob("*.map")):
        inspect_map(path)


if __name__ == "__main__":
    main()
