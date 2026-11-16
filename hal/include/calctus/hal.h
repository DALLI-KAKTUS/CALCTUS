// HAL: donanım soyutlama sözleşmesi.
//
// Bu dosyanın üstündeki her şey (çekirdek, arayüz, uygulamalar) simülatörde ve
// ESP32-S3'te aynı kaynak koddur. Burada bildirilen işlevleri her platform kendi
// klasöründe gerçekler: platform/sim (masaüstü) ve platform/esp32 (cihaz).
//
// Sözleşme ESP32-S3'ün sınırlarına göre yazılır; simülatör ona uyar.
#pragma once

#include <cstdint>
#include <cstddef>

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
// LVGL canvas geçiş katmanı: eski uygulama içeriği için ayrı RGB565 tampon.
Color* legacy_framebuffer();

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

// --- Kalıcı saklama -------------------------------------------------------
// Anahtar başına sabit boyutlu kayıt. Platform dosya/NVS ayrıntılarını saklar.
// Yoksa veya boyut uyuşmazsa false; uygulama güvenli varsayılanlara döner.
bool storage_read(const char* key, void* data, size_t size);
bool storage_write(const char* key, const void* data, size_t size);

// --- microSD ---------------------------------------------------------------
// Kökteki sıradan dosyalar. Yol ayracı kabul edilmez; platform birimi yol
// geçişini tekrar denetler. Card yoksa liste -1, diğer işlemler false döner.
// Okuma tüm dosyayı alır; kapasiteyi aşan dosya kesilmeden reddedilir.
struct FileInfo { char name[32]; uint32_t size; };
bool sd_ready();
bool sd_refresh(); // çıkarılan/takılan kartı yeniden bağlamayı dene
int files_list(FileInfo* out, int capacity);
bool file_read(const char* name, char* data, size_t capacity, size_t& length);
bool file_write(const char* name, const char* data, size_t length);
bool file_rename(const char* from, const char* to);
bool file_delete(const char* name);

// --- Opsiyonel Raspberry Pi Zero 2 W bağlantısı --------------------------
// UART isteği/yanıtı; Pi veya bağlantı yoksa false. Gelen yanıt uzunluğu
// capacity'yi aşarsa reddedilir. Protokol services/pi_link.* içinde.
bool pi_exchange(const uint8_t* tx, size_t tx_length, uint8_t* rx,
                 size_t capacity, size_t& rx_length, uint32_t timeout_ms);

// --- Sistem --------------------------------------------------------------
// Ana döngü bitmeli mi? Cihazda hep false; simülatörde pencere kapanınca true.
bool should_exit();

void log(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

}  // namespace calctus::hal
