#!/usr/bin/env python3

# lists the source file paths left in the executable as text (assertion and
# error messages name them), and compares them with the Halo 1 source tree

import re
import sys
from collections import Counter
from pathlib import Path


def main() -> None:
    data = Path(sys.argv[1]).read_bytes()
    halo1_root = Path(sys.argv[2]) if len(sys.argv) > 2 else None

    paths = sorted(
        {
            m.group().decode("latin-1").lower()
            for m in re.finditer(rb"[A-Za-z]:\\[\x20-\x7e]{3,200}?\.(?:cpp|inl|hpp|c|h)(?![a-z])", data)
        }
    )
    print(f"source paths in the executable: {len(paths)}")

    roots = Counter("\\".join(path.split("\\")[:3]) for path in paths)
    print("\nby root")
    for root, count in roots.most_common(15):
        print(f"  {count:5}  {root}")

    game = [path for path in paths if path.startswith("c:\\halo2\\source\\")]
    folders = Counter(path.split("\\")[3] for path in game)
    print(f"\nc:\\halo2\\source: {len(game)} files, by folder")
    for folder, count in sorted(folders.items()):
        print(f"  {count:5}  {folder}")

    if halo1_root:
        halo1 = {path.name.lower() for path in halo1_root.rglob("*") if path.suffix.lower() in (".c", ".h", ".cpp")}
        names = {path.rsplit("\\", 1)[1] for path in game}
        shared = sorted(names & halo1)
        print(f"\nfile names also in the Halo 1 source: {len(shared)} of {len(names)}")

    for word in (b"Havok", b"havok", b"hkBase", b"hkMath"):
        print(f"\n{word.decode()!r} occurs {data.count(word)} times")

    out = Path(sys.argv[1]).parent.parent / "ghidra" / "source_paths.txt"
    out.write_text("\n".join(paths) + "\n", encoding="utf-8")
    print(f"\nfull list: {out}")


if __name__ == "__main__":
    main()
