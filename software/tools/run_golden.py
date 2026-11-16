#!/usr/bin/env python3
"""Ekran karşılaştırma testi.

Simülatörü penceresiz modda bir tuş betiğiyle çalıştırır; betiğin ürettiği
görüntüleri kayıtlı (golden) görüntülerle piksel piksel karşılaştırır.

Kullanım:
  run_golden.py SIM BETIK GOLDEN_KLASORU CIKTI_KLASORU [--update]

Betikteki `shot` komutları dosya adını yalın yazar (ör. `shot keytest.png`);
simülatör çıktı klasöründe çalıştırıldığı için görüntüler oraya düşer.
--update: çıktıları golden klasörüne kopyalar (görüntüyü gözle doğruladıktan sonra).
"""

import pathlib
import shutil
import subprocess
import sys

from PIL import Image, ImageChops


def main():
    args = [a for a in sys.argv[1:] if a != "--update"]
    update = "--update" in sys.argv[1:]
    if len(args) != 4:
        sys.exit(__doc__)
    sim, script, golden_dir, out_dir = (pathlib.Path(a).resolve() for a in args)

    if out_dir.exists():
        shutil.rmtree(out_dir)
    out_dir.mkdir(parents=True)

    result = subprocess.run([str(sim), "--headless", "--script", str(script)], cwd=out_dir)
    if result.returncode != 0:
        sys.exit(f"simülatör hata ile çıktı: {result.returncode}")

    shots = sorted(out_dir.glob("*.png"))
    if not shots:
        sys.exit("betik hiç görüntü üretmedi")

    if update:
        golden_dir.mkdir(parents=True, exist_ok=True)
        for shot in shots:
            # Simülatör sıkıştırmasız PNG yazar; depoya sıkıştırılmış hali girer.
            Image.open(shot).save(golden_dir / shot.name, optimize=True)
            print(f"güncellendi: {golden_dir / shot.name}")
        return

    failed = False
    for shot in shots:
        golden = golden_dir / shot.name
        if not golden.exists():
            print(f"HATA: golden yok: {golden}")
            failed = True
            continue
        a = Image.open(shot).convert("RGB")
        b = Image.open(golden).convert("RGB")
        if a.size != b.size or ImageChops.difference(a, b).getbbox() is not None:
            print(f"HATA: farklı: {shot.name} (çıktı: {shot})")
            failed = True
        else:
            print(f"aynı: {shot.name}")
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
