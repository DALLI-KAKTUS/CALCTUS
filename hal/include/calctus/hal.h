// HAL: donanım soyutlama sözleşmesi.
//
// Bu dosyanın üstündeki her şey (çekirdek, arayüz, uygulamalar) simülatörde ve
// ESP32-S3'te aynı kaynak koddur. Burada bildirilen işlevleri her platform kendi
// klasöründe gerçekler: platform/sim (masaüstü) ve platform/esp32 (cihaz).
//
// Sözleşme ESP32-S3'ün sınırlarına göre yazılır; simülatör ona uyar.
#pragma once

#include <cstdint>

#include "calctus/keys.h"
#include "calctus/types.h"

namespace calctus::hal {

constexpr int kScreenWidth = 320;
constexpr int kScreenHeight = 240;
constexpr uint32_t kWaitForever = 0xFFFFFFFFu;

// Açılışta bir kez çağrılır.
void init();

// --- Ekran ---------------------------------------------------------------
// Çerçeve belleği: kScreenWidth * kScreenHeight piksel, satır satır.
// Platform ayırır (ESP32'de PSRAM'de durur).
Color* framebuffer();

// Çerçeve belleğinin verilen bölgesini panele gönderir. Yalnız değişen bölge
// gönderilmelidir: cihazda her piksel SPI üzerinden gider.
void display_push(Rect area);

// Arka ışık: 0 kapalı, 255 tam parlak.
void backlight(uint8_t level);

// --- Tuşlar --------------------------------------------------------------
// Bir tuş olayı gelene ya da süre dolana kadar bekler. Olay geldiyse true.
// Cihazda işlemci bu çağrının içinde uyur; boşta güç tüketimini bu belirler.
bool keys_wait(KeyEvent& out, uint32_t timeout_ms);

// --- Zaman ---------------------------------------------------------------
// Açılıştan beri geçen süre (ms). Taşar; fark alınarak kullanılır.
uint32_t millis();

// --- Pil -----------------------------------------------------------------
int battery_percent();  // 0..100
bool battery_charging();

// --- Sistem --------------------------------------------------------------
// Ana döngü bitmeli mi? Cihazda hep false; simülatörde pencere kapanınca true.
bool should_exit();

void log(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

}  // namespace calctus::hal
