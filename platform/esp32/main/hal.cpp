#include "calctus/hal.h"
#include "tca8418.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>

extern "C" {
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/ledc.h"
#include "driver/spi_master.h"
#include "driver/uart.h"
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_lcd_io_spi.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_st7789.h"
#include "esp_timer.h"
#include "esp_vfs_fat.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "sdmmc_cmd.h"
#include "driver/sdspi_host.h"
#include "sdkconfig.h"
}

namespace calctus::hal {
namespace {

constexpr uint8_t kKeyAddress = 0x34; // TCA8418
constexpr uint8_t kGaugeAddress = 0x36; // MAX17048
constexpr uint8_t kRegConfig = 0x01;
constexpr uint8_t kRegInterrupt = 0x02;
constexpr uint8_t kRegEventCount = 0x03;
constexpr uint8_t kRegEvent = 0x04;
constexpr uint8_t kRegKeypad1 = 0x1d;
constexpr uint8_t kRegKeypad2 = 0x1e;

Color* fb = nullptr;
uint8_t* dma_line = nullptr;
esp_lcd_panel_handle_t panel = nullptr;
SemaphoreHandle_t transfer_done = nullptr;
i2c_master_dev_handle_t keypad = nullptr;
i2c_master_dev_handle_t gauge = nullptr;
bool nvs_ready = false;
#if CONFIG_CALCTUS_PI_UART
bool pi_uart_ready = false;
#endif
bool sd_mounted = false;
bool sd_bus_ready = false;
sdmmc_card_t* sd_card = nullptr;
constexpr const char* kSdRoot = "/sdcard";
#ifdef CONFIG_CALCTUS_LCD_SWAP_XY
constexpr bool kSwapXY = true;
#else
constexpr bool kSwapXY = false;
#endif
#ifdef CONFIG_CALCTUS_LCD_MIRROR_X
constexpr bool kMirrorX = true;
#else
constexpr bool kMirrorX = false;
#endif
#ifdef CONFIG_CALCTUS_LCD_MIRROR_Y
constexpr bool kMirrorY = true;
#else
constexpr bool kMirrorY = false;
#endif

bool transfer_complete(esp_lcd_panel_io_handle_t, esp_lcd_panel_io_event_data_t*, void*) {
  BaseType_t wake = pdFALSE;
  xSemaphoreGiveFromISR(transfer_done, &wake);
  return wake == pdTRUE;
}

bool write_register(uint8_t reg, uint8_t value) {
  const uint8_t bytes[] = {reg, value};
  return i2c_master_transmit(keypad, bytes, sizeof bytes, 50) == ESP_OK;
}

bool read_register(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t* data, size_t length) {
  return i2c_master_transmit_receive(dev, &reg, 1, data, length, 50) == ESP_OK;
}

void init_panel() {
  spi_bus_config_t bus{};
  bus.mosi_io_num = CONFIG_CALCTUS_LCD_MOSI;
  bus.miso_io_num = -1;
  bus.sclk_io_num = CONFIG_CALCTUS_LCD_SCLK;
  bus.quadwp_io_num = -1;
  bus.quadhd_io_num = -1;
  bus.max_transfer_sz = kScreenWidth * 2 + 8;
  ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO));

