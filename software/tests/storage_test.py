#!/usr/bin/env python3
"""HAL ayar kaydı açılışlar arasında sürer, bozuk kayıt güvenle atılır."""

import os
import pathlib
import subprocess
import sys
import tempfile

from PIL import Image


def run(sim, folder, script):
    path = folder / "steps.keys"
    path.write_text(script, encoding="utf-8")
    subprocess.run([str(sim), "--headless", "--script", str(path)], cwd=folder, check=True)


def pixel(path):
    return Image.open(path).convert("RGB").getpixel((0, 100))


def main():
    sim = pathlib.Path(sys.argv[1]).resolve()
    with tempfile.TemporaryDirectory() as name:
        folder = pathlib.Path(name)
        run(sim, folder, "shot light.png\n")
        light = pixel(folder / "light.png")
        run(sim, folder,
            "key DOWN\nkey DOWN\nkey RIGHT\nkey RIGHT\nkey RIGHT\n"
            "key OK\nkey OK\nkey DOWN\nkey OK\n")
        data = (folder / ".calctus-settings.bin").read_bytes()
        assert len(data) == 6 and data[1] == 1, data
        # Okunabilen kayıt ve güç tuşuyla ekran döngüsü NVS'yi yeniden yazmamalı.
        settings = folder / ".calctus-settings.bin"
        os.utime(settings, ns=(1_000_000_000, 1_000_000_000))
        run(sim, folder, "shot dark.png\n")
        assert settings.stat().st_mtime_ns == 1_000_000_000
        run(sim, folder, "key POWER\nkey POWER\n")
        assert settings.stat().st_mtime_ns == 1_000_000_000
        dark = pixel(folder / "dark.png")
        assert light != dark, (light, dark)
        (folder / ".calctus-settings.bin").write_bytes(b"corrupt")
        run(sim, folder, "shot recovered.png\n")
        assert pixel(folder / "recovered.png") == light
    print("kalıcı ayar: yazma, yeniden açma, bozuk kayıttan kurtarma geçti")


if __name__ == "__main__":
    main()
