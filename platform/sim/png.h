// Küçük PNG yazıcı (sıkıştırmasız). Yalnız simülatörün ekran görüntüleri için.
#pragma once

#include <cstdint>

namespace calctus::sim {

bool write_png_rgb565(const char* path, const uint16_t* pixels, int width, int height);

}  // namespace calctus::sim