  transfer_done = xSemaphoreCreateBinary();
  if (!transfer_done) ESP_ERROR_CHECK(ESP_ERR_NO_MEM);
  esp_lcd_panel_io_spi_config_t io{};
  io.cs_gpio_num = CONFIG_CALCTUS_LCD_CS;
  io.dc_gpio_num = CONFIG_CALCTUS_LCD_DC;
  io.spi_mode = 0;
  io.pclk_hz = 40000000; // Validate signal integrity with the actual GMT020 wiring.
  io.trans_queue_depth = 1;
  io.on_color_trans_done = transfer_complete;
  io.lcd_cmd_bits = 8;
  io.lcd_param_bits = 8;
  esp_lcd_panel_io_handle_t panel_io = nullptr;
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI2_HOST, &io, &panel_io));

  esp_lcd_panel_dev_config_t config{};
  config.reset_gpio_num = CONFIG_CALCTUS_LCD_RST;
  config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
  config.bits_per_pixel = 16;
  ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(panel_io, &config, &panel));
  ESP_ERROR_CHECK(esp_lcd_panel_reset(panel));
  ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
  ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel, kSwapXY));
  ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel, kMirrorX, kMirrorY));
  ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel, CONFIG_CALCTUS_LCD_X_GAP,
                                        CONFIG_CALCTUS_LCD_Y_GAP));
  ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel, true));

  if (CONFIG_CALCTUS_LCD_BL < 0) return; // 7P modules may not expose LED control.
  ledc_timer_config_t timer{};
  timer.speed_mode = LEDC_LOW_SPEED_MODE;
  timer.duty_resolution = LEDC_TIMER_8_BIT;
  timer.timer_num = LEDC_TIMER_0;
  timer.freq_hz = 20000;
  timer.clk_cfg = LEDC_AUTO_CLK;
  ESP_ERROR_CHECK(ledc_timer_config(&timer));
  ledc_channel_config_t channel{};
  channel.gpio_num = CONFIG_CALCTUS_LCD_BL;
  channel.speed_mode = LEDC_LOW_SPEED_MODE;
  channel.channel = LEDC_CHANNEL_0;
  channel.timer_sel = LEDC_TIMER_0;
  ESP_ERROR_CHECK(ledc_channel_config(&channel));
}

