#!/usr/bin/env python3

# shows the non-zero fields of a cache file header and of the tag header it
# points to, for working out the layout

import struct
import sys
from pathlib import Path


def printable(raw: bytes) -> str:
    return "".join(chr(b) if 0x20 <= b < 0x7F else "." for b in raw)


def dump(path: Path) -> None:
    with path.open("rb") as f:
        header = f.read(0x800)
        print(f"== {path.name}")
        offset = 0
        while offset < 0x800:
            (value,) = struct.unpack_from("<I", header, offset)
            if value:
                # runs of text are shown once, as text
                if all(0x20 <= b < 0x7F or b == 0 for b in header[offset : offset + 4]) and header[offset] >= 0x20:
                    end = header.find(b"\0", offset)
                    if end < 0:
                        end = 0x800
                    if end - offset > 4:
                        print(f"  {offset:#05x}  {header[offset:end].decode('latin-1')!r}")
                        offset = (end + 4) & ~3
                        continue
                print(f"  {offset:#05x}  {value:#010x}  {value:>12,}  {printable(header[offset:offset + 4])}")
            offset += 4

        tag_offset, tag_index_size = struct.unpack_from("<2I", header, 0x10)
        f.seek(tag_offset)
        tags = f.read(0x40)
        print(f"  tag header at {tag_offset:#x} (index size {tag_index_size:#x})")
        for i in range(0, 0x40, 4):
            (value,) = struct.unpack_from("<I", tags, i)
            print(f"    {i:#04x}  {value:#010x}  {value:>12,}  {printable(tags[i:i + 4])}")


def main() -> None:
    for name in sys.argv[2:]:
        dump(Path(sys.argv[1]) / name)


if __name__ == "__main__":
    main()
