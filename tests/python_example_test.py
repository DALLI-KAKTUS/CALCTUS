#!/usr/bin/env python3
"""Örnek .py dosyasını sanal microSD'den Python uygulamasında çalıştır."""

import pathlib
import shutil
import subprocess
import sys
import tempfile

from PIL import Image, ImageChops


def main():
    sim = pathlib.Path(sys.argv[1]).resolve()
    source = pathlib.Path(sys.argv[2]).resolve()
    golden = pathlib.Path(sys.argv[3]).resolve()
    update = "--update" in sys.argv[4:]
    with tempfile.TemporaryDirectory() as directory:
        folder = pathlib.Path(directory)
        sd = folder / ".calctus-sd"
        sd.mkdir()
        shutil.copy2(source, sd / "calctus_demo.py")
        steps = folder / "python_demo.keys"
        steps.write_text(
            "key RIGHT\nkey RIGHT\nkey OK\nwait 100\nkey F1\nwait 100\nshot python_demo.png\n",
            encoding="ascii")
        subprocess.run([str(sim), "--headless", "--script", str(steps)],
                       cwd=folder, check=True)
        image = folder / "python_demo.png"
        if update:
            golden.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(image, golden)
            print("güncellendi:", golden)
            return
        if not golden.exists():
            raise SystemExit(f"golden yok: {golden}")
        with Image.open(golden) as expected, Image.open(image) as actual:
            if expected.size != actual.size or ImageChops.difference(
                    expected.convert("RGB"), actual.convert("RGB")).getbbox():
                raise SystemExit("Python örnek çıktısı ekran görüntüsü farklı")
    print("Python örnek: microSD'den gerçek VM'de 1..5 kareleri çalıştırıldı")


if __name__ == "__main__":
    main()
