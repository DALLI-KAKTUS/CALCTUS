# ESP32-S3 portu (ESP-IDF 5.5.1)

Opsiyonel Pi Zero 2 W bağlantısının UART kontrol protokolü ve henüz
uygulanmamış ekran/USB/Wi‑Fi tasarımı için
[`../../ARCHITECTURE.md`](../../ARCHITECTURE.md) bölüm 5–10'a bakın.

Bu port aynı `cmake/core_sources.cmake` kaynak listesini kullanır: libbf, çekirdek,
uygulamalar ve arayüz kodu masaüstü ile aynıdır. `main/hal.cpp` donanıma özgü
GMT020/ST7789/SPI2, TCA8418/I²C, MAX17048/I²C, isteğe bağlı LEDC arka ışık, NVS ve
microSD/SPI3 (FAT) arayüzünü içerir.
MicroPython 1.25 embed VM de iki hedefte aynı C kaynaklarıyla derlenir;
çalıştırma başına 96 KiB GC belleği ayrılır. `open()` kök dizinde `r/w/a`
metin modlarını ve 4095 bayt sınırını destekler; alt klasörler desteklenmez.
Kökteki `.py` dosyaları çalışır ve birbirini `import` edebilir. Bu çalışma
biçimi gerçek kartta henüz denenmemiştir.
Çerçeve belleği PSRAM'de, SPI aktarım satırı dahili DMA belleğindedir.

`main/Kconfig.projbuild` içindeki GPIO varsayılanları **yalnız derleme örneğidir**;
herhangi bir PCB'nin gerçek bağlantısı olarak doğrulanmamıştır. Gerçek cihazda
ESP32-S3-WROOM-1 N16R8, 240×320 GMT020/ST7789 (320×240 yatay döndürülmüş),
8×7 matrise bağlı TCA8418 (I²C
0x34), MAX17048 (0x36), arka ışık PWM girişi ve harici I²C pull-up'ları
gereklidir. TCA8418 olay kodları 8×10 düzeninden `keymap.def` içindeki 8×7
sıraya dönüştürülür. Şarj durum pini opsiyoneldir; varsayılanda `-1` olup
şarj bilgisi gösterilmez. GMT020 alt modeli (7P/8P) henüz teyit edilmedi;
BL pini yoksa `CONFIG_CALCTUS_LCD_BL=-1` ve yalnız ekran aç/kapat çalışır,
parlaklık PWM ayarı fiziksel olarak uygulanmaz. X/Y RAM offset (varsayılan 0),
ayna, yön ve 40 MHz SPI hızı modül üzerinde denenmelidir. Farklı ekran kontrolcüleri veya pil göstergeleri
bu portta henüz desteklenmez.

microSD kart ayrı SPI3 veri yolu kullanır; CS, CLK, MOSI, MISO ve isteğe
bağlı kart algılama pini Kconfig ile ayarlanır. Kart FAT olarak **önceden
biçimlendirilmiş** olmalıdır; bağlama hatasında yazılım asla kartı otomatik
biçimlendirmez. Kart yoksa hesap uygulamaları açılır, Dosyalar hata gösterir.
Soket ve SPI hatları için gerekli çekme dirençleri gerçek şemada seçilmelidir.
Bu sürüm kök dizindeki kısa ASCII adlı ilk 32 dosyayı listeler; 4095 baytı
aşmayan ASCII metinleri düzenler. Bu sınırlama kartın kapasitesi
değil, ekran editörünün sınırıdır. Cihaz üzerinde kart takma/çıkarma,
bağlanma ve veri tutarlılığı **test edilmemiştir**.

Önce gerçek şemadaki GPIO ve GMT020/ST7789 yön/ayna/offset değerlerini `menuconfig`
ekranında **CALCTUS board wiring** altında ayarlayın. 16 MB flash/8 MB PSRAM
varsayılanları N16R8 içindir. Örnek derleme:

```sh
. "$IDF_PATH/export.sh"
idf.py -C platform/esp32 set-target esp32s3
idf.py -C platform/esp32 menuconfig
idf.py -C platform/esp32 build
```

**Uyarı:** Çapraz derleme yalnız sözdizimi, bağlantı ve kaynak tüketimi için
kanıttır; pinlerin doğruluğunu, ekranın yönünü, tuşların fiziksel sırasını,
PSRAM/SPI bütünlüğünü, güç tüketimini ve gerçek pil yüzdesini donanım olmadan
kanıtlamaz. Bu port için cihaza yükleme ve fiziksel test henüz yapılmadı.
