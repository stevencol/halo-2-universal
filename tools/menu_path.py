#!/usr/bin/env python3

# the functions of halo2ship.exe that the game runs from the start to the main
# menu, and how many of them are decompiled.
#
#   menu_path.py read <log>    reads a log of xemu (xemu.exe -d in_asm -D <log>,
#                              played up to the main menu) and writes
#                              symbols/menu_path.csv
#   menu_path.py report        how many of them symbols/functions.csv has
#   menu_path.py next [count]  the smallest of them that are not decompiled
#
# Only the game's own code counts: the functions of .text at 16-aligned
# addresses below 0x2b0000; the rest are linked libraries (CRT, XDK, Havok).

import bisect
import csv
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PATH = ROOT / "symbols" / "menu_path.csv"


def functions() -> list[tuple[int, int]]:
    with open(ROOT / "ghidra" / "functions.csv", newline="") as f:
        return sorted((int(row["address"], 16), int(row["size"])) for row in csv.DictReader(f) if row["section"] == ".text")


def decompiled() -> set[int]:
    with open(ROOT / "symbols" / "functions.csv", newline="") as f:
        return {int(row[0], 16) for row in csv.reader(f) if len(row) > 2 and row[0] != "address" and row[2]}


def game_code(address: int) -> bool:
    return address % 16 == 0 and address < 0x2B0000


def read(log: str) -> None:
    table = functions()
    starts = [address for address, size in table]
    run = set()
    line_address = re.compile(rb"^0x([0-9a-f]{8}):")
    with open(log, "rb") as f:
        for line in f:
            match = line_address.match(line)
            if match:
                address = int(match.group(1), 16)
                index = bisect.bisect_right(starts, address) - 1
                if index >= 0 and address < table[index][0] + max(table[index][1], 1):
                    run.add(table[index])
    with PATH.open("w", newline="") as f:
        f.write("address,size\n")
        for address, size in sorted(run):
            f.write(f"{address:08x},{size}\n")
    print(f"{len(run)} functions written to {PATH}")


def path() -> list[tuple[int, int]]:
    with PATH.open(newline="") as f:
        return [(int(row["address"], 16), int(row["size"])) for row in csv.DictReader(f)]


def report() -> None:
    done = decompiled()
    game = [(address, size) for address, size in path() if game_code(address)]
    have = [(address, size) for address, size in game if address in done]
    print(
        f"main menu: {len(have)} of {len(game)} functions of the game "
        f"({100 * len(have) / len(game):.2f} %), "
        f"{sum(size for address, size in have)} of {sum(size for address, size in game)} bytes "
        f"({100 * sum(size for address, size in have) / sum(size for address, size in game):.2f} %)"
    )


def next_(count: int) -> None:
    done = decompiled()
    left = sorted((size, address) for address, size in path() if game_code(address) and address not in done)
    for size, address in left[:count]:
        print(f"{address:08x} {size}")


if __name__ == "__main__":
    if len(sys.argv) > 2 and sys.argv[1] == "read":
        read(sys.argv[2])
    elif len(sys.argv) > 1 and sys.argv[1] == "next":
        next_(int(sys.argv[2]) if len(sys.argv) > 2 else 40)
    else:
        report()
