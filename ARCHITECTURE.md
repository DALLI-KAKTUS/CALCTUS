# CALCTUS — sistem mimarisi, çalışma biçimi ve yol haritası

**Güncelleme: 16 Kasım 2026.** Bu belge tek başına projenin parçalarını,
çalışan işlevlerini, kararlarını ve *henüz yalnız tasarım olan* Pi ekran/ağ
özelliklerini anlamak için yazılmıştır. `[VAR]` derlenmiş/testli yazılım,
`[TASARIM]` uygulanmamış hedef, `[KARAR]` kesinleşmesi gereken ayrıntıdır.
`[VAR]` ifadesi fiziksel kartta test edildi anlamına **gelmez**.

**LVGL geçişi (tamam, 16 Kasım 2026):** LVGL v9.4.0 kaynakları
`third_party/lvgl/` içinde sabittir. Simülatör ve ESP aynı RGB565 kısmi flush,
HAL zamanı ve LVGL widget ağacını derler. Durum çubuğu, F tuş etiketleri,
bildirim, genel onay/seçim/kaydırıcı pencereleri ve 14 uygulamanın tamamı
(ana ekran dahil) LVGL nesneleriyle çizilir. Her uygulamanın `App::View`
değeri vardır; `LvglScreen::content` açık uygulamanın görünümünü durumundan
baştan kurar. İlk sekiz görünüm `ui/lvgl_screen.cpp`, Olasılık/Diziler/
Python/Dosyalar/Bileşenler/Tuş testi `ui/lvgl_apps.cpp` içindedir; ortak
yardımcılar `ui/lvgl_kit`. Eski `ui::Canvas` uygulama yolu, `App::draw` ve
ikinci çerçeve tamponu (`hal::legacy_framebuffer`) kaldırıldı. `ui::Canvas`
yalnız simülatör cihaz görünümü ve metin genişliği ölçümü için kalır;
`ListView`/`TableView` çizim yapmaz, seçim/kaydırma durumu tutar. Klavye
olayları `System::dispatch` üzerinden uygulama yaşam döngüsüne gider; LVGL
`indev`/focus grupları kullanılmaz, çünkü 55 tuşun anlamı uygulamaya özgüdür.

## 1. Ürün ve çalışma modelleri

CALCTUS, GMT020 ailesinden ST7789 kontrollü 240×320 fiziksel ekranı
320×240 yatay çevirerek ve 55 tuşla kullanılacak ESP32-S3 tabanlı bir
hesap cihazıdır. Özgün yazılım; başka cihazın firmware'inin çatalı değildir.
Geliştirme masaüstü SDL simülatöründe yapılır, aynı ortak C/C++ uygulama ve
MicroPython kaynakları ESP-IDF 5.5.1 ile ESP32-S3'e çapraz derlenir.

```text
                  bağımsız mod (her zaman çalışmalı)
tuşlar → TCA8418 → ESP32-S3 → GMT020/ST7789 ekran
                        ├─ libbf / uygulamalar / MicroPython
                        ├─ NVS (ayarlar)
                        └─ FAT microSD (belgeler ve .py)

                  opsiyonel eklenti modu
                   UART: algılama / küçük komutlar [VAR]
ESP32-S3  ←────────────────────────────────────→  Pi Zero 2 W + Linux agent
           gelecekte USB veri yolu: ekran + etkileşim [TASARIM]
```

**Sahiplik ilkesi:** ESP daima sistemin sahibi, ekran denetleyicisi, klavye
okuyucusu ve yerel uygulama işlemcisidir. Pi kapalı/çıkarılmış/yanıtsızken
Hesap, Dosyalar, Python, ayarlar ve SD Pi yüzünden bloke olmaz. Pi yalnız
Pi'ye devredilmiş uygulama açıldığında uyandırılmalı/bağlanmalı; şu anda
yalnız UART sorgusu vardır, otomatik güç anahtarlaması **yoktur**. Pi ekranı
ESP'nin ekranını elektriksel olarak devralmaz.

## 2. Kaynak kod akışı ve başlatma

