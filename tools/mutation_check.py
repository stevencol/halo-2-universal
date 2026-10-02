#!/usr/bin/env python3

# breaks the code of source/ one line at a time and runs the tests, to see
# that they notice: a test that passes with the line broken does not check
# that line
#
# mutation_check.py <file of mutations>
#
# The file of mutations is Python, and defines:
#
#   TESTS = "game_state"          # the tests to run: a part of the name of the test files
#   MUTATIONS = [
#       ("source/saved games/game_state.c", "size= (size+3)&~3;", "size= (size+3)&~1;"),
#       ...
#   ]
#
# Each mutation is a file, a text that is in it exactly once and what to put
# in its place. The file is put back as it was after each one, also if the
# run is stopped with Ctrl+C. A mutation that makes the code loop without end
# counts as caught: the tests stop at a limit.

import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TIMEOUT_SECONDS = 300


def main() -> None:
    if len(sys.argv) != 2:
        sys.exit("usage: mutation_check.py <file of mutations>")
    settings: dict = {}
    exec(Path(sys.argv[1]).read_text(encoding="utf-8"), settings)
    tests = settings.get("TESTS", "")
    mutations = settings["MUTATIONS"]

    # all the texts have to be there before anything is changed
    for path, old, _ in mutations:
        count = (ROOT / path).read_text(encoding="utf-8").count(old)
        if count != 1:
            sys.exit(f"{path} has this text {count} times, not once:\n{old}")

    missed = 0
    for path, old, new in mutations:
        file = ROOT / path
        original = file.read_text(encoding="utf-8")
        file.write_text(original.replace(old, new), encoding="utf-8", newline="\n")
        try:
            try:
                command = [sys.executable, str(ROOT / "tools" / "test_functions.py"), *([tests] if tests else [])]
                result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=TIMEOUT_SECONDS)
                caught = [line.split()[1] for line in result.stdout.splitlines() if "FAILED" in line]
                if "does not compile" in result.stdout + result.stderr:
                    caught = ["(does not compile)"]
                elif "Traceback" in result.stderr:
                    caught = ["(the test stops: " + result.stderr.strip().splitlines()[-1][:60] + ")"]
            except subprocess.TimeoutExpired:
                caught = [f"(does not end in {TIMEOUT_SECONDS} s)"]
        finally:
            file.write_text(original, encoding="utf-8", newline="\n")
        missed += not caught
        text = " ".join(old.split())[:64]
        print(("caught  " if caught else "MISSED  ") + text, caught[:2], flush=True)

    print(f"missed: {missed} of {len(mutations)}")
    sys.exit(1 if missed else 0)


if __name__ == "__main__":
    main()
