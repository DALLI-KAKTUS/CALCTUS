#!/usr/bin/env python3
"""Cihaz tuşlarıyla Python kodu ve simge girişi; ekran+SD doğrulaması."""

import pathlib
import subprocess
import sys
import tempfile

from PIL import Image, ImageChops


def main():
    sim = pathlib.Path(sys.argv[1]).resolve()
    script = pathlib.Path(sys.argv[2]).resolve()
    expected = pathlib.Path(sys.argv[3]).resolve()
    with tempfile.TemporaryDirectory() as directory:
        folder = pathlib.Path(directory)
        subprocess.run([str(sim), "--headless", "--script", str(script)],
                       cwd=folder, check=True)
        sd = folder / ".calctus-sd"
        assert (sd / "a.py").read_bytes() == b"print(2+2)"
        assert (sd / "not1.txt").read_bytes() == b":"
        with Image.open(expected) as old, Image.open(folder / "03_sonuc.png") as new:
            assert old.size == new.size
            assert ImageChops.difference(old.convert("RGB"),
                                         new.convert("RGB")).getbbox() is None
    print("Python klavye: .py, FN iki nokta, microSD ve 4 çıktısı geçti")


if __name__ == "__main__":
    main()
