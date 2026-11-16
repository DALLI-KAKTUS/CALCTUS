"""CALCTUS demo: a function, a loop, and printed output."""


def kare(sayi):
    return sayi * sayi


for sayi in range(1, 6):
    print(sayi, "x", sayi, "=", kare(sayi))