`platform/sim/main.cpp` veya `platform/esp32/main/main.cpp` → `boot()`
(`kernel/boot.cpp`) → `System::run()` (`kernel/system.cpp`). `boot()` uygulama
nesnelerini ana ekran kayıtlarına bağlar. Sistem sırasıyla HAL'i başlatır,
kalıcı ayarları yükler, LVGL v9.4 ekran ağacını kurar, ana ekranı açar,
olay döngüsünde değişen bölgede açık uygulamanın LVGL görünümünü yeniden kurar. `lv_timer_handler` kısmi RGB565 flush callback'iyle
HAL framebuffer'ına yazar; `hal::display_push` paneli günceller.
`hal::keys_wait` tuş/timer bekler. Uygulamalar `kernel/app.h` arayüzünde
`view`, `on_open`, `on_key`, `soft_keys` ve isteğe bağlı periyodik tick sunar.
Güç/ana ekran gibi sistem tuşları önce `System` tarafından değerlendirilir.

`hal/include/calctus/hal.h` iki platformun sözleşmesidir. Ekran, tuş, zaman,
pil, kalıcı ayarlar, microSD ve opsiyonel Pi byte alışverişinin iki ayrı
gerçeklemesi vardır: `platform/sim/sim.cpp` ve `platform/esp32/main/hal.cpp`.
`cmake/core_sources.cmake` ortak uygulama listesini iki hedefe de verir;
`cmake/micropython_sources.cmake` VM paketini her ikisinde derler. Ortak kodda
SDL, masaüstü dosya sistemi ve işletim sistemine özgü API yoktur; ESP hedefi
`-fno-exceptions -fno-rtti` derlenir. Platform HAL'inde OS/IDF API bulunur.

Mantıksal ekran 320×240 RGB565'tir (GMT020 panel doğal yönü 240×320,
Kconfig `swap_xy` ile yatay): tam kare 153.600 bayt. ESP'de LVGL çıkış
framebuffer'ı PSRAM'de (153.600 bayt), 16 satırlık LVGL render tamponu ve
SPI gönderim satırı dahili RAM'dedir. LVGL kısmi flush kirli alanı
`display_push` ile gönderir. Tuş adları ve 8×7 matrisin kaynağı `keymap.def`;
simülatör skin/girdi ve TCA8418 eşlemesi bu sırayı paylaşır. Simülatörde
tuş betikleri sanal zamanda çalışır, golden PNG'ler piksel karşılaştırılır.

## 3. Çalışan uygulamalar ve sınırları `[VAR]`

| Uygulama/altyapı | Bugünkü işlev ve sınır |
|---|---|
| Ana ekran/arayüz | Durum, pil göstergesi, temalar, bildirim, modal, F1–F6, ekran kapama, bölgesel çizim. |
| Hesap | libbf `bfdec` ile 34 anlamlı basamak: dört işlem, parantez, üs, kök, trigonometrik, log/ln/exp, pi, Ans; kısa geçmiş RAM'de, kalıcı değil. Trig derece/radyan/grad ayarına uyar. |
| RPN | Dört seviyeli stack; aritmetik, üs, fonksiyonlar, ENTER/DROP/SWAP/ROLL/CLR, LASTx. |
| Grafik | `f(x)` çizim, kaydırma, ölçekleme, sıfırlama, örnekler. |
| Çözücü | İkinci derece denklemde doğrusal/reel/çift/karmaşık kök durumları. |
| İstatistik | 32 değere dek toplam, ortalama, popülasyon varyansı, std sapma, min/max. |
| Olasılık | Tam sayılı faktöriyel, `nPr`, `nCr`; `n≤60`; dağılım fonksiyonları yok. |
| Diziler | Açık ve yinelemeli; indeks üst sınırı 100, beş terim görünümü. |
| Elementler | 1–118 doğrulanmış sabit tablo; sembol, ad, kütle ve sınıflandırma. Doğrulanmamış 119. element yok. |
| Ayarlar | Tema, parlaklık, açı ve sayı biçimi; HAL kayıtları simde `.calctus-settings.bin`, ESP'de NVS. |
| Dosyalar | microSD kökünde dosya liste/yeni/düzenle/kaydet/yeniden aç/yeniden adlandır/sil; ekranda ilk 32 girdi, ASCII ad, en çok 4095 bayt ASCII metin. Alt dizin ve genel dosya yöneticisi yok. |
| Python | Gerçek MicroPython v1.25.0 VM; kök `.py` çalıştırma/import ve sınırlı `open` (`r/w/a`, metin, ≤4095 bayt). Her çalıştırmada temiz VM, 96 KiB GC heap, 8 KiB stack sınırı, 2047 bayt çıktı ve 100.000 VM loop hook sınırı. `float` ikili double'dır; libbf matematiği değildir. Binary/alt dizin/genel VFS yok. |
| Pi Zero 2 | Uygulama açıldığında UART ping; Info ve 32 bit kare hesabı uzakta. Simülatörde `CALCTUS_PI_SIM=1` ile takılı Pi taklidi. Bağlantı yoksa mesaj verir; diğer uygulamalar çalışır. **Henüz Pi terminali/ekranı değil.** |
| Bileşenler/Tuş testi | Arayüz vitrini ve cihaz tuş eşlemesi testi. |
| Terminal / AI | Tam uygulama değil. Eski Terminal menü yeri Pi bağlantı uygulamasına ayrıldı. AI servisi/endpoint/credential ve SSH kapsamı seçilmedi. |

