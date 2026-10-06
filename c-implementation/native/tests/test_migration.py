#!/usr/bin/env python3
"""Small explicit migrations; historical dialects are never auto-translated."""
from pathlib import Path
import subprocess
import sys
import tempfile

def main():
    binary = str(Path(sys.argv[1]).resolve())
    with tempfile.TemporaryDirectory(prefix="qnova-migration-") as directory:
        root = Path(directory)
        for bit in (0, 1):
            code = f"qbit q = |{bit}>\nmeasure q -> result\nemit result\n"
            for suffix in ("qnova", "nova"):
                source = root / f"measure-{bit}.{suffix}"
                source.write_text(code)
                p = subprocess.run([binary, "run", str(source), "--rng", "test:1"],
                                   capture_output=True, text=True, timeout=10)
                assert p.returncode == 0 and p.stdout.strip() == str(bit), (source, p.stderr)
        rejected = [
            'nova universal_dragon\nsay "legacy"\n',
            'brain universal_dragon\nintent "legacy contract"\n',
            'patch "file.txt" {\n replace "old" => "new"\n}\n',
            'qbit q = |0>\nwhen q == 0:\n say "legacy branch"\n',
        ]
        for index, code in enumerate(rejected):
            source = root / f"legacy-{index}.ud"
            source.write_text(code)
            p = subprocess.run([binary, "check", str(source)],
                               capture_output=True, text=True, timeout=10)
            assert p.returncode != 0 and "QNOVA-E" in p.stderr, (source, p.stdout, p.stderr)
        print("PASS migration: 4 explicit measurement/extension checks; 4 legacy rejections")

if __name__ == "__main__":
    main()
