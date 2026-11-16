#!/usr/bin/env python3
"""SD'deki .py yürütme ve hata ekranı; golden görüntülerle karşılaştır."""

import pathlib
import shutil
import subprocess
import sys
import tempfile

from PIL import Image, ImageChops


SCRIPT = "key RIGHT\nkey RIGHT\nkey OK\nshot 01_dosya.png\nkey F1\nshot 02_sonuc.png\n"


def launch(sim, folder, script):
    steps = folder / "steps.keys"
    steps.write_text(script, encoding="utf-8")
    subprocess.run([str(sim), "--headless", "--script", str(steps)],
                   cwd=folder, check=True)


def main():
    sim = pathlib.Path(sys.argv[1]).resolve()
    golden = pathlib.Path(sys.argv[2]).resolve()
    update = "--update" in sys.argv[3:]
    preview = "--preview" in sys.argv[3:]
    with tempfile.TemporaryDirectory() as directory:
        folder = pathlib.Path(directory)
        sd = folder / ".calctus-sd"
        sd.mkdir()
        source = sd / "demo.py"
        (sd / "helper.py").write_text("def square(n):\n    return n * n\n",
                                      encoding="ascii")
        source.write_text(
            "import helper\n"
            "result = helper.square(7)\n"
            "print(result)\n"
            "with open('result.txt', 'w') as f:\n"
            "    f.write(str(result))\n", encoding="ascii")
        launch(sim, folder, SCRIPT)
        assert (sd / "result.txt").read_bytes() == b"49"
        source.write_text("raise ValueError('SD error')\n", encoding="ascii")
        launch(sim, folder, SCRIPT.replace("01_dosya.png", "03_hata_liste.png")
               .replace("02_sonuc.png", "04_hata_sonuc.png"))
        names = ["01_dosya.png", "02_sonuc.png", "03_hata_liste.png", "04_hata_sonuc.png"]
        if preview:
            target = sim.parent / "golden-out/python_files"
            target.mkdir(parents=True, exist_ok=True)
            for name in names:
                shutil.copy2(folder / name, target / name)
                print("önizleme:", target / name)
            return
        if update:
            golden.mkdir(parents=True, exist_ok=True)
            for name in names:
                shutil.copy2(folder / name, golden / name)
                print("güncellendi:", golden / name)
            return
        for name in names:
            expected = golden / name
            if not expected.exists():
                raise SystemExit(f"golden yok: {expected}")
            with Image.open(expected) as old, Image.open(folder / name) as new:
                if old.size != new.size or ImageChops.difference(old.convert("RGB"),
                                                                 new.convert("RGB")).getbbox():
                    raise SystemExit(f"farklı: {name}")
        print("Python microSD: helper import, 49 sonucu ve ValueError ekranları geçti")


if __name__ == "__main__":
    main()
