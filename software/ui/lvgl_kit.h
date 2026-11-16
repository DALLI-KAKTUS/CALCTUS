// LVGL görünümlerinin ortak yardımcıları: bit eşlem yazı tipi köprüsü,
// RGB565 renk dönüşümü ve konumlu kutu/etiket kurucuları. Koordinatlar
// her zaman ebeveyn nesneye göredir.
#pragma once

#include "calctus/types.h"
#include "third_party/lvgl/lvgl.h"
#include "ui/canvas.h"
#include "ui/font.h"

namespace calctus::ui::lvk {

lv_font_t* face(const Font& font);
lv_font_t* face12();
lv_font_t* face16();
lv_font_t* face24();

lv_color_t color(Color rgb565);

// Var olan nesneyi düz, kenarlıksız, kaydırılmayan bir kutu yapar.
void rect(lv_obj_t* obj, int x, int y, int w, int h, Color bg, int radius);
// Yeni kutu; border > 0 ise o kalınlıkta kenarlık çizer.
lv_obj_t* box(lv_obj_t* parent, Rect r, Color bg, int radius, Color border = 0,
              int border_width = 0);

// 12 px etiket; konumu çağıran verir.
lv_obj_t* label(lv_obj_t* parent, const char* text, Color fg);
// Sol üst köşesi (x, y) olan, istenen yazı tipindeki etiket.
lv_obj_t* text(lv_obj_t* parent, int x, int y, const char* text, const Font& font, Color fg);
// Kutunun içinde yatay hizalı, dikeyde ortalı etiket (Canvas::text_in ile aynı yerleşim).
lv_obj_t* text_in(lv_obj_t* parent, Rect box, const char* text, const Font& font, Color fg,
                  Align align = Align::Center);

// Genişliğe sığmayan metnin baştan kırpılmış hali; sondaki rakamlar görünür kalır.
const char* visible_tail(const char* text, int width, const Font& font);
lv_obj_t* clipped_label(lv_obj_t* parent, const char* text, int width, const Font& font,
                        Color fg);

}  // namespace calctus::ui::lvk
