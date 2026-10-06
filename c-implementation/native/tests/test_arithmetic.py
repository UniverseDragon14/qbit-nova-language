#!/usr/bin/env python3
"""Verify signed64 arithmetic boundaries in source and standalone QBC."""
from pathlib import Path
import subprocess
import sys
import tempfile

MAX = "9223372036854775807"
MIN = "(-9223372036854775807 - 1)"
CASES = [
    (f"{MAX} + 1", None), (f"{MIN} - 1", None),
    (f"-({MIN})", None), (f"{MAX} * 2", None),
    (f"{MIN} * -1", None), (f"-1 * {MIN}", None),
    (f"{MIN} / -1", None), ("-3037000500 * -3037000500", None),
    (f"2 * {MIN}", None), (f"{MIN} + -1", None),
    (f"{MAX} - -1", None), (f"-2 * {MAX}", None),
    (f"{MAX} + 0", MAX), (f"{MIN} + 0", str(-2**63)),
    (f"{MIN} % -1", "0"), (f"{MIN} / 1", str(-2**63)),
    (f"{MIN} * 1", str(-2**63)), (f"{MIN} * 0", "0"),
    (f"0 * {MIN}", "0"), (f"{MAX} / -1", str(-(2**63 - 1))),
    ("-7 / 3", "-2"), ("-7 % 3", "-1"),
    ("3037000499 * 3037000499", str(3037000499**2)),
    ("-3037000499 * -3037000499", str(3037000499**2)),
    (f"{MIN} - {MIN}", "0"), (f"{MAX} + {MIN}", "-1"),
]

def main():
    binary = str(Path(sys.argv[1]).resolve())
    count = 0
    def run(*args):
        return subprocess.run([binary, *map(str, args)],
                              capture_output=True, text=True, timeout=10)
    def verify(result, expected, expression, lane):
        if expected is None:
            assert result.returncode != 0 and "QNOVA-E6006 " in result.stderr, (
                expression, lane, result.returncode, result.stdout, result.stderr)
            assert "runtime error:" not in result.stderr and "AddressSanitizer" not in result.stderr
        else:
            assert result.returncode == 0 and result.stdout.strip() == expected, (
                expression, lane, expected, result.stdout, result.stderr)
    with tempfile.TemporaryDirectory(prefix="qnova-arithmetic-") as directory:
        root = Path(directory)
        for index, (expression, expected) in enumerate(CASES):
            source, bytecode = root / f"{index}.qnova", root / f"{index}.qbc"
            source.write_text(f"emit {expression}\n")
            verify(run("run", source), expected, expression, "source")
            compiled = run("compile", source, "-o", bytecode)
            assert compiled.returncode == 0, compiled.stderr
            source.unlink()
            verify(run("run-qbc", bytecode), expected, expression, "bytecode")
            count += 2
    print(f"PASS signed64 arithmetic boundaries: {count}")

if __name__ == "__main__":
    main()
