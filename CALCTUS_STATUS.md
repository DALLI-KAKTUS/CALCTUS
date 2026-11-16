# CALCTUS — güncel proje durumu

**Son güncelleme: 16 Kasım 2026.** Bu dosya mevcut durumu özetler; ayrıntılı
mimari, kullanım sınırları ve tasarım kararları [`ARCHITECTURE.md`](ARCHITECTURE.md)
içindedir.

## Mevcut durum

- CALCTUS, ESP32-S3 ve 320×240 yatay GMT020/ST7789 ekran için geliştirilen,
  masaüstü simülatörü bulunan özgün bir hesap makinesi sistemidir.
- LVGL v9.4.0 simülatörde kullanılır. Ana ekran/kabuk, Ayarlar, Pi,
  Elementler, Hesap/RPN, Çözücü, Grafik, İstatistik ve genel pencereler native
  LVGL'dir. Diğer uygulama içeriklerinin bir kısmı LVGL canvas içindeki eski
  çizim kodunu kullanır; klavye olayları mevcut sistem yönlendirmesindedir.
- Hesap, RPN, Grafik, Çözücü, İstatistik, Olasılık, Diziler, Elementler,
  Dosyalar, Python/MicroPython ve ayarlar simülatörde çalışır. Pi UART kontrol
  prototipi vardır. Terminal, AI, Pi ekran aktarımı ve Wi‑Fi paylaşımı
  uygulanmamıştır.
- ESP32-S3 için ESP-IDF portu mevcuttur. GMT020 alt modeli, PCB pinleri ve
  ekran offset'i doğrulanmamıştır; fiziksel kart/ekran/tuş/microSD testi yoktur.

## Son doğrulama

| Kontrol | Sonuç |
|---|---|
| Tam simülatör CTest paketi | **35/35 geçti** |
| İstatistik hedefli testler | **3/3 geçti** |
| `git diff --check` | Temiz |
| ESP32-S3 derlemesi, son LVGL değişiklikleriyle | Çalıştırılamadı; bu ortamda `idf.py` yok |
| Fiziksel donanım | Doğrulanmadı |

Önceki ESP imaj boyutu son İstatistik değişikliğini kapsamadığından güncel
derleme sonucu veya boyutu olarak kullanılmamalıdır.

## Açık işler

1. ESP-IDF 5.5.1 ortamında ESP32-S3 derlemesini tekrar çalıştırıp imaj/bölüm
   boyutunu kaydetmek.
2. Canvas tabanlı uygulama içeriklerini aşamalı olarak native LVGL'ye taşımak;
   her geçişte simülatör testlerini çalıştırmak.
3. PCB şeması, GMT020 alt modeli ve pin/offset değerleri kesinleşince fiziksel
   ekran, tuş takımı ve microSD doğrulaması yapmak.
4. Terminal, AI, Pi ekran aktarımı ve ağ paylaşımı için kullanıcı kararlarını
   almadan uygulama kapsamını genişletmemek.
