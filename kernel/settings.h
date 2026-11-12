// Sistem ayarları. Şimdilik yalnız bellekte; kalıcı saklama depolama servisiyle gelecek.
#pragma once

#include <cstdint>

namespace calctus {

enum class AngleUnit : uint8_t { Degree, Radian, Gradian };
enum class NumberFormat : uint8_t { Auto, Decimal, Scientific };

struct Settings {
  bool dark = false;
  uint8_t brightness = 100;  // yüzde
  AngleUnit angle = AngleUnit::Degree;
  NumberFormat format = NumberFormat::Auto;
};

constexpr const char* kVersion = "0.1 (S1)";

}  // namespace calctus
