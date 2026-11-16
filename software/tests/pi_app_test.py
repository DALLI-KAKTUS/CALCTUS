#!/usr/bin/env python3
"""Pi absent/present UI with remote computation; snapshot comparison."""
import os
import pathlib
import shutil
import subprocess
import sys
import tempfile

from PIL import Image, ImageChops


def run(sim, connected, dest, update=False):
    with tempfile.TemporaryDirectory() as directory:
        folder = pathlib.Path(directory)
        script = folder / "steps.keys"
        script.write_text("key DOWN\nkey DOWN\nkey OK\nwait 450\n"
                          "shot absent.png\nkey F2\nkey F3\nshot result.png\n",
                          encoding="ascii")
        env = dict(os.environ)
        env["CALCTUS_PI_SIM"] = "1" if connected else "0"
        subprocess.run([str(sim), "--headless", "--script", str(script)],
                       cwd=folder, check=True, env=env)
        for name in ("absent.png", "result.png"):
            expected = dest / (("present_" if connected else "absent_") + name)
            if update:
                dest.mkdir(parents=True, exist_ok=True)
                shutil.copy2(folder / name, expected)
                continue
            with Image.open(expected) as old, Image.open(folder / name) as new:
                assert old.size == new.size and not ImageChops.difference(
                    old.convert("RGB"), new.convert("RGB")).getbbox(), expected


if __name__ == "__main__":
    sim = pathlib.Path(sys.argv[1]).resolve()
    dest = pathlib.Path(sys.argv[2]).resolve()
    update = "--update" in sys.argv[3:]
    run(sim, False, dest, update)
    run(sim, True, dest, update)
    print("Pi app: absent fallback, attached status and remote 144 UI passed")