libbf sürümü 2025-06-03 olarak vendored. Element tablosu
`Bowserinator/Periodic-Table-JSON` kaynağının `a78c7a48` revizyonundaki
olgusal verilerden oluşturuldu. MicroPython v1.25.0 commit'i
`f498a16c7db6d4b2de200b3e0856528dfe0613c3`, MIT lisanslı; kaynaklar
`third_party/micropython_embed/` içinde, derlemede ağ erişimi yok.
`tools/vendor_micropython.py` aynı paket için yeniden üretme aracıdır.

## 4. SD, bellek, dosya ve güç modeli

Simülatörde SD kökü çalıştırma dizininde `.calctus-sd/` (repo kökünden
`./sim` bu dizine girer). ESP FAT microSD'yi SPI3 üzerinden `/sdcard` olarak
bağlar. FAT önceden hazırlanmalı; montaj başarısızsa otomatik biçimlendirme
**kesinlikle kapalı**. `sd_refresh()` yeniden bağlanmayı dener. Kart yokken
SD/Python dosya işlevleri hata verir ama yerel hesap devam eder. NVS kullanıcı
belgesi için değil, yalnız ayarlar için ayrıldı. Python import yalnız kök
ASCII `.py`; `open()` metin `read/readline/write/seek/tell/flush/close/with`
işlemlerini HAL üzerinden yürütür; `flush/close` diske yazar.

**Güç tasarımı `[TASARIM]`:** Pi ayrı, ölçülendirilmiş 5 V güç rayı ve
ESP denetimli load switch/regülatör ister. ESP GPIO'sundan Pi beslenmez.
Güç açılışı, Linux boot/READY, kapanış isteği + güvenli shutdown, timeout,
voltaj düşümü, hot-unplug ve batarya ölçümü ayrıca tasarlanacak. Şu an Pi
uygulaması yalnız protokol sorgusu yapar; güç açıp kapamaz. Ortak GND ve
seviye uyumlu 3,3 V UART gerekir; 5 V GPIO bağlantısı yapılmaz.

## 5. Pi kontrol protokolü `[VAR]`

`services/pi_link.{h,cpp}` platformdan bağımsız küçük bir istek/yanıt
protokolüdür; `hal::pi_exchange` alttaki UART'ı sağlar. ESP tarafında UART1,
115200 8N1, TX/RX Kconfig ile ayarlanır, **varsayılan olarak kapalıdır**.
Pi Zero 2 W üzerinde Raspberry Pi OS + Python 3 standart kütüphanesi agent'ı
`platform/pi-zero-2/calctus_agent.py`, örnek `/dev/serial0` kullanır.
Simde varsayılan Pi yok; `CALCTUS_PI_SIM=1 ./sim` ile aynı sözleşmenin mock'u
etkinleşir. Pi agent gerçek UART uçlarının fiziksel bağlantısı test edilmedi.

