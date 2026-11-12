#!/usr/bin/env python3
"""Ortak kodun taşınabilirlik denetimi.

Kural: masaüstünde çalışıp ESP32-S3'te çalışmayacak hiçbir şey ortak koda
giremez. Bu betik ortak klasörlerdeki kaynakları tarar; yasaklı bir başlık ya
da dil özelliği görürse hata verir. platform/ ve tools/ taranmaz: onlar
platforma özeldir.
"""

import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
CORE_DIRS = ["hal", "kernel", "ui", "math", "services", "apps"]
SUFFIXES = {".h", ".hpp", ".cpp", ".c", ".def"}

# Başlık -> neden yasak
FORBIDDEN_INCLUDES = {
    r"SDL.*": "SDL yalnız platform/sim içinde kullanılabilir",
    r"iostream|fstream|sstream|iomanip|ostream|istream": "akışlar cihazda çok yer tutar; hal::log kullanın",
    r"filesystem": "dosyalara depolama servisi üzerinden erişilir",
    r"thread|future|mutex|condition_variable|shared_mutex|semaphore": "görevler çekirdek üzerinden açılır",
    r"chrono": "zaman için hal::millis kullanın",
    r"regex|locale|codecvt|random": "cihazda çok yer tutar ya da yavaştır",
    r"exception|stdexcept|typeinfo|typeindex|any": "istisna ve RTTI kapalı",
    r"unistd\.h|sys/.*|fcntl\.h|pthread\.h|dirent\.h|signal\.h|windows\.h|mach/.*": "işletim sistemi başlığı",
    r"esp_.*|freertos/.*|driver/.*": "ESP-IDF yalnız platform/esp32 içinde kullanılabilir",
}

FORBIDDEN_CODE = {
    r"\btry\s*\{|\bcatch\s*\(|\bthrow\b": "istisna kapalı",
    r"\bdynamic_cast\b|\btypeid\b": "RTTI kapalı",
    r"\bstd::(cout|cerr|cin)\b": "akışlar yasak; hal::log kullanın",
    r"\b(system|popen|fork|exec[lv]p?e?)\s*\(": "süreç çağrıları cihazda yok",
}

INCLUDE_RE = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]')


def strip_comments(line):
    return line.split("//", 1)[0]


def main():
    problems = []
    checked = 0
    for d in CORE_DIRS:
        base = ROOT / d
        if not base.exists():
            continue
        for path in sorted(base.rglob("*")):
            if path.suffix not in SUFFIXES or not path.is_file():
                continue
            checked += 1
            rel = path.relative_to(ROOT)
            for n, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
                m = INCLUDE_RE.match(line)
                if m:
                    header = m.group(1)
                    for pattern, why in FORBIDDEN_INCLUDES.items():
                        if re.fullmatch(pattern, header):
                            problems.append(f"{rel}:{n}: <{header}>: {why}")
                    continue
                code = strip_comments(line)
                for pattern, why in FORBIDDEN_CODE.items():
                    if re.search(pattern, code):
                        problems.append(f"{rel}:{n}: {why}: {line.strip()}")

    if problems:
        print("Taşınabilirlik kuralı ihlali:")
        for p in problems:
            print("  " + p)
        return 1
    print(f"taşınabilirlik denetimi: {checked} dosya temiz")
    return 0


if __name__ == "__main__":
    sys.exit(main())
