#!/usr/bin/env python3
"""Sabit MicroPython embed kaynak ve qstr başlıklarını iki hedef için üret.

Kullanım: python3 tools/vendor_micropython.py /yol/micropython-v1.25.0
Derleme sırasında internet ya da kaynak MicroPython deposu gerekmez.
"""

import pathlib
import shutil
import subprocess
import sys
import tempfile


ROOT = pathlib.Path(__file__).resolve().parent.parent
COMMIT = "f498a16c7db6d4b2de200b3e0856528dfe0613c3"


def main():
    if len(sys.argv) != 2:
        raise SystemExit(__doc__)
    source = pathlib.Path(sys.argv[1]).resolve()
    revision = subprocess.check_output(
        ["git", "rev-parse", "HEAD"], cwd=source, text=True).strip()
    if revision != COMMIT:
        raise SystemExit(f"Beklenen MicroPython commit'i {COMMIT}, bulunan {revision}")
    with tempfile.TemporaryDirectory(prefix="calctus-mpy-") as directory:
        stage = pathlib.Path(directory)
        shutil.copy2(ROOT / "services/micropython/mpconfigport.h", stage / "mpconfigport.h")
        subprocess.run(
            ["make", "-B", "-f", str(source / "ports/embed/embed.mk"),
             f"MICROPYTHON_TOP={source}", "-j4"], cwd=stage, check=True)
        destination = ROOT / "third_party/micropython_embed"
        shutil.copytree(stage / "micropython_embed", destination, dirs_exist_ok=True)
        shutil.copy2(source / "LICENSE", destination / "LICENSE")
    print("MicroPython v1.25.0 embed paketi yenilendi")


if __name__ == "__main__":
    main()
