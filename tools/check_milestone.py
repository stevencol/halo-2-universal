#!/usr/bin/env python3

# checks a milestone before its commit: what it adds to symbols/, the C of
# source/, its tests and its mutations; with --run it also runs the tests and
# the mutations. Says what is wrong, and writes everything it says, with the
# output of the runs, to build/scratch/check_milestone.txt (for agents whose
# terminal does not show the output of commands)
#
# check_milestone.py <milestone> [--run] [--base <commit>]
#
# What is compared is the working tree with the commit (HEAD if no --base):
# the rows of symbols/*.csv may only be added at the end. The tests of the
# milestone are tests/test_small_<milestone>.py and its mutations
# tests/mutations_<milestone>.py.

import csv
import os
import re
import runpy
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
REPORT = ROOT / "build" / "scratch" / "check_milestone.txt"
CLANG = Path(r"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\Llvm\x64\bin")

# what tools/difftest.py knows how to compare: with anything else the value
# that a function returns is not compared at all
RETURNS = {"void", "int", "bool", "byte", "short", "pointer", "pair", "string", "wstring", "real", "real:eax", "real:xmm0"}
ARGUMENTS = {"eax", "ebx", "ecx", "edx", "esi", "edi", "ebp", "stack", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"}
CONVENTIONS = {"", "stdcall", "thiscall"}

FUNCTION_ROW = re.compile(r"^([0-9a-f]{8}),([A-Za-z_][A-Za-z0-9_]*),(source/[^,]+\.c),([a-z0-9 ]*),([a-z0-9:]*)(?:,([a-z]*))?$")
GLOBAL_ROW = re.compile(r"^([0-9a-f]{8}),([A-Za-z_][A-Za-z0-9_]*),(source/[^,]+\.c)$")

lines: list[str] = []
errors: list[str] = []


def say(text: str = "") -> None:
    lines.append(text)
    print(text)


def error(text: str) -> None:
    errors.append(text)
    say(f"ERROR: {text}")


def git(*arguments: str) -> str:
    result = subprocess.run(["git", *arguments], cwd=ROOT, capture_output=True, encoding="utf-8", errors="replace")
    return result.stdout


def text_of(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace").replace("\r\n", "\n")


def added_rows(path: str, base: str) -> tuple[list[str], list[str]]:
    """The rows of a file of symbols/ in the commit, and those added at its end."""
    old = git("show", f"{base}:{path}").replace("\r\n", "\n")
    new = text_of(ROOT / path)
    if not new.startswith(old.rstrip("\n")):
        old_lines, new_lines = old.split("\n"), new.split("\n")
        for n, (a, b) in enumerate(zip(old_lines, new_lines)):
            if a != b:
                error(f"{path}, línea {n + 1}: una fila que ya estaba se cambió o se borró ({a!r} -> {b!r}). Solo se pueden AÑADIR filas al final: deshaz con `git checkout {path}` y añade otra vez")
                break
        else:
            error(f"{path}: se borraron filas del final")
        return old_lines, []
    return old.split("\n"), [row for row in new[len(old.rstrip("\n")) :].split("\n") if row.strip()]


def disassembly_returns(address: str) -> list[int]:
    """What each ret of the game's function takes off the stack."""
    result = subprocess.run(
        [sys.executable, str(ROOT / "tools" / "disassemble.py"), "0x" + address], cwd=ROOT, capture_output=True, encoding="utf-8", errors="replace"
    )
    taken = []
    for line in result.stdout.split("\n"):
        if line.startswith("data"):
            break
        match = re.search(r"\sret\s*(0x[0-9a-f]+|\d+)?\s*$", line)
        if match:
            taken.append(int(match.group(1), 0) if match.group(1) else 0)
    return taken


def check_functions(rows: list[str], old: list[str]) -> list[str]:
    game = {r["address"] for r in csv.DictReader(open(ROOT / "ghidra" / "functions.csv", newline="")) if r["section"] == ".text"}
    old_addresses = {row.split(",")[0] for row in old if row}
    old_names = {row.split(",")[1] for row in old if row.count(",") >= 1}
    names = []
    for row in rows:
        match = FUNCTION_ROW.match(row)
        if not match:
            error(
                f"symbols/functions.csv: la fila {row!r} no tiene el formato "
                "`dirección,nombre,source/archivo.c,argumentos,retorno[,stdcall]` "
                "(dirección de 8 cifras en minúscula sin 0x; argumentos como `eax ecx stack`, no números)"
            )
            continue
        address, name, source, arguments, returns, convention = match.groups()
        convention = convention or ""
        names.append(name)
        if address in old_addresses:
            error(f"symbols/functions.csv: la dirección {address} ya tenía una fila: no se añade otra, se completa la que hay")
        if name in old_names:
            error(f"symbols/functions.csv: el nombre {name} ya existe")
        if address not in game:
            error(f"symbols/functions.csv: {address} no es el principio de una función de ghidra/functions.csv")
        for argument in arguments.split():
            if argument not in ARGUMENTS:
                error(f"symbols/functions.csv: {name}: argumento {argument!r} desconocido (vale: {', '.join(sorted(ARGUMENTS))})")
        if returns not in RETURNS:
            error(f"symbols/functions.csv: {name}: el retorno {returns!r} no lo compara el arnés; usa uno de {', '.join(sorted(RETURNS))}")
        if convention not in CONVENTIONS:
            error(f"symbols/functions.csv: {name}: convención {convention!r} desconocida (vale: stdcall, thiscall o nada)")
        path = ROOT / source
        if not path.exists():
            error(f"{name}: {source} no existe")
        elif not re.search(rf"\b{name}\s*\(", text_of(path)):
            error(f"{name}: no está escrita en {source}")
        elif convention == "stdcall" and not re.search(rf"__attribute__\(\(stdcall\)\)\s*\**\s*{name}\s*\(", text_of(path)):
            error(f"{name}: la fila dice stdcall pero en {source} no tiene __attribute__((stdcall))")
        if convention != "thiscall" and address in game:
            stack = arguments.split().count("stack")
            for taken in set(disassembly_returns(address)):
                if taken and (convention != "stdcall" or taken != stack * 4):
                    error(f"{name}: el juego acaba en `ret {taken}`: es stdcall con {taken // 4} argumento(s) de pila (la fila tiene {stack} y convención {convention or 'ninguna'})")
                if not taken and convention == "stdcall" and stack:
                    error(f"{name}: la fila dice stdcall pero el juego acaba en `ret` sin número")
    if len(names) != len(set(names)):
        error("symbols/functions.csv: hay nombres repetidos entre las filas nuevas")
    return names


def check_globals(rows: list[str], old: list[str]) -> None:
    old_addresses = {row.split(",")[0]: row for row in old if row}
    old_names = {row.split(",")[1] for row in old if row.count(",") >= 1}
    for row in rows:
        match = GLOBAL_ROW.match(row)
        if not match:
            error(f"symbols/globals.csv: la fila {row!r} no tiene el formato `dirección,nombre,source/archivo.c` (8 cifras sin 0x; el tercer campo es el archivo .c, no un tipo)")
            continue
        address, name, source = match.groups()
        if address in old_addresses:
            error(f"symbols/globals.csv: {address} ya existe como {old_addresses[address]!r}: usa esa variable, no crees otra")
        if name in old_names:
            error(f"symbols/globals.csv: el nombre {name} ya existe")
        path = ROOT / source
        if not path.exists():
            error(f"{name}: {source} no existe")
        elif not re.search(rf"\b{name}\s*(\[|;|=)", text_of(path)):
            error(f"{name}: no está declarada en {source}")


def array_globals() -> set[str]:
    names = set()
    for row in csv.DictReader(open(ROOT / "symbols" / "globals.csv", encoding="utf-8", newline="")):
        path = ROOT / row["source"]
        if row["source"] and path.exists() and re.search(rf"^[^/*\n(]*\b{row['name']}\s*\[", text_of(path), re.M):
            names.add(row["name"])
    return names


def check_tests(milestone: str, names: list[str]) -> None:
    test = ROOT / "tests" / f"test_small_{milestone}.py"
    if not test.exists():
        error(f"falta {test.relative_to(ROOT)} (las pruebas del hito; copia la forma de tests/test_small_741.py)")
        return
    text = text_of(test)
    if "def run(t)" not in text:
        error(f"{test.name}: no tiene `def run(t) -> None:`; las pruebas del arnés se escriben así, con t.check(...) (mira tests/test_small_741.py)")
    if re.search(r"^\s*(from|import)\s+tools\b", text, re.M):
        error(f"{test.name}: importa de `tools`; se importa de `difftest` (from difftest import Buffer, GlobalData)")
    for name in names:
        if f'"{name}"' not in text:
            error(f"{test.name}: no prueba {name} (no hay t.check(\"{name}\", ...))")
    arrays = array_globals()
    buffers = set(re.findall(r"\b(\w+)\s*=\s*Buffer\(", text))
    for key, value in re.findall(r'"(\w+)"\s*:\s*(Buffer\(|\w+)', text):
        if key in arrays and (value == "Buffer(" or value in buffers):
            error(f"{test.name}: el global {key} es una tabla en C y se da como Buffer: así solo se escribe un puntero y la prueba no compara la tabla. Dalo como GlobalData(bytes) o como bytes")


def check_mutations(milestone: str) -> None:
    path = ROOT / "tests" / f"mutations_{milestone}.py"
    if not path.exists():
        error(f"falta {path.relative_to(ROOT)} (una o dos mutaciones por función; copia la forma de tests/mutations_741.py)")
        return
    try:
        spec = runpy.run_path(str(path))
    except Exception as exception:
        error(f"{path.name} no se puede leer: {exception}")
        return
    if spec.get("TESTS") != f"test_small_{milestone}":
        error(f"{path.name}: TESTS tiene que ser \"test_small_{milestone}\"")
    for mutation in spec.get("MUTATIONS", []):
        file, before = mutation[0], mutation[1]
        count = text_of(ROOT / file).count(before) if (ROOT / file).exists() else 0
        if count != 1:
            error(f"{path.name}: el texto {before[:60]!r} está {count} veces en {file}; tiene que estar exactamente una")


def check_source(base: str) -> None:
    changed = git("diff", "-U0", base, "--", "source").split("\n")
    added = [line[1:] for line in changed if line.startswith("+") and not line.startswith("+++")]
    for path in git("ls-files", "--others", "--exclude-standard", "source").split():
        added += text_of(ROOT / path).split("\n")
    for line in added:
        if re.search(r"\b(__asm__|__asm|asm)\s*(volatile\s*)?\(", line):
            error(f"source/: ensamblador en el C ({line.strip()!r}); el C tiene que ser C normal")
        if re.search(r'#include\s+"\.\./', line):
            error(f"source/: include con ../ ({line.strip()!r}); se incluye desde source/, como \"cseries/cseries.h\"")


def run(milestone: str) -> None:
    environment = dict(os.environ)
    if CLANG.exists():
        environment["PATH"] = str(CLANG) + os.pathsep + environment.get("PATH", "")
    else:
        error(f"no está el clang 19.1.5 en {CLANG}")
    for title, command, bad in (
        ("pruebas", [sys.executable, "tools/test_functions.py", f"test_small_{milestone}"], r"FAILED|does not compile|Traceback|Error"),
        ("mutaciones", [sys.executable, "tools/mutation_check.py", f"tests/mutations_{milestone}.py"], r"MISSED|Traceback|not once|times, not"),
    ):
        say(f"---------- {title}: {' '.join(command[1:])}")
        result = subprocess.run(command, cwd=ROOT, env=environment, capture_output=True, encoding="utf-8", errors="replace")
        output = (result.stdout + result.stderr).strip()
        say(output)
        if result.returncode or re.search(bad, output):
            error(f"{title}: algo falló (mira arriba)")
        if title == "mutaciones" and not re.search(r"missed: 0 of \d+", output):
            error("mutaciones: alguna sobrevivió o no se ejecutaron: la prueba no detecta esa línea rota; mejora la prueba")


def main() -> None:
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    arguments = sys.argv[1:]
    if not arguments or not arguments[0].isdigit():
        print(__doc__ or "check_milestone.py <milestone> [--run] [--base <commit>]")
        sys.exit(2)
    milestone = arguments[0]
    base = arguments[arguments.index("--base") + 1] if "--base" in arguments else "HEAD"

    say(f"hito {milestone}, comparado con {base}")
    old, rows = added_rows("symbols/functions.csv", base)
    say(f"symbols/functions.csv: {len(rows)} fila(s) nueva(s)")
    names = check_functions(rows, old)
    old, rows = added_rows("symbols/globals.csv", base)
    say(f"symbols/globals.csv: {len(rows)} fila(s) nueva(s)")
    check_globals(rows, old)
    if not names and not errors:
        error("no hay funciones nuevas en symbols/functions.csv")
    check_source(base)
    check_tests(milestone, names)
    check_mutations(milestone)
    if "--run" in arguments:
        run(milestone)

    say()
    say("TODO BIEN: se puede hacer el commit" if not errors else f"{len(errors)} ERROR(ES): corrígelos antes del commit")
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    sys.exit(1 if errors else 0)


if __name__ == "__main__":
    main()
