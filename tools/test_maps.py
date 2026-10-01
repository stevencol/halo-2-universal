#!/usr/bin/env python3

# runs the tools on every map of the game and checks that each opens, that
# every tag's data is where the loader expects it, and that the collision
# geometry of its structure bsps is whole
#
# test_maps.py <maps folder>
#
# the OBJ files are left in build/obj

import contextlib
import io
import re
import struct
import subprocess
import sys
from pathlib import Path

from bsp2obj import export_map
from cache_file import CacheFileError

BUILD = Path(__file__).resolve().parent.parent / "build"
MAPINFO = BUILD / "mapinfo.exe"


def field(output: str, name: str) -> str:
    match = re.search(rf"^\s*{re.escape(name)}\s+(.+)$", output, re.MULTILINE)
    return match.group(1).strip() if match else "?"


def main() -> None:
    maps = sorted(Path(sys.argv[1]).glob("*.map"))
    obj_folder = BUILD / "obj"
    obj_folder.mkdir(parents=True, exist_ok=True)
    failures = 0

    print(
        f"{'map':<22} {'type':<21} {'tags':>5} {'bad':>4}   "
        f"{'bsps':>4} {'vertices':>8} {'surfaces':>8} {'broken':>6} {'reversed':>8}"
    )
    for path in maps:
        try:
            info = subprocess.run([MAPINFO, path], capture_output=True, text=True)
        except OSError as error:
            # Windows can refuse to start a program that is not signed
            sys.exit(f"cannot run {MAPINFO.name}: {error}")
        if info.returncode != 0:
            print(f"{path.stem:<22} FAILED: {info.stderr.strip()}")
            failures += 1
            continue

        try:
            with contextlib.redirect_stdout(io.StringIO()):
                totals = export_map(path, obj_folder / (path.stem + ".obj"))
        except (CacheFileError, struct.error) as error:
            print(f"{path.stem:<22} FAILED: {error}")
            failures += 1
            continue

        map_type = field(info.stdout, "type")
        bad = field(info.stdout, "outside (bad)")
        ok = (
            bad == "0"
            and field(info.stdout, "name") == path.stem
            and totals.broken == 0
            and totals.reversed == 0
            and totals.surfaces > 0
        )
        failures += not ok
        print(
            f"{path.stem:<22} {map_type:<21} {field(info.stdout, 'tags'):>5} {bad:>4}   "
            f"{totals.bsps:>4} {totals.vertices:>8} {totals.surfaces:>8} {totals.broken:>6} {totals.reversed:>8}"
            f"{'' if ok else '  <-- FAILED'}"
        )

    print(f"\n{len(maps) - failures} of {len(maps)} maps passed")
    sys.exit(1 if failures else 0)


if __name__ == "__main__":
    main()