void init_i2c() {
  i2c_master_bus_config_t bus{};
  bus.i2c_port = I2C_NUM_0;
  bus.sda_io_num = static_cast<gpio_num_t>(CONFIG_CALCTUS_I2C_SDA);
  bus.scl_io_num = static_cast<gpio_num_t>(CONFIG_CALCTUS_I2C_SCL);
  bus.clk_source = I2C_CLK_SRC_DEFAULT;
  bus.glitch_ignore_cnt = 7;
  bus.flags.enable_internal_pullup = true;
  i2c_master_bus_handle_t handle = nullptr;
  ESP_ERROR_CHECK(i2c_new_master_bus(&bus, &handle));
  i2c_device_config_t key_config{};
  key_config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
  key_config.device_address = kKeyAddress;
  key_config.scl_speed_hz = 100000;
  ESP_ERROR_CHECK(i2c_master_bus_add_device(handle, &key_config, &keypad));
  key_config.device_address = kGaugeAddress;
  ESP_ERROR_CHECK(i2c_master_bus_add_device(handle, &key_config, &gauge));

  // ROW0..ROW7 ve COL0..COL6 aktif, 8x7 matris; olay kodu TCA'nın 8x10 düzenindedir.
  if (!write_register(kRegKeypad1, 0xff) || !write_register(kRegKeypad2, 0x7f) ||
      !write_register(kRegConfig, 0x81) || !write_register(kRegInterrupt, 0xff))
    ESP_ERROR_CHECK(ESP_ERR_NOT_FOUND);
  if (CONFIG_CALCTUS_KEY_IRQ >= 0) {
    const auto pin = static_cast<gpio_num_t>(CONFIG_CALCTUS_KEY_IRQ);
    ESP_ERROR_CHECK(gpio_set_direction(pin, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_pull_mode(pin, GPIO_PULLUP_ONLY));
  }
  if (CONFIG_CALCTUS_CHARGE_GPIO >= 0) {
    const auto pin = static_cast<gpio_num_t>(CONFIG_CALCTUS_CHARGE_GPIO);
    ESP_ERROR_CHECK(gpio_set_direction(pin, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_pull_mode(pin, GPIO_PULLUP_ONLY));
  }
}

bool sd_filename(const char* name) {
  if (!name || !*name) return false;
  size_t i = 0;
  for (; i < sizeof(FileInfo::name) && name[i]; ++i) {
    const char c = name[i];
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.')) return false;
  }
  return i && i < sizeof(FileInfo::name) &&
         std::strcmp(name, ".") && std::strcmp(name, "..");
}

bool sd_path(const char* name, char (&path)[96], const char* suffix = "") {
  return sd_filename(name) &&
         std::snprintf(path, sizeof path, "%s/%s%s", kSdRoot, name, suffix) <
             static_cast<int>(sizeof path);
}

void init_sd() {
  sdmmc_host_t host = SDSPI_HOST_DEFAULT();
  host.slot = SPI3_HOST;
  host.max_freq_khz = 10000;
  spi_bus_config_t bus{};
  bus.mosi_io_num = CONFIG_CALCTUS_SD_MOSI;
  bus.miso_io_num = CONFIG_CALCTUS_SD_MISO;
  bus.sclk_io_num = CONFIG_CALCTUS_SD_SCLK;
  bus.quadwp_io_num = -1;
  bus.quadhd_io_num = -1;
  bus.max_transfer_sz = 4096;
  if (!sd_bus_ready) {
    if (spi_bus_initialize(SPI3_HOST, &bus, SPI_DMA_CH_AUTO) != ESP_OK) return;
    sd_bus_ready = true;
  }
  sdspi_device_config_t slot = SDSPI_DEVICE_CONFIG_DEFAULT();
  slot.gpio_cs = static_cast<gpio_num_t>(CONFIG_CALCTUS_SD_CS);
  slot.host_id = SPI3_HOST;
  if (CONFIG_CALCTUS_SD_CD >= 0)
    slot.gpio_cd = static_cast<gpio_num_t>(CONFIG_CALCTUS_SD_CD);
  esp_vfs_fat_sdmmc_mount_config_t mount{};
  mount.format_if_mount_failed = false; // Kullanıcının kartını ASLA otomatik biçimlendirme.
  mount.max_files = 5;
  const esp_err_t err = esp_vfs_fat_sdspi_mount(kSdRoot, &host, &slot, &mount, &sd_card);
  sd_mounted = err == ESP_OK;
  if (!sd_mounted) std::printf("[calctus] microSD baglanamadi: %s\n", esp_err_to_name(err));
}

bool fetch_key(KeyEvent& out) {
  uint8_t count = 0;
  if (!read_register(keypad, kRegEventCount, &count, 1) || (count & 0x0f) == 0) {
    write_register(kRegInterrupt, 0xff);
    return false;
  }
  uint8_t event = 0;
  if (!read_register(keypad, kRegEvent, &event, 1)) return false;
  const int index = esp32::key_index_from_tca8418(event);
  if (index < 0) return false;
  out.key = static_cast<Key>(index);
  out.down = (event & 0x80) != 0;
  out.time_ms = millis();
  out.repeat = false;
  return true;
}

} // namespace

void init() {
  fb = static_cast<Color*>(heap_caps_calloc(kScreenWidth * kScreenHeight,
                                              sizeof(Color), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  dma_line = static_cast<uint8_t*>(heap_caps_malloc(kScreenWidth * 2,
                                                     MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL));
  if (!fb || !dma_line) ESP_ERROR_CHECK(ESP_ERR_NO_MEM);
  init_panel();
  init_i2c();
  nvs_ready = nvs_flash_init() == ESP_OK;
  init_sd();
#if CONFIG_CALCTUS_PI_UART
  uart_config_t config{};
  config.baud_rate = 115200;
  config.data_bits = UART_DATA_8_BITS;
  config.parity = UART_PARITY_DISABLE;
  config.stop_bits = UART_STOP_BITS_1;
  config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
  if (uart_driver_install(UART_NUM_1, 256, 256, 0, nullptr, 0) == ESP_OK &&
      uart_param_config(UART_NUM_1, &config) == ESP_OK &&
      uart_set_pin(UART_NUM_1, CONFIG_CALCTUS_PI_TX, CONFIG_CALCTUS_PI_RX,
                   UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) == ESP_OK)
    pi_uart_ready = true;
#endif
}

Color* framebuffer() { return fb; }

void display_push(Rect area) {
  area = intersect(area, make_rect(0, 0, kScreenWidth, kScreenHeight));
  if (area.empty()) return;
  for (int y = area.y; y < area.bottom(); ++y) {
    for (int x = 0; x < area.w; ++x) {
      const Color color = fb[y * kScreenWidth + area.x + x];
      dma_line[x * 2] = static_cast<uint8_t>(color >> 8);
      dma_line[x * 2 + 1] = static_cast<uint8_t>(color);
    }
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel, area.x, y, area.right(), y + 1,
                                               dma_line));
    xSemaphoreTake(transfer_done, portMAX_DELAY);
  }
}

void backlight(uint8_t level) {
  if (CONFIG_CALCTUS_LCD_BL < 0) {
    // No PWM pin: display off/on works, intermediate brightness does not.
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel, level != 0));
    return;
  }
  ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, level));
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
}