Geliştirme ortamında deneme: `CALCTUS_PI_SIM=1 ./sim` ile ana ekranda
**Pi Zero 2** uygulamasını açın; `F1` ping, `F2` agent etiketi, `F3`
(yukarı/aşağı ile değiştirilebilen) sayının Pi'de karesi. `./sim` tek başına
başlatıldığında aynı uygulama “Pi yok” gösterir ve yerel uygulamalara
dönülebilir. Fiziksel Pi üzerinde `python3
platform/pi-zero-2/calctus_agent.py /dev/serial0` ancak Linux UART konsolu,
doğru seri aygıtı ve izinleri doğrulandıktan sonra kullanılmalıdır; systemd
servisi olarak otomatik başlatma henüz eklenmedi.

Paket tam olarak `A5 5A | version=01 | sequence:u8 | command:u8 |
length:u8 | payload[0..48] | CRC16:u16le`. CRC-CCITT polynomial `0x1021`,
initial `0xFFFF`, `version` ile yük sonuna kadar hesaplanır. Toplam en fazla
56 bayt. Yanıtta aynı sıra ve komut beklenir; sürüm/uzunluk/CRC hatalıysa
reddedilir. UART işlemi için 400 ms timeout ve girişte eski veriyi boşaltma
vardır. Komutlar: `1 Ping → "OK"`, `2 Info → kısa agent etiketi`, `3 Square →
little-endian signed int32 girdi/sonuç`, `127 Error` rezerve, henüz ayrıntılı
hata aktarımı yok. Kare hesabı `[-46340, 46340]` girdiyle sınırlı. Protokol
şimdilik kimlik doğrulamasızdır: **parola, Wi‑Fi anahtarı, key, shell veya
gizli veri bu protokolle taşınmamalıdır**. Kötü niyetli cihaza karşı güvenli
değildir. Bu kontrol kanalı görüntü/veri akışına uygun değildir.

Pi üzerinde agent'ı çalıştırmadan önce Linux serial console/Bluetooth UART
ataması, pinler, `/dev/serial0` izinleri ve 3,3 V lojik gerçek PCB için
teyit edilmelidir. Doğru seri port veya Pi pinleri şu an donanım şemasıyla
belirlenmiş değildir.

## 6. Pi arayüzünü CALCTUS ekranında göstermek `[TASARIM]`

**Yanlış varsayım:** Pi HDMI sinyalini ESP'nin ST7789 SPI portuna bağlayıp
göstermek mümkün değildir. Pi HDMI compositor'ü ayrıca yakalanıp yeniden
kodlanmadıkça görüntü ESP'ye gelmez. 320×240 RGB565 tam kare 153.600 bayt;
115200 baud 8N1 UART yaklaşık 11.520 bayt/sn taşır. Çerçeveleme hariç tek
tam kare ≥13,3 saniye sürer. Dolayısıyla mevcut UART yalnız kontrol içindir.

**Tercih edilen gelecek mimari:** Pi Zero 2 W'nin USB *data/OTG* portu gadget
modunda, ESP32-S3 USB-OTG portu host modunda ayrı bir yüksek hızlı *mantıksal*
veri kanalı kurar. ESP32-S3 USB FS teorik 12 Mbit/sn'dir; protokol/host/CPU
yükü yüzünden etkin hız daha düşer. USB CDC-ACM ile prototip veya uygun bulk
endpoint sürücüsü seçilecek; **bu bileşenler henüz yazılmadı**, PCB/USB
portlarının aynı anda programlama için nasıl kullanılacağı çözülmeli.
USB pin, VBUS/5 V izolasyonu, yön ve güç yönetimi karta özgüdür.

