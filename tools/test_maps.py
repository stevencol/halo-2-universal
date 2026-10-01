#!/usr/bin/env python3

# runs mapinfo on every map of the game and checks that each opens and that
# every tag's data is where the loader expects it
#
# test_maps.py <maps folder>

import re
import subprocess
import sys
from pathlib import Path

MAPINFO = Path(__file__).resolve().parent.parent / "build" / "mapinfo.exe"


def field(output: str, name: str) -> str:
    match = re.search(rf"^\s*{re.escape(name)}\s+(.+)$", output, re.MULTILINE)
    return match.group(1).strip() if match else "?"


def main() -> None:
    maps = sorted(Path(sys.argv[1]).glob("*.map"))
    failures = 0

    print(f"{'map':<26} {'type':<21} {'groups':>6} {'tags':>6} {'here':>6} {'no data':>7} {'bad':>5}")
    for path in maps:
        result = subprocess.run([MAPINFO, path], capture_output=True, text=True)
        if result.returncode != 0:
            print(f"{path.stem:<26} FAILED: {result.stderr.strip()}")
            failures += 1
            continue

        out = result.stdout
        bad = field(out, "outside (bad)")
        ok = bad == "0" and field(out, "name") == path.stem
        failures += not ok
        print(
            f"{path.stem:<26} {field(out, 'type'):<21} {field(out, 'tag groups'):>6} {field(out, 'tags'):>6} "
            f"{field(out, 'in this file'):>6} {field(out, 'no data here'):>7} {bad:>5}{'' if ok else '  <-- FAILED'}"
        )

    print(f"\n{len(maps) - failures} of {len(maps)} maps passed")
    sys.exit(1 if failures else 0)


if __name__ == "__main__":
    main()