bool keys_wait(KeyEvent& out, uint32_t timeout_ms) {
  const uint32_t start = millis();
  for (;;) {
    if (fetch_key(out)) return true;
    if (timeout_ms != kWaitForever && millis() - start >= timeout_ms) return false;
    const uint32_t remaining = timeout_ms == kWaitForever ? 10 :
                               timeout_ms - (millis() - start);
    const uint32_t delay_ms = remaining < 10 ? remaining : 10;
    vTaskDelay(pdMS_TO_TICKS(delay_ms ? delay_ms : 1) + 1);
  }
}

uint32_t millis() { return static_cast<uint32_t>(esp_timer_get_time() / 1000); }

int battery_percent() {
  uint8_t soc[2]{};
  if (!read_register(gauge, 0x04, soc, sizeof soc)) return 0;
  const int percent = soc[0] + (soc[1] >= 128 ? 1 : 0);
  return percent > 100 ? 100 : percent;
}

bool battery_charging() {
  if (CONFIG_CALCTUS_CHARGE_GPIO < 0) return false;
  return gpio_get_level(static_cast<gpio_num_t>(CONFIG_CALCTUS_CHARGE_GPIO)) == 0;
}

bool storage_read(const char* key, void* data, size_t size) {
  if (!nvs_ready || !key || !data || !size) return false;
  nvs_handle_t handle;
  if (nvs_open("calctus", NVS_READONLY, &handle) != ESP_OK) return false;
  size_t length = size;
  const bool ok = nvs_get_blob(handle, key, data, &length) == ESP_OK && length == size;
  nvs_close(handle);
  return ok;
}

bool storage_write(const char* key, const void* data, size_t size) {
  if (!nvs_ready || !key || !data || !size) return false;
  nvs_handle_t handle;
  if (nvs_open("calctus", NVS_READWRITE, &handle) != ESP_OK) return false;
  const bool ok = nvs_set_blob(handle, key, data, size) == ESP_OK &&
                  nvs_commit(handle) == ESP_OK;
  nvs_close(handle);
  return ok;
}

bool sd_ready() {
  if (!sd_mounted) init_sd();
  return sd_mounted;
}

bool sd_refresh() {
  if (sd_mounted) {
    esp_vfs_fat_sdcard_unmount(kSdRoot, sd_card);
    sd_mounted = false;
    sd_card = nullptr;
  }
  return sd_ready();
}

int files_list(FileInfo* out, int capacity) {
  if (!sd_ready() || !out || capacity < 0) return -1;
  DIR* dir = opendir(kSdRoot);
  if (!dir) return -1;
  int count = 0;
  while (dirent* entry = readdir(dir)) {
    if (!sd_filename(entry->d_name) || entry->d_name[0] == '.' ||
        count >= capacity) continue;
    char path[96];
    if (!sd_path(entry->d_name, path)) continue;
    struct stat s{};
    if (stat(path, &s) != 0 || !S_ISREG(s.st_mode)) continue;
    std::memcpy(out[count].name, entry->d_name, std::strlen(entry->d_name) + 1);
    out[count++].size = s.st_size >= 0 ? static_cast<uint32_t>(s.st_size) : 0;
  }
  closedir(dir);
  return count;
}