Pi'nin HDMI masaüstünü otomatik aynalamak yerine ilk hedef **320×240 sanal Pi
uygulama yüzeyi**: Pi uygulaması framebuffer/dirty rectangle üretir; ESP
alır, PSRAM tamponundaki bölgeyi günceller, mevcut `display_push` ile ST7789'a
iletir. Pi'den gelen içerik uygulamanın içerik alanıyla kırpılır; ESP durum
çubuğu, ana ekran/Güç ve F tuşları sahipliğini bırakmaz. Pi tuş olaylarını
`keymap.def` kimlikleriyle alır; tuş up/down, session id ve zaman damgası
eklenir. Ekran açılırken session müzakere edilir, ilk keyframe istenir, sonra
kirli dikdörtgenler gönderilir. Planlanan mesaj: version, session, frame id,
x/y/w/h, RGB565 little-endian veya basit sıkıştırma, payload length, CRC;
boyut/koordinat doğrulanmadan ekrana yazılmaz. Eksik/tekrar paket, timeout,
yeniden bağlanma ve geriden gelen kareler için ACK/yeniden keyframe gerekir.
Tam kare yükü ≈150 KiB; 32×32 bölge 2 KiB. Hedef 5 fps **bir gereksinim
adayıdır**, ölçüm yapılmadan performans vaadi değildir. Video/HDMI tam
masaüstü aktarımı daha sonra Linux DRM/KMS capture, ölçekleme, sıkıştırma ve
USB throughput ile ayrıca sınanabilir; mevcut ekranda küçücük terminal metni
uygulamaya özgü metin olaylarıyla daha verimli aktarılır.

**Terminal için öneri `[TASARIM]`:** Pi üzerinde PTY aracılığıyla bir terminal
oturumu başlat; giriş tuşları USB uygulama kanalıyla Pi'ye, sınırlı UTF-8
satırları ESP'nin LVGL metin görünümüne veya Pi sanal yüzeyine aktar. Yeniden
boyutlandırma, Ctrl/Alt, ANSI kaçışları, scrollback, kesinti ve onaylı shell
izinleri açıkça tasarlanmalı. Şu an Pi agent hiçbir shell komutu yürütmez;
terminal/PTY ve Pi HDMI ekran erişimi **çalışmıyor**. SSH'in yerel cihaza mı
uzak sunucuya mı yönelik olduğu da henüz kesin değil.

## 7. Wi‑Fi ve internet paylaşımı `[TASARIM]`

**Bugün:** ESP uygulamasında Wi‑Fi bağlanma/provisioning, Pi ile credential
aktarımı ve internet yönlendirme yok. Pi agent yalnız UART servisidir.
“Bir kez bağla” istek olarak kayıtlıdır, mevcut işlev değildir.

**Önerilen kullanım:** ESP ana cihaz olarak ağ adını/şifresini *tek seferlik*
onaylı provisioning arayüzünde alır, güvenli NVS alanında saklar; Pi ilk
eşleştirmede cihaz kimliğini onaylayıp bu bilgiyi **şifreli ve doğrulanmış**
bir eşleşme oturumundan alır, Raspberry Pi OS NetworkManager profiline
gerekli izinlerle kaydeder. Her ikisi sonraki açılışta aynı ağa kendi Wi‑Fi
radyosuyla bağlanır; kullanıcının parolayı tekrar girmesi gerekmez. Bu, tek
radyo/tek IP veya Pi'nin ESP üzerinden NAT yapması demek **değildir**. Pi
çıkarıldığında ESP ayarlarını korur. SSID/parola asla log, ekran, `.py` veya
CRC-only UART paketinde gönderilmez. Gerçek güvenlik için karşılıklı
kimlik doğrulama, kısa fiziksel eşleştirme penceresi, anahtar değişimi,
şifreli kanal, kullanıcı onayı, anahtar silme ve yeniden eşleştirme gerekir;
yalnız kablo bağlantısı güvenlik sağlamaz. NVS encryption/secure boot ve
Pi dosya izinleri nihai şemada ayrıca kararlaştırılmalı.

**Alternatif:** Yalnız tek uplink istendiğinde USB gadget Ethernet (ECM)
üzerinden bir taraf gateway, NAT/DNS/DHCP; ESP USB-host ECM desteği,
lwIP yönlendirme ve güç maliyeti ayrı prototip gerektirir. Birden fazla
uygulamanın ağını paylaşmak ile ağ *kimlik bilgisini* paylaşmak farklıdır.
Kullanıcının tercihi netleşmeden NAT veya düz metin parola taşıma eklenmez.

## 8. Donanım eşlemesi ve güvenlik açıkları

