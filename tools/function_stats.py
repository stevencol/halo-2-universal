#!/usr/bin/env python3

# summarises functions.csv (ExportFunctions.java): how many functions the game
# code has, how large they are, and how many the analysis named by itself

import csv
import sys


def main() -> None:
    with open(sys.argv[1], newline="") as f:
        rows = list(csv.DictReader(f))

    text = [row for row in rows if row["section"] == ".text"]
    named = [row for row in text if not row["name"].startswith(("FUN_", "thunk_FUN_"))]
    print(f"functions in .text: {len(text)}")
    print(f"named by the analysis: {len(named)} ({sum(int(row['size']) for row in named):,} bytes)")

    sizes = sorted(int(row["size"]) for row in text)
    print(f"median size: {sizes[len(sizes) // 2]} bytes; largest: {sizes[-5:]}")
    for low, high in ((0, 32), (32, 128), (128, 512), (512, 2048), (2048, None)):
        group = [size for size in sizes if size >= low and (high is None or size < high)]
        label = f"{low}-{high}" if high else f"{low}+"
        print(f"  {label:>10} bytes  {len(group):6} functions  {sum(group):>10,} bytes")

    print("sample names: " + ", ".join(row["name"] for row in named[:40]))


if __name__ == "__main__":
    main()
