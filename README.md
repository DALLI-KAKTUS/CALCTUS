# CALCTUS

Sıfırdan yazılan, açık kaynak bir hesap makinesi sistemi. Hedef donanım ESP32-S3
(320×240 renkli ekran, 55 tuş). Yazılım önce masaüstü simülatöründe geliştirilir;
aynı kaynak kod sonra cihaza derlenir.

Başka bir hesap makinesi yazılımının çatalı ya da kopyası değildir.

## Durum

| Aşama | Durum |
|---|---|
| S0 İskelet | tamam |
| S1 Arayüz kiti | tamam (bu dal: `s1-arayuz-kiti`) |
| S2 Hesaplama | sırada |

**S1 ile gelenler**

- Sistem: durum çubuğu (başlık, açı birimi, pil), içerik alanı, F1–F6 menü çubuğu,
  tuş yönlendirme, basılı tutunca tekrar, açılır pencereler, bildirimler.
- Bileşenler: liste, tablo, onay penceresi, seçim listesi, kaydırıcı, anahtar.
- Uygulamalar: ana ekran, Ayarlar (tema, parlaklık, açı birimi, sayı biçimi,
  hakkında), Bileşenler (kitin vitrini), Tuş testi.
- Ekrana yalnız değişen bölge çizilir ve gönderilir.

Ana ekranda soluk görünen uygulamalar henüz yazılmadı; açılınca "yakında" der.
Ayarlar şimdilik yalnız bellekte durur (kalıcı saklama depolama servisiyle gelecek).

**Cihaz tuşları**

| Tuş | İşlev |
|---|---|
| Ok tuşları, OK / EXE | Gezinme, seçme |
| ↩ (geri) | Bir önceki sayfa; uygulamanın ilk sayfasında ana ekran |
| ⌂ | Ana ekran |
| güç | Ekranı kapat / aç |
| F1–F6 | Ekranın altındaki menü çubuğu |

## Çalıştırma

Gereken tek şey [Nix](https://nixos.org) (flake desteğiyle). Derleyici, SDL2 ve
araçlar geliştirme kabuğundan gelir; sisteme bir şey kurulmaz.

```sh
./sim            # derle ve simülatörü aç
./sim test       # testleri çalıştır
```

Pencerede:

| Giriş | Karşılığı |
|---|---|
| Fare | Tuşlara tıklama |
| Rakamlar, `. , + - * / ( ) ^` | Aynı tuşlar |
| Enter / Backspace / Esc | EXE / sil / geri |
| Ok tuşları, Boşluk | Yön tuşları, OK |
| F1–F6 | Menü tuşları |
| Tab | mod |
| Sağ Shift / Sağ Option / Ctrl / Sağ Cmd | shift / alpha / ctrl / fn |
| F12 / F11 | Ekran görüntüsü / cihaz görüntüsü (PNG) |

Penceresiz mod ve tuş betiği (otomatik test için):

```sh
./sim --headless --script tests/scripts/keytest_all.keys
```

Betik komutları: `key AD`, `down AD`, `up AD`, `wait MS`, `shot DOSYA`,
`shot-device DOSYA`, `quit`. Tuş adları `hal/include/calctus/keymap.def` içinde.
Penceresiz modda zaman sanaldır; betikler anında ve her seferinde aynı sonuçla biter.

## Düzen

```
hal/include/calctus/   HAL sözleşmesi, tuş tanımları (keymap.def), temel tipler
kernel/                sistem (ana döngü, ekran düzeni, tuş yönlendirme), uygulama arayüzü, ayarlar
ui/                    çizim (Canvas), tema, yazı tipleri, bileşenler (widgets/)
apps/                  uygulamalar (home, settings, gallery, keytest)
platform/sim/          masaüstü simülatörü (SDL2) — HAL'in masaüstü gerçeklemesi
platform/esp32/        ESP-IDF gerçeklemesi (donanım aşamasında)
assets/                yazı tipi karakter listesi
tools/                 yazı tipi üreteci, taşınabilirlik denetimi, ekran testi
tests/                 tuş betikleri ve kayıtlı ekran görüntüleri
```

## Kurallar

1. **Tek kod, iki hedef.** `platform/` dışındaki her şey simülatörde ve cihazda
   aynı kaynaktır.
2. **Masaüstünde çalışıp ESP32'de çalışmayacak hiçbir şey ortak koda girmez.**
   SDL, işletim sistemi başlıkları, istisnalar, RTTI, akışlar ve iş parçacıkları
   yalnız `platform/sim` içinde kullanılabilir. `tools/check_portable.py` bunu her
   test çalışmasında denetler; ortak kod ayrıca `-fno-exceptions -fno-rtti` ile
   derlenir.
3. **Ekrana yalnız değişen bölge gönderilir.** Uygulamalar `System::invalidate`
   ile bölge bildirir; sistem yalnız orayı çizip `hal::display_push` ile gönderir.
   Cihazda her piksel SPI üzerinden gider.
4. **Bekleme HAL'de yapılır** (`hal::keys_wait`). Cihazda işlemci orada uyur.
5. **Tuş takımının tek kaynağı `keymap.def`.** Simülatör kaplaması, tuş kimlikleri
   ve ileride PCB tuş matrisi oradan üretilir.

## Yazı tipleri

Glifler [DejaVu Sans](https://dejavu-fonts.github.io)'tan derleme zamanında bit
eşleme çevrilir (`tools/gen_fonts.sh`) ve `ui/fonts/` altında kaynak olarak durur.
Cihazda yazı tipi dosyası ya da çizici yoktur. DejaVu yazı tipleri serbest
lisanslıdır (Bitstream Vera lisansı).

## Lisans

Henüz seçilmedi (öneri: GPLv3). Seçilene kadar tüm hakları saklıdır.