Önerilen modül ESP32-S3-WROOM-1 N16R8 (16 MiB flash, 8 MiB PSRAM),
GMT020/ST7789/SPI2, TCA8418/I²C `0x34`, MAX17048/I²C `0x36`, microSD/SPI3.
**Ekran kararı:** Kullanıcının elindeki GMT020 ST7789 kullanılacak; mantıksal
arayüz 320×240 kalacak. GMT020 tek parça numarası değil: örneğin
GMT020-02'nin 7 pinli ve ayrı arka ışık ucu bulunan 8 pinli modülleri
vardır; tam alt model/revizyon henüz verilmedi. 7P'de `CS`, `RST`, `DC`,
`SDA=MOSI`, `SCL=SCLK`, `VCC`, `GND` 4 telli SPI sinyalleridir;
**SDA/SCL I²C değildir**. 8P/diğer versiyonda BL, besleme ve panel offset'i
farklı olabilir. `CONFIG_CALCTUS_LCD_BL=-1` ayrı PWM hattı olmayan panel
için yeni güvenli varsayılandır: güç tuşu ST7789 display off/on yapar,
ara parlaklık değerleri **çalışmaz**. BL pini ve sürücü devresi gerçekten
varsa PWM GPIO'su Kconfig'te atanır. LCD X/Y RAM offset'i başlangıçta 0,
`swap_xy=y`, mirror X/Y opsiyoneldir; bunlar kart üzerinde renk/çerçeve
testiyle doğrulanmalıdır. 40 MHz SPI hızı da kablolama ve modüle göre
ölçülmelidir. Modüle göre 3,3 V lojik/besleme ve LED akım sınırlaması
veri sayfasıyla doğrulanmadan 5 V GPIO uygulanmaz.

Mevcut `platform/esp32/sdkconfig` eski `CONFIG_CALCTUS_LCD_BL=7` değerini
korur; yeni `-1` Kconfig varsayılanı bu dosyayı kendiliğinden değiştirmez.
7 pinli modülde `menuconfig` ile gerçek konfigürasyon ayrıca ayarlanmalıdır.
BL=7/UART kapalı ve BL=-1/UART açık yapılandırmaları ayrı ayrı çapraz
derlenmiştir; modül üzerinde ölçülmüş test değildir.

