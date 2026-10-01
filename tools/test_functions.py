#!/usr/bin/env python3

# checks every function of source/ against the function of the game's
# executable it was decompiled from: runs both in an emulator with the cases
# of tests/ and compares their results (tools/difftest.py)
#
# test_functions.py [name of a test file]
#
# needs disc/default.xbe, clang and ld.lld in the PATH, and the Python package
# unicorn

import csv
import importlib.util
import sys
from pathlib import Path

from difftest import ROOT, Buffer, DifftestError, FunctionPointer, Harness, Pointer, almost_equal

TESTS = ROOT / "tests"
GHIDRA_FUNCTIONS = ROOT / "ghidra" / "functions.csv"


class Tester:
    """What a test file gets: check() runs a case in both functions."""

    def __init__(self, harness: Harness) -> None:
        self.harness = harness
        # per function: the cases run and the first failures
        self.cases: dict[str, int] = {}
        self.failures: dict[str, list[str]] = {}

    def check(self, name: str, *arguments: object, globals: dict[str, object] | None = None) -> None:
        """Runs both functions with the arguments and compares what they return and
        what they leave in the buffers (difftest.Buffer) of the arguments.

        `globals` gives global variables of symbols/globals.csv a value for
        the call: an integer, or a buffer that the variable points to, which
        is compared too."""
        self.cases[name] = self.cases.get(name, 0) + 1
        shown = ", ".join(
            f"<{len(argument.data)} bytes>"
            if isinstance(argument, Buffer)
            else "<pointer>"
            if isinstance(argument, (Pointer, FunctionPointer))
            else repr(argument)
            for argument in arguments
        )
        try:
            original = self.harness.original(name, *arguments, globals=globals)
            ours = self.harness.ours(name, *arguments, globals=globals)
        except DifftestError as error:
            self._fail(name, f"({shown}): {error}")
            return
        if not almost_equal(original.value, ours.value):
            self._fail(name, f"({shown}): the game returns {original.value!r}, source/ returns {ours.value!r}")
        elif not almost_equal(original.buffers, ours.buffers):
            self._fail(name, f"({shown}): the game and source/ leave different data in a buffer{self._difference(original.buffers, ours.buffers)}")

    @staticmethod
    def _difference(original: list, ours: list) -> str:
        """Where two sets of buffers first differ."""
        for number, (a, b) in enumerate(zip(original, ours)):
            for at, (x, y) in enumerate(zip(a, b)):
                if not almost_equal(x, y):
                    place = f"value {at}" if isinstance(a, tuple) else f"offset {at:#x}"
                    return f": buffer {number}, {place}: {x!r} and {y!r}"
        return ""

    def _fail(self, name: str, message: str) -> None:
        self.failures.setdefault(name, [])
        if len(self.failures[name]) < 3:
            self.failures[name].append(message)


def main() -> None:
    try:
        harness = Harness()
    except DifftestError as error:
        sys.exit(f"test_functions: {error}")
    tester = Tester(harness)

    wanted = sys.argv[1] if len(sys.argv) > 1 else None
    # a test file can take what another one has
    sys.path.insert(0, str(TESTS))
    for path in sorted(TESTS.glob("test_*.py")):
        if wanted and wanted not in path.stem:
            continue
        spec = importlib.util.spec_from_file_location(path.stem, path)
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        module.run(tester)

    failed = 0
    for name, symbol in sorted(harness.symbols.items(), key=lambda item: item[1].address):
        cases = tester.cases.get(name, 0)
        failures = tester.failures.get(name, [])
        status = "FAILED" if failures else "ok" if cases else "no test"
        failed += bool(failures)
        if not wanted or cases:
            print(f"{symbol.address:08x}  {name:<44} {cases:>5} cases  {status}")
        for failure in failures:
            print(f"          {failure}")

    tested = sum(1 for name in harness.symbols if tester.cases.get(name) and name not in tester.failures)
    if wanted:
        # only some tests were run: no totals
        sys.exit(1 if failed else 0)
    print(f"\n{len(harness.symbols)} functions decompiled, {tested} of them checked against the game")
    if GHIDRA_FUNCTIONS.exists():
        with GHIDRA_FUNCTIONS.open(newline="") as f:
            game = {row["address"]: int(row["size"]) for row in csv.DictReader(f) if row["section"] == ".text"}
        size = sum(game.get(f"{symbol.address:08x}", 0) for symbol in harness.symbols.values())
        print(
            f"of the game's code: {len(harness.symbols) / len(game):.2%} of {len(game)} functions, "
            f"{size / sum(game.values()):.3%} of {sum(game.values()):,} bytes"
        )
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
