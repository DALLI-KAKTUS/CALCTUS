# CALCTUS yazılımı (simülatör + gömülü yazılım)

Sistemin nasıl çalıştığı, opsiyonel Pi Zero 2 W mimarisi, ekran/Wi‑Fi
tasarımı, çalışan/eksik özellikler ve kararlar için
[`docs/ARCHITECTURE.md`](../docs/ARCHITECTURE.md) dosyasına bakın. Güncel durum özeti
[`docs/STATUS.md`](../docs/STATUS.md) içindedir.

Sıfırdan yazılan bir hesap makinesi sistemi. Hedef donanım ESP32-S3
(GMT020/ST7789 240×320 panel, yatay 320×240 arayüz, 55 tuş).
Yazılım önce masaüstü simülatöründe geliştirilir; aynı kaynak kod sonra
cihaza derlenir. Ekran LVGL v9.4.0 ile RGB565 olarak çizilir; ana ekran,
sistem kabuğu, bütün uygulamalar ve pencereler yerel LVGL nesneleridir.
Donanımda görüntü ve tuş eşlemesi henüz fiziksel olarak doğrulanmadı.

Başka bir hesap makinesi yazılımının çatalı ya da kopyası değildir.

## Durum

| Aşama | Durum |
|---|---|
| Masaüstü simülatörü | Hesaplama uygulamaları ve sistem akışları mevcut; tam CTest paketi 35/35 geçti |
| LVGL | v9.4.0; temel ekranlar ve belirtilen uygulamalar native, kalan içerikler aşamalı taşınıyor |
| ESP32-S3 | ESP-IDF portu mevcut; son UI değişikliğinden sonra derleme doğrulanmadı (`idf.py` bu ortamda yok) |
| Fiziksel donanım | GMT020 alt modeli/pinler kesin değil; kart testi yapılmadı |

**Mevcut sistem kapsamı**

- Sistem: durum çubuğu (başlık, açı birimi, pil), içerik alanı, F1–F6 menü çubuğu,
  tuş yönlendirme, basılı tutunca tekrar, açılır pencereler, bildirimler.
- Bileşenler: liste, tablo, onay penceresi, seçim listesi, kaydırıcı, anahtar.
- Uygulamalar: ana ekran, Ayarlar (tema, parlaklık, açı birimi, sayı biçimi,
  hakkında), Bileşenler (kitin vitrini), Tuş testi.
- Ekrana yalnız değişen bölge çizilir ve gönderilir.

