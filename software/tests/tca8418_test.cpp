#include <cstdio>

#include "platform/esp32/main/tca8418.h"

int main() {
  using calctus::esp32::key_index_from_tca8418;
  for (int i = 0; i < calctus::kKeyCount; ++i) {
    const uint8_t code = static_cast<uint8_t>(i / 7 * 10 + i % 7 + 1);
    if (key_index_from_tca8418(code) != i ||
        key_index_from_tca8418(static_cast<uint8_t>(code | 0x80)) != i) return 1;
  }
  const uint8_t invalid[] = {0, 8, 10, 77, 78, 79, 80, 97};
  for (uint8_t code : invalid)
    if (key_index_from_tca8418(code) != -1) return 2;
  std::puts("TCA8418: 55 tuşun basma/bırakma kodları ve sınırlar geçti");
  return 0;
}
