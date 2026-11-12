#include "platform/sim/png.h"

#include <cstdio>
#include <vector>

namespace calctus::sim {

namespace {

uint32_t crc32(const uint8_t* data, size_t len, uint32_t crc = 0) {
  static uint32_t table[256];
  static bool ready = false;
  if (!ready) {
    for (uint32_t n = 0; n < 256; ++n) {
      uint32_t c = n;
      for (int k = 0; k < 8; ++k) c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
      table[n] = c;
    }
    ready = true;
  }
  crc = ~crc;
  for (size_t i = 0; i < len; ++i) crc = table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
  return ~crc;
}

void put32(std::vector<uint8_t>& out, uint32_t v) {
  out.push_back(static_cast<uint8_t>(v >> 24));
  out.push_back(static_cast<uint8_t>(v >> 16));
  out.push_back(static_cast<uint8_t>(v >> 8));
  out.push_back(static_cast<uint8_t>(v));
}

void chunk(std::vector<uint8_t>& out, const char type[4], const std::vector<uint8_t>& data) {
  put32(out, static_cast<uint32_t>(data.size()));
  const size_t start = out.size();
  out.insert(out.end(), type, type + 4);
  out.insert(out.end(), data.begin(), data.end());
  put32(out, crc32(out.data() + start, out.size() - start));
}

}  // namespace

bool write_png_rgb565(const char* path, const uint16_t* pixels, int width, int height) {
  // Ham satırlar: süzgeç baytı (0) + RGB.
  std::vector<uint8_t> raw;
  raw.reserve(static_cast<size_t>(height) * (static_cast<size_t>(width) * 3 + 1));
  for (int y = 0; y < height; ++y) {
    raw.push_back(0);
    for (int x = 0; x < width; ++x) {
      const uint16_t c = pixels[y * width + x];
      const int r = (c >> 11) & 0x1F, g = (c >> 5) & 0x3F, b = c & 0x1F;
      raw.push_back(static_cast<uint8_t>((r * 527 + 23) >> 6));
      raw.push_back(static_cast<uint8_t>((g * 259 + 33) >> 6));
      raw.push_back(static_cast<uint8_t>((b * 527 + 23) >> 6));
    }
  }

  // zlib: sıkıştırmasız (stored) bloklar.
  std::vector<uint8_t> z = {0x78, 0x01};
  uint32_t a = 1, b = 0;
  size_t pos = 0;
  while (pos < raw.size()) {
    const size_t n = raw.size() - pos < 65535 ? raw.size() - pos : 65535;
    z.push_back(pos + n == raw.size() ? 1 : 0);
    z.push_back(static_cast<uint8_t>(n & 0xFF));
    z.push_back(static_cast<uint8_t>(n >> 8));
    z.push_back(static_cast<uint8_t>(~n & 0xFF));
    z.push_back(static_cast<uint8_t>((~n >> 8) & 0xFF));
    for (size_t i = 0; i < n; ++i) {
      a = (a + raw[pos + i]) % 65521;
      b = (b + a) % 65521;
    }
    z.insert(z.end(), raw.begin() + static_cast<long>(pos), raw.begin() + static_cast<long>(pos + n));
    pos += n;
  }
  put32(z, (b << 16) | a);

  std::vector<uint8_t> out = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
  std::vector<uint8_t> ihdr;
  put32(ihdr, static_cast<uint32_t>(width));
  put32(ihdr, static_cast<uint32_t>(height));
  ihdr.insert(ihdr.end(), {8, 2, 0, 0, 0});
  chunk(out, "IHDR", ihdr);
  chunk(out, "IDAT", z);
  chunk(out, "IEND", {});

  std::FILE* f = std::fopen(path, "wb");
  if (f == nullptr) return false;
  const bool ok = std::fwrite(out.data(), 1, out.size(), f) == out.size();
  std::fclose(f);
  return ok;
}

}  // namespace calctus::sim