bool file_read(const char* name, char* data, size_t capacity, size_t& length) {
  length = 0;
  char path[96];
  if (!sd_ready() || !data || !capacity || !sd_path(name, path)) return false;
  FILE* file = std::fopen(path, "rb");
  if (!file) return false;
  const bool size_ok = std::fseek(file, 0, SEEK_END) == 0;
  const long size = size_ok ? std::ftell(file) : -1;
  if (size < 0 || static_cast<unsigned long>(size) >= capacity ||
      std::fseek(file, 0, SEEK_SET) != 0) { std::fclose(file); return false; }
  length = static_cast<size_t>(size);
  const bool ok = std::fread(data, 1, length, file) == length && std::fgetc(file) == EOF;
  std::fclose(file);
  if (ok) data[length] = '\0';
  else length = 0;
  return ok;
}

bool file_write(const char* name, const char* data, size_t length) {
  char path[96], temporary[96], backup[96];
  if (!sd_ready() || (!data && length) || !sd_path(name, path) ||
      !sd_path(name, temporary, ".tmp") || !sd_path(name, backup, ".bak")) return false;
  FILE* file = std::fopen(temporary, "wb");
  if (!file) return false;
  const bool written = std::fwrite(data, 1, length, file) == length;
  const bool closed = std::fclose(file) == 0;
  if (!written || !closed) { std::remove(temporary); return false; }
  struct stat old_stat{};
  const bool had_old = stat(path, &old_stat) == 0;
  const bool existing = had_old && std::rename(path, backup) == 0;
  if (had_old && !existing) { std::remove(temporary); return false; }
  if (std::rename(temporary, path) == 0) {
    if (existing) std::remove(backup);
    return true;
  }
  if (existing) std::rename(backup, path);
  std::remove(temporary);
  return false;
}

bool file_delete(const char* name) {
  char path[96];
  return sd_ready() && sd_path(name, path) && std::remove(path) == 0;
}

bool file_rename(const char* from, const char* to) {
  char old_path[96], new_path[96];
  struct stat s{};
  return sd_ready() && sd_path(from, old_path) && sd_path(to, new_path) &&
         stat(old_path, &s) == 0 && S_ISREG(s.st_mode) &&
         stat(new_path, &s) != 0 && std::rename(old_path, new_path) == 0;
}

bool pi_exchange(const uint8_t* tx, size_t tx_length, uint8_t* rx,
                 size_t capacity, size_t& rx_length, uint32_t timeout_ms) {
  rx_length = 0;
#if CONFIG_CALCTUS_PI_UART
  if (!pi_uart_ready || !tx || !rx || tx_length < 8 || capacity < 8) return false;
  uart_flush_input(UART_NUM_1);
  if (uart_write_bytes(UART_NUM_1, tx, tx_length) != static_cast<int>(tx_length)) return false;
  uint8_t header[6]{};
  size_t count = 0;
  const uint32_t start = millis();
  while (count < sizeof header && millis() - start < timeout_ms) {
    uint8_t byte = 0;
    if (uart_read_bytes(UART_NUM_1, &byte, 1, pdMS_TO_TICKS(10)) != 1) continue;
    if (count == 0 && byte != 0xa5) continue;
    if (count == 1 && byte != 0x5a) { count = byte == 0xa5 ? 1 : 0; continue; }
    header[count++] = byte;
  }
  if (count != 6 || header[5] > 48 || header[5] + 8 > capacity) return false;
  std::memcpy(rx, header, sizeof header);
  const size_t total = header[5] + 8;
  count = 6;
  while (count < total && millis() - start < timeout_ms) {
    const int received = uart_read_bytes(UART_NUM_1, rx + count, total - count,
                                          pdMS_TO_TICKS(10));
    if (received > 0) count += received;
  }
  rx_length = count == total ? total : 0;
  return rx_length != 0;
#else
  (void)tx; (void)tx_length; (void)rx; (void)capacity; (void)timeout_ms;
  return false;
#endif
}

bool should_exit() { return false; }

void log(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  std::vprintf(fmt, args);
  std::putchar('\n');
  va_end(args);
}

}  // namespace calctus::hal