**S2 ilk dilim:** Ana ekrandaki Hesap uygulaması, [libbf](https://bellard.org/libbf/)
2025-06-03 sürümünün ondalık `bfdec` API'siyle 34 anlamlı basamakta işlem yapar.
`+ - * /`, parantez, tekli eksi, üs (`^`, üssün mutlak değeri en çok 100),
`sqrt`, `sin`, `cos`, `tan`, `ln`, `log`, `exp`, `pi`, `Ans` ve bilimsel giriş
(`EE` → `e`) desteklenir. Trigonometri Ayarlar'daki derece/radyan/grad birimini kullanır.
Ayarlardaki sayı biçimi
sonuç gösterimini belirler (ondalık/bilimsel biçimde 16 anlamlı basamak).
EXE/OK sonucu geçmişe ekler;
F1 girişi siler, F2 Ans, F3 işaret, F4 kök, F5 üs, F6 geçmişi temizler.
⌫ son karakteri siler. Son üç işlem ve son cevap bellekte tutulur.
`1/0` ve negatif kök gibi hatalar bildirilir. `Mod` cebirsel/RPN arasında
geçer. RPN'de dört seviyeli yığın, EXE ile giriş, dört işlem, üs, tekli
fonksiyonlar, DROP/ENTER/SWAP/ROLL/CLR ve `Ans` ile LASTx vardır.
Grafik uygulaması değişkenli `f(x)` çizer; Hesap'ta atanmamış `x` hata verir.
Yönlerle kaydırma, F3/F4 yakınlaştırma,
F5 sıfırlama, F6 örnek fonksiyonlar, F1 giriş temizleme destekler.
İstatistik uygulaması en çok 32 değerin toplamını, aritmetik ortalamasını,
popülasyon varyansını (`n` böleni), standart sapmasını ve min/max değerlerini
hesaplar. EXE/F1 veri ekler; F2 son veriyi geri alır, F3–F5 sonuç sayfaları,
F6 verileri temizler. Sonuçlar libbf ile hesaplanır; uzun değerler ekranda
bilimsel biçimde kısaltılır.
Olasılık uygulamasında `n!`, `nPr` ve `nCr` tam sayılı olarak hesaplanır;
`0! = 1`, `0 <= k <= n <= 60` desteklenir. Sonuçlar 60! dahil eksiksiz
ondalık basamaklarıyla üç satıra kadar gösterilir. F4/F5 n/k alanını seçer,
F1–F3 işlemi hesaplar, F6 alanları sıfırlar. Dağılım işlevleri henüz yoktur.
Çözücü, `ax²+bx+c=0` için doğrusal, iki reel, çift ve karmaşık kökleri
hesaplar; `a=b=0` durumunda çözümsüz/her sayı ayrımını yapar. F1–F3
katsayıları seçer, F4/EXE çözer, F5 seçili katsayıyı sıfırlar. Karmaşık
kökte reel parça ile pozitif sanal büyüklük gösterilir (`Re ± Im·i`).
Diziler, açık `u(n)=f(x=n)` ve yinelemeli `u(0)=başlangıç`,
`u(n)=f(x=n, Ans=u(n−1))` biçimlerini destekler. F1/F2 mod, F3 formül,
F4 başlangıç, F5 ilk indeks, F6/EXE hesaplama; ilk indeks 0–96
aralığında seçildiğinde beş terim görüntülenir (son indeks en çok 100).
Elementler kataloğu 1–118 atom numarasını kapsar. Numarayı yazıp EXE/F1 ile
atla; yönler veya F2–F5 ile gezin, F6 ile liste/ayrıntı arasında geç.
Sembol, uluslararası ad, yaklaşık atom kütlesi, periyot, grup, blok ve faz
gösterilir. En ağır elementlerin fazı tahmini olabilir. Sabit veri
`Bowserinator/Periodic-Table-JSON` projesinin `a78c7a48` revizyonundaki
olgusal alanlarından `tools/gen_elements.py` ile üretildi; kaynaktaki
doğrulanmamış 119. element tabloya alınmadı. Derleme veya çalışma sırasında
ağa erişim gerekmez.
Uzun sonuçlar ekranda sağa hizalı son kısmıyla görünür.

Kompleks sayılar, sembolik işlemler, CAS, ağ/AI ve kalıcı hesap
geçmişi henüz yoktur; bunlar bitmiş özellikler değildir. Ayarlar HAL saklama
arayüzüyle simülatörde çalışma klasöründeki `.calctus-settings.bin` dosyasına
yazılır; bozuk kayıt varsayılanlara döner.

 Ana ekranda soluk görünen uygulamalar henüz yazılmadı; açılınca "yakında" der.
Python uygulaması MicroPython 1.25.0 VM ile microSD kökündeki 4095 bayta kadar
ASCII `.py` dosyalarını çalıştırır; F2 gerçek VM'de örnek programı çalıştırır.
Fonksiyon, döngü, liste, istisna ve yerleşik modüller desteklenir; program
başına temiz VM ve çıktı/işlem sınırı vardır. microSD kökündeki başka bir
`.py` dosyası `import ad` ile yüklenebilir. `open()` yalnız metin modu `r`,
`w`, `a` ile kök dizindeki en çok 4095 baytlık dosyalarda çalışır;
`read/readline/write/seek/tell/flush/close` ve `with` desteklenir.
Alt klasörler, binary mod ve genel VFS henüz desteklenmez; Python'un kendi
`float` işlemi ikili double, Hesap uygulamasının yüksek hassasiyetli işlemi
libbf'dir. Pi Zero 2 bağlantı uygulaması UART ping/bilgi/örnek uzaktan hesap
akışını destekler; gerçek Pi terminali/ekran aktarımı ve AI henüz çalışmaz.
Dosyalar uygulaması
simülatörde çalışma dizinindeki `.calctus-sd/` sanal kartı kullanır;
ESP32'de FAT microSD kartı SPI3 üzerinden kullanır. Ekran editörünün
sınırı 4095 baytlık ASCII metin ve kökte 32 görünür dosyadır; microSD'nin
tamamı için dosya yöneticisi henüz yoktur. F1 yeni, F2 aç, F3 iki kez sil,
F5 yenile; editörde F1 kaydet, F2/ALPHA harf-rakam, F3 satır, F4 ad,
F6 liste. Harf modunda 2–9 tuşlarına art arda basarak harf seçilir;
Sağ tuşu aynı harfi tekrar girmeden önce seçimi sabitler. Listeden çıkarken
değişiklikler kaydedilir. FN simge modunda `1=:` ve `3='` gibi Python
noktalama işaretleri girilir. ESP32
portunun örnek GPIO değerleri gerçek PCB bağlantısı değildir; ayrıntılar ve
derleme yolu [`platform/esp32/README.md`](platform/esp32/README.md) içindedir.

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
| Rakamlar, `. , + - * / ( ) ^`, `x p a` | Aynı tuşlar; x / π / Ans |
| Enter / Backspace / Esc | EXE / sil / geri |
| Ok tuşları, Boşluk | Yön tuşları, OK |
| F1–F6 | Menü tuşları |
| Tab | mod |
| Sağ Shift / Sağ Option / Ctrl / Sağ Cmd | shift / alpha / ctrl / fn |
| F12 / F11 | Ekran görüntüsü / cihaz görüntüsü (PNG) |

Penceresiz mod ve tuş betiği (otomatik test için):

```sh
./sim --headless --script tests/scripts/calc.keys
```

Betik komutları: `key AD`, `down AD`, `up AD`, `wait MS`, `shot DOSYA`,
`shot-device DOSYA`, `quit`. Tuş adları `hal/include/calctus/keymap.def` içinde.
Penceresiz modda zaman sanaldır; betikler anında ve her seferinde aynı sonuçla biter.

## Düzen

```
hal/include/calctus/   HAL sözleşmesi, tuş tanımları (keymap.def), temel tipler
kernel/                sistem (ana döngü, ekran düzeni, tuş yönlendirme), uygulama arayüzü, ayarlar
ui/                    LVGL görünümleri (lvgl_screen, lvgl_apps, lvgl_kit), tema, font, bileşen durumları
apps/                  ortak uygulamalar (hesaplama, dosyalar, Python, Pi, sistem)
math/                  libbf üzerinde taşınabilir ifade ayrıştırıcı
third_party/libbf/      MIT lisanslı sabitlenmiş libbf kaynakları
third_party/lvgl/       MIT lisanslı LVGL v9.4.0 kaynakları (SDL/ESP ortak)
platform/sim/          masaüstü simülatörü (SDL2) — HAL'in masaüstü gerçeklemesi
platform/esp32/        ESP-IDF 5.5.1 portu (ekran, klavye, pil, NVS, SD, Pi UART)
platform/pi-zero-2/    opsiyonel Linux UART agent'ı; ESP ortak kaynaklarına girmez
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

CALCTUS için henüz seçilmedi (öneri: GPLv3). Seçilene kadar CALCTUS kodunun
tüm hakları saklıdır. Üçüncü taraf libbf MIT lisanslıdır; telif ve lisans
bildirimleri `third_party/libbf/` altında korunur.
