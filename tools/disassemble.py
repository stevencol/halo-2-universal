#!/usr/bin/env python3

# prints the instructions of a function of the game's executable
# (disc/default.xbe), and the reals and the integers at the addresses of data
# that it reads
#
# disassemble.py <address> [size]
#
# without the size, takes it from ghidra/functions.csv; needs the Python
# package capstone

import csv
import re
import struct
import sys
from pathlib import Path

from capstone import CS_ARCH_X86, CS_MODE_32, Cs

ROOT = Path(__file__).resolve().parent.parent


class Xbe:
    def __init__(self, path: Path) -> None:
        self.data = path.read_bytes()
        self.base, _, self.image_size = struct.unpack_from("<3I", self.data, 0x104)
        section_count, sections = struct.unpack_from("<2I", self.data, 0x11C)
        # (address, virtual size, raw address, raw size)
        self.sections = [
            struct.unpack_from("<4I", self.data, sections - self.base + n * 0x38 + 4) for n in range(section_count)
        ]

    def read(self, address: int, size: int) -> bytes:
        """The bytes at an address; zeros where the executable has none stored."""
        for section_address, virtual_size, raw_address, raw_size in self.sections:
            if section_address <= address < section_address + virtual_size:
                offset = address - section_address
                stored = self.data[raw_address + offset : raw_address + min(offset + size, raw_size)]
                return stored + b"\0" * (size - len(stored))
        raise ValueError(f"no section has the address {address:#010x}")


def function_size(address: int) -> int:
    with (ROOT / "ghidra" / "functions.csv").open(newline="") as f:
        for row in csv.DictReader(f):
            if int(row["address"], 16) == address:
                return int(row["size"])
    sys.exit(f"ghidra/functions.csv has no function at {address:#010x}; give the size")


def main() -> None:
    if len(sys.argv) < 2:
        sys.exit("usage: disassemble.py <address> [size]")
    address = int(sys.argv[1], 16)
    size = int(sys.argv[2], 0) if len(sys.argv) > 2 else function_size(address)
    xbe = Xbe(ROOT / "disc" / "default.xbe")

    data_addresses: list[int] = []
    for instruction in Cs(CS_ARCH_X86, CS_MODE_32).disasm(xbe.read(address, size), address):
        print(f"{instruction.address:08x}  {instruction.mnemonic:<8} {instruction.op_str}")
        # an absolute address in the image, between brackets
        for match in re.finditer(r"\[(0x[0-9a-f]+)\]", instruction.op_str):
            value = int(match.group(1), 16)
            if xbe.base <= value < xbe.base + xbe.image_size and value not in data_addresses:
                data_addresses.append(value)

    if data_addresses:
        print("\ndata")
        for value in sorted(data_addresses):
            try:
                raw = xbe.read(value, 4)
            except ValueError:
                continue
            print(f"{value:08x}  {raw.hex(' ')}  integer {struct.unpack('<i', raw)[0]:<12} real {struct.unpack('<f', raw)[0]!r}")


if __name__ == "__main__":
    main()
