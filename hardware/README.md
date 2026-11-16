# Devre

CALCTUS'un elektronik tasarımı. Araç: [KiCad](https://www.kicad.org) (sürüm
proje dosyasında yazılı olacak). Henüz şema çizilmedi; aşağıdaki düzen
çizime başlandığında kullanılacak.

```
kicad/         KiCad projesi (.kicad_pro, .kicad_sch, .kicad_pcb)
  lib/         projeye özel sembol ve footprint kütüphaneleri
bom/           malzeme listesi (CSV) ve parça seçim notları
datasheets/    kullanılan parçaların veri sayfalarına bağlantılar (PDF eklenmez)
production/    üretim çıktıları: Gerber, delik, dizgi (pick-and-place) dosyaları
```

## Planlanan bloklar

| Blok | Parça | Not |
|---|---|---|
| İşlemci | ESP32-S3-WROOM-1 N16R8 | USB-C ile programlama |
| Ekran | GMT020 (ST7789, SPI) | Tam alt model ve pinler kartta doğrulanacak |
| Tuşlar | 55 tuş, TCA8418 (I2C 0x34) | İlk sürümde tact switch |
| Güç | LiPo, USB-C şarj, buck-boost 3,3 V, MAX17048 (I2C 0x36) | Arka ışık, SD ve modül yuvası için yük anahtarları |
| Depolama | microSD (SPI) | |
| Eklenti | Pi Zero 2 W: UART, ayrı 5 V güç hattı | Tasarım `docs/ARCHITECTURE.md` bölüm 4–8 |
| Modül yuvası | Güç, I2C, SPI, UART, GPIO, kimlik EEPROM'u | Konnektör tipi karar bekliyor |

Yazılımdaki pin numaraları (`software/platform/esp32/main/Kconfig.projbuild`)
yalnız derleme örnekleridir; gerçek pinler şema bitince oraya işlenir.
