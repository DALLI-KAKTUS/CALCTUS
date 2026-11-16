// Tuş kimlikleri ve tuş bilgisi. Kaynak: keymap.def
#pragma once

#include <cstdint>

namespace calctus {

enum class Key : uint8_t {
#define CALCTUS_KEY(id, name, label, kind, x, y, w, h) id,
#include "calctus/keymap.def"
#undef CALCTUS_KEY
  Count,
  None = 0xFF,
};

constexpr int kKeyCount = static_cast<int>(Key::Count);
static_assert(kKeyCount == 55, "CALCTUS tuş takımı 55 tuştur");

// Tuş matrisi 8 satır x 7 sütun (56 konum, 55'i dolu).
constexpr int kKeyMatrixCols = 7;
constexpr int kKeyMatrixRows = 8;

enum class KeyKind : uint8_t { Soft, Nav, Func, Num, Op, Exe };

struct KeyInfo {
  const char* name;   // betiklerde ve günlükte kullanılan ad
  const char* label;  // tuş üstü yazı (UTF-8)
  KeyKind kind;
  int16_t x, y, w, h;  // tuş takımı kökenine göre, 1x piksel
};

constexpr int kKeypadWidth = 320;
constexpr int kKeypadHeight = 374;

const KeyInfo& key_info(Key key);
Key key_from_name(const char* name);

constexpr int key_index(Key key) { return static_cast<int>(key); }
constexpr int key_row(Key key) { return key_index(key) / kKeyMatrixCols; }
constexpr int key_col(Key key) { return key_index(key) % kKeyMatrixCols; }

struct KeyEvent {
  Key key = Key::None;
  bool down = false;
  uint32_t time_ms = 0;
  bool repeat = false;  // basılı tutma tekrarı (çekirdek üretir)
};

}  // namespace calctus
