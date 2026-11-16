# Lisanslar ve üçüncü taraf bileşenler

## CALCTUS'un kendi lisansları

| Kapsam | Lisans | Metin |
|---|---|---|
| Yazılım (`software/`, üçüncü taraf klasörleri hariç) | AGPL-3.0-or-later | [`LICENSE`](LICENSE), [`LICENSES/AGPL-3.0-or-later.txt`](LICENSES/AGPL-3.0-or-later.txt) |
| Devre (`hardware/`) | CERN-OHL-S-2.0 | [`hardware/LICENSE`](hardware/LICENSE) |
| Kasa ve 3B modeller (`mechanical/`) | CERN-OHL-S-2.0 | [`mechanical/LICENSE`](mechanical/LICENSE) |
| Belgeler (`docs/`, README dosyaları) | CC BY-SA 4.0 | [`docs/LICENSE`](docs/LICENSE) |

Copyright (C) 2026 CALCTUS katkıcıları.

## Üçüncü taraf bileşenler

Bunlar kendi lisanslarıyla gelir; CALCTUS lisansı onların yerine geçmez.
Hepsi AGPL-3.0 ile birlikte dağıtılabilir.

| Bileşen | Yer | Lisans | Telif |
|---|---|---|---|
| libbf 2025-06-03 | `software/third_party/libbf/` | MIT (dosya başlıklarında) | Fabrice Bellard |
| LVGL v9.4.0 | `software/third_party/lvgl/` | MIT ([`LICENCE.txt`](software/third_party/lvgl/LICENCE.txt)); içindeki alt kütüphaneler için [`COPYRIGHTS.md`](software/third_party/lvgl/COPYRIGHTS.md) | LVGL Kft |
| MicroPython v1.25.0 (embed portu) | `software/third_party/micropython_embed/` | MIT ([`LICENSE`](software/third_party/micropython_embed/LICENSE)); yalnız MIT lisanslı `py/`, `extmod/`, `shared/` kısımları alındı | Damien P. George ve katkıcılar |
| DejaVu Sans yazı tipi | `software/ui/fonts/` (bit eşlem olarak üretildi) | Bitstream Vera lisansı + kamu malı DejaVu değişiklikleri ([`LICENSES/DejaVu-Fonts.txt`](LICENSES/DejaVu-Fonts.txt)) | Bitstream, Tavmjong Bah, DejaVu katkıcıları |
| Periyodik tablo verisi | `software/apps/elements/data.cpp` | CC BY-SA 3.0 ([`LICENSES/CC-BY-SA-3.0.txt`](LICENSES/CC-BY-SA-3.0.txt)) | [Bowserinator/Periodic-Table-JSON](https://github.com/Bowserinator/Periodic-Table-JSON), commit `a78c7a48` |

### Periyodik tablo verisi hakkında

`data.cpp` yalnız olgusal alanları (atom numarası, sembol, ad, yaklaşık kütle,
periyot, grup, blok, faz) içerir. Kaynak veri seti CC BY-SA 3.0 ile
lisanslıdır; bu dosya da aynı lisansla, yukarıdaki atıfla dağıtılır ve
yazılımın geri kalanından ayrı lisanslı bir veri dosyasıdır.