GMT020-02 için üretici referansı:
[GoldenMorning GMT020-02 240×320 ST7789V](https://goldenmorninglcd.com/tft-display-module/2-inch-240x320-st7789v-gmt020-02/).
Bu **kullanıcının kesin alt modeli olduğunu kanıtlamaz**.
`platform/esp32/main/Kconfig.projbuild` içindeki LCD 7–12, I²C 4–5,
SD 13–16, Pi UART TX 17/RX 18 yalnız **derleme örnekleri**; hiçbirine PCB
pinout'u denmez. SD kart FAT biçimli ve önceden hazırlanmış olmalı, otomatik
format yok. Pi güç rayı, USB data/OTG, ortak toprak, ESD, UART lojik ve
batarya bütçesi prototip PCB üzerinde ölçülmedi. Pi fiziksel olarak
bağlanmadan cihaz/ekran/SD/Wi‑Fi işlevi doğrulanmış sayılamaz.

UART1 varsayılan kapalı (`CONFIG_CALCTUS_PI_UART=n`). Gerçek bağlantı pinleri
çakışma denetiminden sonra `idf.py -C platform/esp32 menuconfig` ile açılır.
Linux agent çok genel yetkilerle root olarak çalıştırılmamalı; servis için
sınırlı kullanıcı ve port izni belirlenmeli. Otomatik shell/AI credential
aktarımına giden komut yok.

## 9. Test, derleme ve kanıt sınırı

`./sim` SDL arayüzünü açar; `./sim test` CTest/birim testleri ve golden ekran
karşılaştırmalarını çalıştırır. Pi agent testi Python subprocess üzerinden
protokol ping/info/kare/CRC'yi, `pi_link` byte doğrulaması/bağlı-değil
fallback'ini, `pi_app` Pi takılı/takılı değil UI karşılaştırmasını sınar.
`CALCTUS_PI_SIM=1 ./sim` Pi simülasyonunu açar; varsayılan kapalıdır.
`python3 platform/pi-zero-2/calctus_agent.py --stdio` yalnız test taşımasıdır;
fiziksel Pi üzerinde `/dev/serial0` ile çalıştırılabilir ama cihaz/pin testi
yapılmadı. `python3 tools/check_portable.py` ve `git diff --check` kod denetimi.

ESP için IDF 5.5.1 ortamında `idf.py -C platform/esp32 -B /özel/build build`.
Grafik eski fonksiyon örneklemesini (77 örnek, asimptot kopuklukları, pan ve
zoom) kullanır; grafik yüzeyi/formül/eksen/eğri LVGL nesneleridir.

**En son doğrulama (16 Kasım 2026, LVGL geçişi sonrası):** Tam CTest paketi
**35/35 geçti**; geçirilen ekranların golden görüntüleri eski Canvas
çıktısıyla yan yana incelenip güncellendi (fark: glif kenar yumuşatması ve ana
ekrana geri eklenen kaydırma göstergesi);
İstatistik hedefli testler **3/3 geçti**; `git diff --check` temiz. ESP32-S3
derlemesi bu değişikliklerden sonra çalıştırılamadı: mevcut ortamda `idf.py`
bulunmuyor. Önceki ESP imaj boyutları bu sürüm için geçerli doğrulama değildir.
Fiziksel GMT020/pin/offset, Pi bağlantısı, USB ekran aktarımı, Wi‑Fi paylaşımı
ve gerçek terminal hâlâ doğrulama veya uygulama bekliyor. Hesap için koyu
tema/uzun sonuç/RPN ve Çözücü için hatalı katsayı/bildirim kapanışı golden
testleri vardır. Çözücü `ui::Modal` açmaz; hatalarda LVGL bildirim katmanını
kullanır. Genel modal davranışı korunur ve görünümü `LvglScreen::modal`
oluşturur; Gallery kaydırma/seçim akışları da golden testlidir.

## 10. Karar kayıtları, eksikler ve gerçekleştirme sırası

**Alınmış kararlar:** Ortak HAL; yüksek hassasiyet için libbf; kullanıcı
dosyaları SD'de, NVS ayarlara; sabit MicroPython VM; 118 element; otomatik
SD format yok; Pi *opsiyonel*, ESP ana sistem; UART yalnız kontrol prototipi.
CALCTUS ana proje lisansı hâlâ seçilmedi (üçüncü taraf MIT bildirimleri var).

**Karar gerektirenler:** (1) Pi ekranı için uygulamaya özgü sanal 320×240 UI
yeterli mi, yoksa HDMI masaüstünün aynısı şart mı? (2) Pi terminali yerel PTY
mi yoksa SSH istemcisi mi; shell için izin/tehdit modeli? (3) Wi‑Fi
şifresinin *bir kez aktarılması* mı yoksa gerçekten *tek internet uplink*
paylaşılması mı isteniyor? (4) gerçek PCB GPIO'ları, USB OTG port erişimi,
ayrı Pi güç bütçesi/anahtarlama ve güç kapatma yöntemi? (5) AI sağlayıcı,
endpoint, model, ağ ve API anahtarı saklama ilkesi? Kimlik bilgileri repoya
yazılmamalı.

**Sıradaki teknik kilometre taşları:**

1. Pi UART'ını gerçek kartta doğrula (GPIO, Pi serial console, 3,3 V, güç).
2. USB OTG host↔gadget prototipinde sürekli 320×240 dirty-rect transfer,
   CRC, retry, kopma ve FPS ölçümlerini yap; yalnız buradan sonra ESP
   framebuffer köprüsü ve Pi sanal UI/tuş yönlendirmesini uygula.
3. Terminal/PTY metin arayüzünü güvenli ve Pi yokken yerel fallback ile ekle.
4. Wi‑Fi provisioning, doğrulanmış eşleştirme ve Pi NetworkManager kurulumunu
   kullanıcının seçtiği modele göre tasarla/uygula; gizli veriyi UART v1'e koyma.
5. AI servisi kararından sonra ağ/hata/çevrimdışı simülasyon ve fiziksel test.

Her aşamada sim testleri, ESP çapraz derlemesi ve gerçek kart ölçümleri farklı
kanıt kategorileridir. Hiçbiri öbürünün yerine yazılmamalıdır.
