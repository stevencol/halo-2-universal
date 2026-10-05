# updates README.md, ESTADO.md and CHANGELOG.md of halo-2-universal for a
# milestone; run from the root of the repository
#
#   milestone_docs.py <spec.py>
#
# The spec defines MILESTONE (number), TITLE (Spanish, of the changelog entry),
# COMMIT (the title of the commit), INTRO (a sentence; {count}, {before},
# {percent} and {size} are filled in), FUNCTIONS (name, address, file) and
# BODY (the rest of the entry, markdown). ESTADO (optional): pairs of text to
# replace in ESTADO.md.

import csv
import datetime
import re
import runpy
import sys
from pathlib import Path

spec = runpy.run_path(sys.argv[1])


def rewrite(path, function):
    p = Path(path)
    s = p.read_text(encoding="utf-8", newline="")
    nl = "\r\n" if "\r\n" in s else "\n"
    s = function(s.replace("\r\n", "\n")).replace("\n", nl)
    p.write_text(s, encoding="utf-8", newline="")


def once(s, pattern, replacement):
    s, n = re.subn(pattern, lambda match: replacement, s, count=1, flags=re.S)
    assert n == 1, pattern
    return s


game = {r["address"]: int(r["size"]) for r in csv.DictReader(open("ghidra/functions.csv", newline="")) if r["section"] == ".text"}
rows = list(csv.DictReader(open("symbols/functions.csv", encoding="utf-8")))
done = [r for r in rows if r["source"]]
pending = [r for r in rows if not r["source"]]
unknown = sum(r["name"].startswith("unknown_") for r in pending)
count = len(done)
before = count - len(spec["FUNCTIONS"])
percent = f"{count / len(game):.2%}".replace(".", ",").replace("%", " %")
size = f"{sum(game.get(r['address'], 0) for r in done) / sum(game.values()):.3%}".replace(".", ",").replace("%", " %")
sound = sum("sound_manager.c" in r["source"] for r in done)
for name, address, _ in spec["FUNCTIONS"]:
    assert any(r["name"] == name and int(r["address"], 16) == int(address, 16) for r in done), name

rewrite("README.md", lambda s: once(s, r"hay descompiladas \d+ funciones de unas 11\.265 \(el [\d,]+ %\)", f"hay descompiladas {count} funciones de unas 11.265 (el {percent})"))


def estado(s):
    s = once(s, r"al terminar el hito \d+\.", f"al terminar el hito {spec['MILESTONE']}.")
    s = once(s, r"- \*\*\d+ funciones descompiladas de 11\.265: el [\d,]+ % del juego\*\* \(el\n  [\d,]+ % de los bytes de código\)\.", f"- **{count} funciones descompiladas de 11.265: el {percent} del juego** (el\n  {size} de los bytes de código).")
    s = once(s, r"- Las \d+ están comprobadas", f"- Las {count} están comprobadas")
    s = once(s, r"- Último hito: \*[^*]+\*", f"- Último hito: *{spec['COMMIT']}*")
    s = once(s, r"- Además hay \d+ funciones apuntadas", f"- Además hay {len(pending)} funciones apuntadas")
    s = once(s, r"llama\); \d+ de ellas", f"llama); {unknown} de ellas")
    s = once(s, r"`sound\\sound_manager\.c` \(\d+;", f"`sound\\sound_manager.c` ({sound};")
    for a, b in spec.get("ESTADO", []):
        assert s.count(a) == 1, a
        s = s.replace(a, b)
    return s


rewrite("ESTADO.md", estado)

table = "\n".join(f"| `{name}` | `0x{int(address, 16):08x}` | `{file}` |" for name, address, file in spec["FUNCTIONS"])
intro = spec["INTRO"].format(count=count, before=before, percent=percent, size=size)
entry = f"""## {datetime.date.today().isoformat()} — {spec['TITLE']}

{intro}

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
{table}

{spec['BODY'].strip()}

"""


def changelog(s):
    at = s.index("\n## ") + 1
    return s[:at] + entry + s[at:]


rewrite("CHANGELOG.md", changelog)
print(count, percent, size, len(pending), unknown, sound)
