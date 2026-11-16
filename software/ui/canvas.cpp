#include "ui/canvas.h"

namespace calctus::ui {

namespace {

// Saydamlık 0..15. Kanallar ayrı ayrı karıştırılır.
Color blend(Color bg, Color fg, int a) {
  const int ia = 15 - a;
  const int r = (((fg >> 11) & 0x1F) * a + ((bg >> 11) & 0x1F) * ia + 7) / 15;
  const int g = (((fg >> 5) & 0x3F) * a + ((bg >> 5) & 0x3F) * ia + 7) / 15;
  const int b = ((fg & 0x1F) * a + (bg & 0x1F) * ia + 7) / 15;
  return static_cast<Color>((r << 11) | (g << 5) | b);
}

// Yuvarlak köşede, kenardan k. satırın içeri çekilme miktarı.
int corner_inset(int radius, int k) {
  const int dy = radius - 1 - k;
  const int limit = (radius - 1) * (radius - 1) + (radius - 1);
  int dx = 0;
  while ((dx + 1) * (dx + 1) + dy * dy <= limit) ++dx;
  return (radius - 1) - dx;
}

}  // namespace

Canvas::Canvas(Color* pixels, int width, int height)
    : px_(pixels), w_(width), h_(height), clip_(make_rect(0, 0, width, height)) {}

void Canvas::set_clip(Rect clip) { clip_ = intersect(clip, make_rect(0, 0, w_, h_)); }

void Canvas::reset_clip() { clip_ = make_rect(0, 0, w_, h_); }

void Canvas::fill(Color c) { fill_rect(make_rect(0, 0, w_, h_), c); }

void Canvas::fill_rect(Rect r, Color c) {
  r = intersect(r, clip_);
  if (r.empty()) return;
  for (int y = r.y; y < r.bottom(); ++y) {
    Color* row = px_ + y * w_ + r.x;
    for (int x = 0; x < r.w; ++x) row[x] = c;
  }
  dirty_ = unite(dirty_, r);
}

void Canvas::frame_rect(Rect r, Color c) {
  if (r.empty()) return;
  fill_rect(make_rect(r.x, r.y, r.w, 1), c);
  fill_rect(make_rect(r.x, r.bottom() - 1, r.w, 1), c);
  fill_rect(make_rect(r.x, r.y, 1, r.h), c);
  fill_rect(make_rect(r.right() - 1, r.y, 1, r.h), c);
}

void Canvas::fill_round_rect(Rect r, int radius, Color c) {
  if (r.empty()) return;
  if (radius * 2 > r.w) radius = r.w / 2;
  if (radius * 2 > r.h) radius = r.h / 2;
  for (int k = 0; k < r.h; ++k) {
    int inset = 0;
    if (k < radius) {
      inset = corner_inset(radius, k);
    } else if (k >= r.h - radius) {
      inset = corner_inset(radius, r.h - 1 - k);
    }
    fill_rect(make_rect(r.x + inset, r.y + k, r.w - 2 * inset, 1), c);
  }
}

void Canvas::box(Rect r, int radius, Color border, Color fill) {
  fill_round_rect(r, radius, border);
  fill_round_rect(make_rect(r.x + 1, r.y + 1, r.w - 2, r.h - 2), radius > 0 ? radius - 1 : 0,
                  fill);
}

void Canvas::glyph(int left, int top, const Glyph& g, const Font& font, Color c) {
  const Rect area = intersect(make_rect(left, top, g.w, g.h), clip_);
  if (area.empty()) return;
  const uint8_t* data = font.bitmap + g.offset;
  for (int y = area.y; y < area.bottom(); ++y) {
    const int gy = y - top;
    Color* row = px_ + y * w_;
    for (int x = area.x; x < area.right(); ++x) {
      const int index = gy * g.w + (x - left);
      const uint8_t byte = data[index >> 1];
      const int a = (index & 1) ? (byte & 0x0F) : (byte >> 4);
      if (a == 0) continue;
      row[x] = (a == 15) ? c : blend(row[x], c, a);
    }
  }
  dirty_ = unite(dirty_, area);
}

int Canvas::text(int x, int y, std::string_view s, const Font& font, Color c) {
  int pen = x;
  while (!s.empty()) {
    const uint32_t cp = utf8_next(s);
    const Glyph* g = font.find(cp);
    if (g == nullptr) g = font.find('?');
    if (g == nullptr) continue;
    glyph(pen + g->xoff, y + font.ascent + g->yoff, *g, font, c);
    pen += g->advance;
  }
  return pen - x;
}

int Canvas::text_width(std::string_view s, const Font& font) {
  int width = 0;
  while (!s.empty()) {
    const uint32_t cp = utf8_next(s);
    const Glyph* g = font.find(cp);
    if (g == nullptr) g = font.find('?');
    if (g != nullptr) width += g->advance;
  }
  return width;
}

int Canvas::text_wrapped(Rect box, std::string_view s, const Font& font, Color c) {
  const int space = text_width(" ", font);
  int x = box.x, y = box.y;
  while (!s.empty()) {
    if (s[0] == '\n') {
      x = box.x;
      y += font.line_height;
      s.remove_prefix(1);
      continue;
    }
    if (s[0] == ' ') {
      x += space;
      s.remove_prefix(1);
      continue;
    }
    size_t n = 0;
    while (n < s.size() && s[n] != ' ' && s[n] != '\n') ++n;
    const std::string_view word = s.substr(0, n);
    const int w = text_width(word, font);
    if (x > box.x && x + w > box.right()) {
      x = box.x;
      y += font.line_height;
    }
    text(x, y, word, font, c);
    x += w;
    s.remove_prefix(n);
  }
  return y + font.line_height - box.y;
}

void Canvas::text_in(Rect box, std::string_view s, const Font& font, Color c, Align align) {
  const int width = text_width(s, font);
  int x = box.x;
  if (align == Align::Center) {
    x = box.x + (box.w - width) / 2;
  } else if (align == Align::Right) {
    x = box.right() - width;
  }
  const int y = box.y + (box.h - font.line_height) / 2;
  text(x, y, s, font, c);
}

}  // namespace calctus::ui
