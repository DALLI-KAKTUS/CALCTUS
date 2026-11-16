MicroPython v1.25.0 embed port kaynakları ve üretilmiş başlıkları.
Kaynak commit: `f498a16c7db6d4b2de200b3e0856528dfe0613c3`.
Lisans: `LICENSE` (MIT). Orijinal proje: https://github.com/micropython/micropython

`ports/embed/embed.mk` ile `services/micropython/mpconfigport.h` kullanılarak
üretilmiştir. `port/mphalport.c` yerine CALCTUS'un sınırlı çıktı yakalama
işlevi (`services/micropython/port.c`) derlenir. Kaynaklar derleme sırasında
indirilmez; ESP32-S3 ve simülatör aynı paket ve yapılandırmayı kullanır.
Yeniden üretme: `python3 tools/vendor_micropython.py /yol/micropython-v1.25.0`.
