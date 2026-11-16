# CALCTUS

Açık kaynak, sıfırdan tasarlanan bir cep hesap makinesi ve küçük bilgisayar.
Qalculate, NumWorks ve DB48X'in sevilen yanlarından esinlenir; ama başka bir
cihazın yazılımının çatalı ya da kopyası değildir. Devre, kasa, simülatör ve
gömülü yazılımın hepsi bu depodadır.

- **İşlemci:** ESP32-S3-WROOM-1 N16R8 (16 MB flash, 8 MB PSRAM)
- **Ekran:** GMT020, 2,0" IPS TFT, ST7789, SPI; arayüz 320×240 yatay
- **Tuşlar:** 55 tuş, TCA8418 tuş matrisi entegresi
- **Güç:** tek hücre LiPo, USB-C şarj, MAX17048 pil göstergesi
- **Depolama:** microSD (belgeler, Python programları)
- **İsteğe bağlı:** Raspberry Pi Zero 2 W eklentisi (UART), üstte modül yuvası

## Durum

Erken geliştirme. Yazılım masaüstü simülatöründe çalışıyor: hesap/RPN,
grafik, Python (MicroPython), denklem çözücü, istatistik, olasılık, diziler,
periyodik tablo, dosyalar ve ayarlar. ESP32-S3 portu yazıldı ama gerçek kartta
henüz denenmedi. Devre şeması ve kasa henüz çizilmedi. Ayrıntı:
[`docs/STATUS.md`](docs/STATUS.md).

## Depo düzeni

```
software/      simülatör ve gömülü yazılım (tek kod tabanı, iki hedef)
  platform/sim/        masaüstü simülatörü (SDL2)
  platform/esp32/      ESP-IDF 5.5.1 portu (cihaz yazılımı)
  platform/pi-zero-2/  isteğe bağlı Pi için Linux ajanı
hardware/      devre: KiCad şeması ve PCB, malzeme listesi, üretim dosyaları
mechanical/    kasa ve tuşlar: CAD kaynakları, STL/STEP, teknik çizimler
docs/          mimari, durum, görseller
flake.nix      geliştirme ortamı (nix)
```

## Hızlı başlangıç (simülatör)

[Nix](https://nixos.org) gerekir; derleyici, SDL2 ve Python araçlarını o getirir.

```sh
cd software
./sim          # derler ve simülatör penceresini açar
./sim test     # birim ve ekran karşılaştırma testleri
```

Klavye eşlemesi, tuş betikleri ve cihaz derlemesi:
[`software/README.md`](software/README.md).

## Belgeler

- [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md): sistem mimarisi, sınırlar, Pi ve ağ tasarımı
- [`docs/STATUS.md`](docs/STATUS.md): güncel durum ve açık işler
- [`hardware/README.md`](hardware/README.md): devre
- [`mechanical/README.md`](mechanical/README.md): kasa ve 3B modeller

## Lisans

- Yazılım: **AGPL-3.0-or-later** ([`LICENSE`](LICENSE))
- Devre, kasa ve 3B modeller: **CERN-OHL-S-2.0** ([`hardware/LICENSE`](hardware/LICENSE), [`mechanical/LICENSE`](mechanical/LICENSE))
- Belgeler: **CC BY-SA 4.0** ([`docs/LICENSE`](docs/LICENSE))

Üçüncü taraf bileşenler (libbf, LVGL, MicroPython, DejaVu yazı tipi,
periyodik tablo verisi) kendi lisanslarıyla gelir:
[`THIRD_PARTY.md`](THIRD_PARTY.md).
