#!/usr/bin/env python3
"""Check comparison semantics against Python's exact integer oracle."""
import operator
from pathlib import Path
import subprocess
import sys
import tempfile

OPS = [("==", operator.eq), ("!=", operator.ne), ("<", operator.lt),
       ("<=", operator.le), (">", operator.gt), (">=", operator.ge)]
PAIRS = [
    (0, 0), (1, -1), (-1, 1),
    (2**53 - 1, 2**53), (2**53, 2**53 + 1),
    (2**53 + 1, 2**53), (2**53 + 1, 2**53 + 1),
    (-2**53 - 1, -2**53), (-2**53, -2**53 - 1),
    (2**63 - 1, 2**63 - 2), (2**63 - 2, 2**63 - 1),
    (-2**63, -2**63 + 1), (-2**63 + 1, -2**63),
    (-2**63, -2**63), (2**63 - 1, 2**63 - 1),
    (-2**63, 2**63 - 1),
]

def literal(value):
    # INT64_MIN is expressed without a positive out-of-range literal.
    return "(-9223372036854775807 - 1)" if value == -2**63 else str(value)

def main():
    binary = Path(sys.argv[1]).resolve()
    expressions, expected = [], []
    for a, b in PAIRS:
        for token, compare in OPS:
            expressions.append(f"{literal(a)} {token} {literal(b)}")
            expected.append(str(compare(a, b)).lower())
    # Native v0.1 promotes mixed int/real operands to binary64.
    for a, b in [(2**53 + 1, float(2**53)), (3, 3.5), (-4.5, -4), (1.5, 2.5)]:
        for token, compare in OPS:
            expressions.append(f"{a} {token} {b}")
            expected.append(str(compare(float(a), float(b))).lower())
    for expression, result in [
        ('true == true', True), ('false != true', True),
        ('"nova" == "nova"', True), ('"nova" != "eve"', True),
        ('"வணக்கம்" == "வணக்கம்"', True),
    ]:
        expressions.append(expression)
        expected.append(str(result).lower())

    def command(*args):
        p = subprocess.run([str(binary), *map(str, args)],
                           capture_output=True, text=True, timeout=20)
        if p.returncode:
            raise RuntimeError(f"{args[0]} exited {p.returncode}: {p.stderr.strip()}")
        return p.stdout

    def verify(label, output):
        got = output.splitlines()
        if got != expected:
            mismatches = [(expressions[i], want, got[i] if i < len(got) else "<missing>")
                          for i, want in enumerate(expected)
                          if i >= len(got) or got[i] != want]
            raise AssertionError(f"{label}: got {len(got)} lines, expected {len(expected)}; "
                                 f"first mismatches: {mismatches[:6]}")
        print(f"PASS comparisons {label}: {len(expected)}")

    with tempfile.TemporaryDirectory(prefix="qnova-comparisons-") as directory:
        root = Path(directory)
        source = root / "comparisons.qnova"
        source.write_text("".join(f"emit {e}\n" for e in expressions), encoding="utf-8")
        verify("source", command("run", source))
        first, second = root / "first.qbc", root / "second.qbc"
        command("compile", source, "-o", first)
        command("compile", source, "-o", second)
        if first.read_bytes() != second.read_bytes():
            raise AssertionError("comparison bytecode is not deterministic")
        source.unlink()
        verify("bytecode-without-source", command("run-qbc", first))
        print("PASS comparisons deterministic bytecode")

if __name__ == "__main__":
    try:
        main()
    except (AssertionError, RuntimeError, subprocess.TimeoutExpired) as exc:
        print(f"FAIL comparisons: {exc}", file=sys.stderr)
        sys.exit(1)
