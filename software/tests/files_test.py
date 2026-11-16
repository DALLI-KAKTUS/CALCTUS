#!/usr/bin/env python3
"""Sanal microSD: oluşturma, kalıcılık, yeniden adlandırma, silme."""

import pathlib
import subprocess
import sys
import tempfile


OPEN_FILES = "key DOWN\nkey DOWN\nkey RIGHT\nkey RIGHT\nkey OK\n"


def run(sim, folder, actions):
    steps = folder / "steps.keys"
    steps.write_text(OPEN_FILES + actions, encoding="utf-8")
    subprocess.run([str(sim), "--headless", "--script", str(steps)],
                   cwd=folder, check=True)


def main():
    sim = pathlib.Path(sys.argv[1]).resolve()
    with tempfile.TemporaryDirectory() as path:
        folder = pathlib.Path(path)
        sd = folder / ".calctus-sd"
        run(sim, folder,
            "key F1\nkey 1\nkey 2\nkey 3\nkey F3\n"
            "key ALPHA\nkey 2\nkey 2\nkey RIGHT\nkey 2\n"
            "key F1\nkey F6\n")
        document = sd / "not1.txt"
        assert document.read_bytes() == b"123\nba", document.read_bytes()
        # Yeni oturumda gerçek dosya okunur; ad değişimi eski dosyayı kaldırır.
        run(sim, folder,
            "key F2\nkey F4\nkey BACKSPACE\nkey BACKSPACE\nkey BACKSPACE\n"
            "key ALPHA\nkey 7\nkey 9\nkey 9\nkey 9\nkey EXE\n"
            "key F1\nkey F6\n")
        renamed = sd / "not1.py"
        assert not document.exists() and renamed.read_bytes() == b"123\nba"
        run(sim, folder, "key F3\nkey F3\n")
        assert not renamed.exists(), "silme işlemi tamamlanmadı"
        print("microSD sim: kaydetme, yeniden açma, ad değiştirme, silme geçti")


if __name__ == "__main__":
    main()
