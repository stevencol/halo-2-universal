#!/usr/bin/env python3

# writes the file of mutations of a milestone for mutation_check.py: for each
# function added to symbols/functions.csv since a commit, it breaks a few of
# its lines (a comparison turned around, a constant one more, an && made ||,
# an assignment taken out)
#
# make_mutations.py <milestone> --base <commit> [--tests <name>] [--each <n>] [--out <file>]
#
# The file is written as tests/mutations_<milestone>.py. Mutations of the
# kind only a person thinks of (the size of an entry, a loop one short) can
# be added to it by hand afterwards; one that no test can notice (it does not
# change what the function does) is taken out of the list by hand, with a
# comment saying why.

import argparse
import random
import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

OPERATORS = re.compile(r"->|<<=|>>=|<<|>>|==|!=|<=|>=|&&|\|\||<|>")
TURNED = {"==": "!=", "!=": "==", "<=": "<", ">=": ">", "<": "<=", ">": ">=", "&&": "||", "||": "&&"}
CONSTANT = re.compile(r"\b(0x[0-9A-Fa-f]+|\d+)\b(?![.\w])")
WORDS = {"TRUE": "FALSE", "FALSE": "TRUE", "NONE": "0"}


def added_functions(base: str) -> list[tuple[str, str]]:
    diff = subprocess.run(["git", "diff", base, "--", "symbols/functions.csv"], cwd=ROOT, capture_output=True, text=True, check=True).stdout
    rows = [line[1:].split(",") for line in diff.splitlines() if line.startswith("+") and not line.startswith("+++")]
    return [(row[1], row[2]) for row in rows if len(row) > 2 and row[2]]


def added_tests(base: str) -> str:
    names = subprocess.run(["git", "diff", "--name-only", "--diff-filter=A", base, "--", "tests"], cwd=ROOT, capture_output=True, text=True, check=True).stdout
    tests = [Path(name).stem for name in names.split() if Path(name).name.startswith("test_")]
    if len(tests) != 1:
        raise SystemExit(f"tests added since the base: {tests}; say which with --tests")
    return tests[0]


def body_lines(lines: list[str], name: str) -> range | None:
    # the definition: the name at the start of a line or after its type, then { on a later line
    head = re.compile(rf"^\w[^;=]*\b{re.escape(name)}\s*\(")
    for index, line in enumerate(lines):
        if head.match(line) and not line.rstrip().endswith(";"):
            start = next((i for i in range(index, min(index + 12, len(lines))) if lines[i].startswith("{")), None)
            if start is None:
                continue
            end = next(i for i in range(start, len(lines)) if lines[i].startswith("}"))
            return range(start + 1, end)
    return None


def candidates(line: str) -> list[str]:
    code = line.split("//")[0]
    if code.strip().startswith(("#", "/*", "*")) or '"' in code:
        return []
    found = []
    for match in OPERATORS.finditer(code):
        if match.group() == "<" and code.lstrip().startswith("for"):
            # one more turn would only touch what is after the array: one less instead
            bound = re.match(r"([^;]+);", code[match.end() :])
            if bound:
                at = match.end() + bound.end(1)
                found.append(code[:at] + "-1" + code[at:])
        elif match.group() in TURNED:
            found.append(code[: match.start()] + TURNED[match.group()] + code[match.end() :])
    for match in CONSTANT.finditer(code):
        text = match.group()
        value = int(text, 0) + 1
        found.append(code[: match.start()] + (hex(value) if text.startswith("0x") else str(value)) + code[match.end() :])
    for word, other in WORDS.items():
        for match in re.finditer(rf"\b{word}\b", code):
            found.append(code[: match.start()] + other + code[match.end() :])
    # an assignment, not a declaration (taken out, it would not compile)
    left = code.split("=")[0]
    declaration = re.match(r"^\s*(const\s+|unsigned\s+|struct\s+)*\w+[\s*]+\w+(\[\w*\])?\s*$", left)
    if re.search(r"[^=!<>]=[^=]", code) and code.rstrip().endswith(";") and not declaration and not left.strip().startswith("for"):
        found.append("")
    return [new + line[len(code) :] for new in found]


def unique_text(text: str, lines: list[str], index: int, new_line: str) -> tuple[str, str]:
    # the line, with lines around it until it is in the file only once
    first, last = index, index
    while True:
        old = "\n".join(lines[first : last + 1])
        if text.count(old) == 1:
            new = "\n".join(lines[first:index] + ([new_line] if new_line else []) + lines[index + 1 : last + 1])
            return old, new
        if first > 0:
            first -= 1
        if last < len(lines) - 1:
            last += 1


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("milestone", type=int)
    parser.add_argument("--base", required=True)
    parser.add_argument("--tests")
    parser.add_argument("--each", type=int, default=4, help="mutations for each function")
    parser.add_argument("--out", help="where to write it (to not cover a list written by hand)")
    arguments = parser.parse_args()

    random_ = random.Random(arguments.milestone)
    tests = arguments.tests or added_tests(arguments.base)
    files: dict[str, str] = {}
    mutations: list[tuple[str, str, str]] = []
    for name, path in added_functions(arguments.base):
        text = (ROOT / path).read_text(encoding="utf-8").replace("\r\n", "\n")
        lines = text.split("\n")
        body = body_lines(lines, name)
        if body is None:
            print(f"not found: {name} in {path}")
            continue
        options = [(index, new) for index in body for new in candidates(lines[index])]
        # one for each line before a second one in any
        random_.shuffle(options)
        seen: dict[int, int] = {}
        rank = []
        for index, _ in options:
            seen[index] = seen.get(index, 0) + 1
            rank.append(seen[index])
        options = [option for _, option in sorted(zip(rank, options), key=lambda pair: pair[0])]
        chosen = 0
        for index, new_line in options:
            if chosen == arguments.each:
                break
            old, new = unique_text(text, lines, index, new_line)
            if (path, old) in {(p, o) for p, o, _ in mutations}:
                continue
            mutations.append((path, old, new))
            files.setdefault(path, path.replace("/", "_").replace(".", "_").replace(" ", "_").upper())
            chosen += 1
        if not chosen:
            print(f"nothing to break in {name}")

    out = [f"# mutations of milestone {arguments.milestone}, written by make_mutations.py", "", f'TESTS = "{tests}"', ""]
    out += [f'{variable} = "{path}"' for path, variable in files.items()]
    out += ["", "MUTATIONS = ["]
    out += [f"    ({files[path]}, {old!r}, {new!r})," for path, old, new in mutations]
    out += ["]", ""]
    target = Path(arguments.out) if arguments.out else ROOT / "tests" / f"mutations_{arguments.milestone}.py"
    target.write_text("\n".join(out), encoding="utf-8", newline="\n")
    print(f"{target.name}: {len(mutations)} mutations of {len(added_functions(arguments.base))} functions")


if __name__ == "__main__":
    main()
