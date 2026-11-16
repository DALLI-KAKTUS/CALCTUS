#include "ui/lvgl_kit.h"

#include <cstdio>

namespace calctus::ui::lvk {
namespace {

bool glyph_dsc(const lv_font_t* f, lv_font_glyph_dsc_t* out, uint32_t codepoint,
               uint32_t) {
  const auto* face = static_cast<const Font*>(f->dsc);
  const Glyph* glyph = face->find(codepoint);
  if (!glyph) return false;
  out->adv_w = glyph->advance;
  out->box_w = glyph->w;
  out->box_h = glyph->h;
  out->ofs_x = glyph->xoff;
  out->ofs_y = -glyph->yoff - glyph->h;
  out->stride = 0;
  out->format = LV_FONT_GLYPH_FORMAT_A4;
  out->gid.index = static_cast<uint32_t>(glyph - face->glyphs);
  return true;
}

const void* glyph_bitmap(lv_font_glyph_dsc_t* dsc, lv_draw_buf_t* buf) {
  const auto* face = static_cast<const Font*>(dsc->resolved_font->dsc);
  const Glyph& glyph = face->glyphs[dsc->gid.index];
  const uint8_t* src = face->bitmap + glyph.offset;
  if (dsc->req_raw_bitmap) return src;
  if (!buf) return nullptr;
  const int stride = lv_draw_buf_width_to_stride(glyph.w, LV_COLOR_FORMAT_A8);
  for (int y = 0; y < glyph.h; ++y) {
    for (int x = 0; x < glyph.w; ++x) {
      const int p = y * glyph.w + x;
      const uint8_t nibble = (p & 1) ? (src[p / 2] & 15) : (src[p / 2] >> 4);
      buf->data[y * stride + x] = nibble * 17;
    }
  }
  lv_draw_buf_flush_cache(buf, nullptr);
  return buf;
}

lv_font_t* font_for(const Font& source, lv_font_t& face) {
  if (!face.dsc) {
    face.get_glyph_dsc = glyph_dsc;
    face.get_glyph_bitmap = glyph_bitmap;
    face.line_height = source.line_height;
    face.base_line = face.line_height - source.ascent;
    face.dsc = &source;
  }
  return &face;
}

}  // namespace

lv_font_t* face12() { static lv_font_t f{}; return font_for(font_sans12, f); }
lv_font_t* face16() { static lv_font_t f{}; return font_for(font_sans16, f); }
lv_font_t* face24() { static lv_font_t f{}; return font_for(font_sans24, f); }

lv_font_t* face(const Font& font) {
  if (&font == &font_sans24) return face24();
  if (&font == &font_sans16) return face16();
  return face12();
}

lv_color_t color(Color rgb565) {
  return lv_color_make((rgb565 >> 11) * 255 / 31,
                       ((rgb565 >> 5) & 63) * 255 / 63, (rgb565 & 31) * 255 / 31);
}

void rect(lv_obj_t* obj, int x, int y, int w, int h, Color bg, int radius) {
  lv_obj_set_pos(obj, x, y);
  lv_obj_set_size(obj, w, h);
  lv_obj_set_style_bg_color(obj, color(bg), 0);
  lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(obj, radius, 0);
  lv_obj_set_style_border_width(obj, 0, 0);
  lv_obj_set_style_pad_all(obj, 0, 0);
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

lv_obj_t* box(lv_obj_t* parent, Rect r, Color bg, int radius, Color border,
              int border_width) {
  auto* obj = lv_obj_create(parent);
  rect(obj, r.x, r.y, r.w, r.h, bg, radius);
  if (border_width > 0) {
    lv_obj_set_style_border_width(obj, border_width, 0);
    lv_obj_set_style_border_color(obj, color(border), 0);
  }
  return obj;
}

lv_obj_t* label(lv_obj_t* parent, const char* text, Color fg) {
  auto* obj = lv_label_create(parent);
  lv_label_set_text(obj, text);
  lv_obj_set_style_text_color(obj, color(fg), 0);
  lv_obj_set_style_text_font(obj, face12(), 0);
  return obj;
}

lv_obj_t* text(lv_obj_t* parent, int x, int y, const char* text, const Font& font, Color fg) {
  auto* obj = label(parent, text, fg);
  if (&font != &font_sans12) lv_obj_set_style_text_font(obj, face(font), 0);
  lv_obj_set_pos(obj, x, y);
  return obj;
}

lv_obj_t* text_in(lv_obj_t* parent, Rect box, const char* s, const Font& font, Color fg,
                  Align align) {
  const int width = Canvas::text_width(s, font);
  int x = box.x;
  if (align == Align::Center) x = box.x + (box.w - width) / 2;
  else if (align == Align::Right) x = box.right() - width;
  return text(parent, x, box.y + (box.h - font.line_height) / 2, s, font, fg);
}

// Eski hesap görünümü de en sağdaki rakamları görünür tutuyordu. Uzun sonuçları
// LVGL etiketi içinde gizlemeden önce aynı davranışı koru.
const char* visible_tail(const char* text, int width, const Font& font) {
  while (*text && Canvas::text_width(text, font) > width) {
    // UTF-8 karakterinin ortasından kesme.
    do ++text; while ((*text & 0xC0) == 0x80);
  }
  return text;
}

lv_obj_t* clipped_label(lv_obj_t* parent, const char* text, int width,
                        const Font& font, Color fg) {
  if (Canvas::text_width(text, font) <= width) return label(parent, text, fg);
  const char* tail = visible_tail(text, width - Canvas::text_width("…", font), font);
  char shown[160];
  std::snprintf(shown, sizeof shown, "…%s", tail);
  return label(parent, shown, fg);
}

}  // namespace calctus::ui::lvk
