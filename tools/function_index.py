#!/usr/bin/env python3

# looks up the index of the game's functions that ExportAllDecompiled.java
# writes (ghidra/decompiled/all/index.jsonl), to choose what to decompile
#
# function_index.py leaves [count]      the functions that call no other, by how many call them
# function_index.py callers [count]     all the functions, by how many call them
# function_index.py strings <text>      the functions that refer to a string with the text
# function_index.py show <address>      a function: what it calls, its strings and its pseudocode
#
# the functions already in symbols/functions.csv are marked with their name

import csv
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DECOMPILED = ROOT / "ghidra" / "decompiled" / "all"


def read_index() -> dict[str, dict]:
    with (DECOMPILED / "index.jsonl").open(encoding="utf-8") as f:
        return {entry["address"]: entry for entry in map(json.loads, f)}


def read_names() -> dict[str, str]:
    with (ROOT / "symbols" / "functions.csv").open(newline="") as f:
        return {row["address"]: row["name"] for row in csv.DictReader(f)}


def line(entry: dict, names: dict[str, str]) -> str:
    name = names.get(entry["address"], entry["name"])
    strings = "  " + " | ".join(entry["strings"][:2])[:60] if entry["strings"] else ""
    return f"{entry['address']}  {entry['size']:5} bytes  {entry['callers']:4} callers  {len(entry['callees']):3} calls  {name}{strings}"


def main() -> None:
    if len(sys.argv) < 2:
        sys.exit(__doc__ or "usage: function_index.py leaves|callers|strings|show")
    index = read_index()
    names = read_names()
    command = sys.argv[1]

    if command in ("leaves", "callers"):
        count = int(sys.argv[2]) if len(sys.argv) > 2 else 40
        entries = [entry for entry in index.values() if command == "callers" or not entry["callees"]]
        for entry in sorted(entries, key=lambda entry: -entry["callers"])[:count]:
            print(line(entry, names))
    elif command == "strings":
        text = sys.argv[2].lower()
        for entry in index.values():
            if any(text in string.lower() for string in entry["strings"]):
                print(line(entry, names))
                for string in entry["strings"]:
                    if text in string.lower():
                        print(f"          {string!r}")
    elif command == "show":
        address = sys.argv[2].lower().removeprefix("0x").rjust(8, "0")
        entry = index[address]
        print(line(entry, names))
        for callee in entry["callees"]:
            if callee in index:
                print("  calls  " + line(index[callee], names))
        for string in entry["strings"]:
            print(f"  string {string!r}")
        print((DECOMPILED / f"{address}.c").read_text(encoding="utf-8"))
    else:
        sys.exit("usage: function_index.py leaves|callers|strings|show")


if __name__ == "__main__":
    main()
