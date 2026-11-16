// Olasılık, Diziler, Python, Dosyalar, Bileşenler ve Tuş testi görünümleri.
// Koordinatlar içerik alanına göredir (durum çubuğunun altı y = 0).
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string_view>

#include "apps/files/files.h"
#include "apps/gallery/gallery.h"
#include "apps/keytest/keytest.h"
#include "apps/probability/probability.h"
#include "apps/python/python.h"
#include "apps/sequences/sequences.h"
#include "kernel/system.h"
#include "math/eval.h"
#include "ui/lvgl_kit.h"
#include "ui/lvgl_screen.h"
#include "ui/theme.h"

namespace calctus::ui {
namespace {

using namespace lvk;

constexpr int kTop = System::kStatusH;

Rect local(Rect r) { return make_rect(r.x, r.y - kTop, r.w, r.h); }

lv_obj_t* prepare(lv_obj_t* view, const Theme& t) {
  lv_obj_clean(view);
  lv_obj_set_style_bg_color(view, color(t.bg), 0);
  return view;
}

// Görünmez, içindekileri kendi sınırında kırpan kap.
lv_obj_t* clip_area(lv_obj_t* parent, Rect r) {
  auto* obj = box(parent, r, 0, 0);
  lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);
  return obj;
}

// Kutunun sağına yaslı metin; sığmazsa baştan kırpılır ve sonu görünür kalır.
void right_in(lv_obj_t* parent, int width, const char* s, const Font& font, Color fg,
              int pad) {
  auto* obj = clipped_label(parent, s, width - 2 * pad, font, fg);
  lv_obj_set_style_text_font(obj, face(font), 0);
  lv_obj_align(obj, LV_ALIGN_RIGHT_MID, -pad, 0);
}

// Satırın width pikseline sığan sonu; sondan geriye doğru tek geçişte ölçer.
std::string_view fitting_tail(std::string_view s, int width, const Font& font) {
  size_t start = s.size();
  int used = 0;
  while (start > 0) {
    size_t p = start - 1;
    while (p > 0 && (static_cast<uint8_t>(s[p]) & 0xC0) == 0x80) --p;
    const int w = Canvas::text_width(s.substr(p, start - p), font);
    if (used + w > width) break;
    used += w;
    start = p;
  }
  return s.substr(start);
}

void copy_line(char* out, size_t size, std::string_view s) {
  const size_t n = s.size() < size - 1 ? s.size() : size - 1;
  std::memcpy(out, s.data(), n);
  out[n] = '\0';
}

void list_view(lv_obj_t* parent, const ListView& list, const Theme& t) {
  const Rect bounds = local(list.bounds());
  if (list.on_surface()) box(parent, bounds, t.surface, 0);
  const ListModel* model = list.model();
  if (model == nullptr) return;

  const int n = model->count();
  const int rows = list.visible_rows();
  const bool scrollbar = n > rows;
  for (int i = list.first_visible(); i < n && i < list.first_visible() + rows; ++i) {
    ListItem item;
    model->item(i, item);
    const Rect r = local(list.row_rect(i));
    if (i == list.selected()) {
      box(parent, r, t.select, 0);
      box(parent, make_rect(r.x, r.y, 3, r.h), t.accent, 0);
    }
    int right = r.right() - 10 - (scrollbar ? 5 : 0);
    switch (item.mark) {
      case ListItem::Mark::Chevron:
        right -= 8;
        text_in(parent, make_rect(right, r.y, 8, r.h), "›", font_sans12, t.muted);
        right -= 6;
        break;
      case ListItem::Mark::Checked:
        right -= 12;
        text_in(parent, make_rect(right, r.y, 12, r.h), "✓", font_sans12, t.accent);
        right -= 6;
        break;
      case ListItem::Mark::On:
      case ListItem::Mark::Off: {
        const bool on = item.mark == ListItem::Mark::On;
        const Rect pill = make_rect(right - 26, r.y + (r.h - 14) / 2, 26, 14);
        box(parent, pill, on ? t.accent : t.line, 7);
        box(parent, make_rect(on ? pill.right() - 12 : pill.x + 2, pill.y + 2, 10, 10),
            t.surface, 5);
        right -= 32;
        break;
      }
      case ListItem::Mark::None:
        break;
    }
    if (item.value != nullptr) {
      text_in(parent, make_rect(r.x + 10, r.y, right - (r.x + 10), r.h), item.value,
              font_sans12, t.muted, Align::Right);
    }
    text_in(parent, make_rect(r.x + 10, r.y, r.w - 20, r.h), item.label, font_sans12,
            item.enabled ? t.text : t.disabled, Align::Left);
  }

  if (scrollbar) {
    const int track_h = rows * ListView::kRowH;
    int thumb_h = track_h * rows / n;
    if (thumb_h < 8) thumb_h = 8;
    const int thumb_y = bounds.y + (track_h - thumb_h) * list.first_visible() / (n - rows);
    box(parent, make_rect(bounds.right() - 5, thumb_y, 3, thumb_h), t.muted, 1);
  }
}

void table_view(lv_obj_t* parent, const TableView& table, const Theme& t) {
  const TableModel* model = table.model();
  if (model == nullptr) return;
  const Rect bounds = local(table.bounds());
  const int top = table.first_visible();
  const int visible = table.visible_rows();

  box(parent, make_rect(bounds.x, bounds.y, bounds.w, TableView::kRowH), t.select, 0);
  for (int col = 0; col < model->cols(); ++col) {
    Rect r = local(table.cell_rect(top, col));
    r.y = bounds.y;
    text_in(parent, r, model->header(col), font_sans12, t.text);
  }

  char buf[32];
  for (int row = top; row < model->rows() && row < top + visible; ++row) {
    box(parent, make_rect(bounds.x + 6, local(table.cell_rect(row, 0)).bottom() - 1,
                          bounds.w - 12, 1), t.line, 0);
    for (int col = 0; col < model->cols(); ++col) {
      model->cell(row, col, buf, sizeof buf);
      text_in(parent, local(table.cell_rect(row, col)), buf, font_sans12, t.text);
    }
  }

  // Seçili hücre en son kurulur ki kutusu çizgilerin üstünde kalsın.
  if (table.row() >= top && table.row() < top + visible) {
    const Rect r = local(table.cell_rect(table.row(), table.col()));
    model->cell(table.row(), table.col(), buf, sizeof buf);
    box(parent, make_rect(r.x, r.y - 1, r.w, r.h + 1), t.surface, 3, t.accent, 1);
    text_in(parent, r, buf, font_sans12, t.text);
  }
}

}  // namespace

void LvglScreen::probability(const apps::ProbabilityApp& app) {
  const Theme& t = theme();
  lv_obj_t* view = prepare(root(App::View::Probability), t);
  text(view, 8, 5, app.mode_title(), font_sans16, t.text);
  text(view, 8, 37, "n =", font_sans16, t.text);
  text(view, 8, 69, "k =", font_sans16, t.text);
  const bool k = app.k_selected();
  box(view, make_rect(50, 33, 260, 27), k ? t.surface : t.select, 4, k ? t.line : t.accent, 1);
  box(view, make_rect(50, 65, 260, 27), k ? t.select : t.surface, 4, k ? t.accent : t.line, 1);
  text(view, 58, 37, *app.n() ? app.n() : "_", font_sans16, t.text);
  text(view, 58, 69, *app.k() ? app.k() : "_", font_sans16, t.text);
  box(view, make_rect(0, 100, 320, 1), t.line, 0);
  text(view, 8, 106, "SONUÇ (TAM SAYI)", font_sans12, t.muted);
  const char* result = app.result();
  if (!*result) {
    text(view, 8, 128, "EXE ile hesapla", font_sans12, t.muted);
    return;
  }
  // Büyük tam sayılar 32 basamaklık satırlara bölünür.
  const size_t length = std::strlen(result);
  for (size_t start = 0; start < length; start += 32) {
    char line[33];
    copy_line(line, sizeof line, std::string_view(result + start, length - start));
    text(view, 8, 128 + static_cast<int>(start / 32) * 19, line, font_sans12, t.text);
  }
}

void LvglScreen::sequences(const apps::SequencesApp& app) {
  const Theme& t = theme();
  lv_obj_t* view = prepare(root(App::View::Sequences), t);
  text(view, 8, 4, app.explicit_mode() ? "Açık: u(n) = f(n)" : "Yineleme: u(0), u(n)=f(n,Ans)",
       font_sans12, t.text);
  const char* names[] = {"f(n)", "u(0)", "İlk n"};
  for (int i = 0; i < 3; ++i) {
    const int y = 22 + i * 27;
    text(view, 8, y + 5, names[i], font_sans12, t.muted);
    const bool selected = app.selected() == i;
    auto* field = box(view, make_rect(62, y, 250, 24), selected ? t.select : t.surface, 4,
                      selected ? t.accent : t.line, 1);
    right_in(field, 250, *app.field(i) ? app.field(i) : "_", font_sans12, t.text, 9);
  }
  box(view, make_rect(0, 105, 320, 1), t.line, 0);
  text(view, 8, 107, "n", font_sans12, t.muted);
  text(view, 65, 107, "u(n)", font_sans12, t.muted);
  if (!app.has_rows()) {
    text(view, 65, 139, "F6/EXE ile hesapla", font_sans12, t.muted);
    return;
  }
  const unsigned first = static_cast<unsigned>(std::strtoul(app.field(2), nullptr, 10));
  for (int i = 0; i < apps::SequencesApp::kRows; ++i) {
    const int y = 124 + i * 15;
    char index[12], compact[128];
    std::snprintf(index, sizeof index, "%u", first + static_cast<unsigned>(i));
    text(view, 8, y, index, font_sans12, t.muted);
    const char* value = app.row(i);
    if (Canvas::text_width(value, font_sans12) > 230 &&
        math::evaluate("Ans", value, NumberFormat::Scientific, compact, sizeof compact) ==
            math::Error::None) value = compact;
    right_in(clip_area(view, make_rect(65, y, 245, 15)), 245, value, font_sans12, t.text, 5);
  }
}

void LvglScreen::python(const apps::PythonApp& app) {
  const Theme& t = theme();
  lv_obj_t* view = prepare(root(App::View::Python), t);
  char line[160];
  if (!app.showing_output()) {
    text(view, 8, 5, "MicroPython 1.25 · .py / microSD", font_sans12, t.muted);
    if (app.script_count() < 0) {
      text(view, 12, 50, "microSD bağlı değil", font_sans16, t.text);
      text(view, 12, 85, "F2 ile örnek programı çalıştır", font_sans12, t.muted);
      return;
    }
    if (app.script_count() == 0) {
      text(view, 12, 50, "Kökte .py dosyası yok", font_sans16, t.text);
      text(view, 12, 85, "Dosyalar'da .py oluşturun", font_sans12, t.muted);
      return;
    }
    const int first = app.selected() >= 7 ? app.selected() - 7 : 0;
    for (int i = first; i < app.script_count() && i < first + 8; ++i) {
      const int y = 24 + (i - first) * 21;
      const bool selected = i == app.selected();
      if (selected) box(view, make_rect(5, y, 310, 21), t.select, 4);
      std::snprintf(line, sizeof line, "%.31s · %lu B", app.script(i).name,
                    static_cast<unsigned long>(app.script(i).size));
      text(view, 12, y + 3, line, font_sans12, selected ? t.accent : t.text);
    }
    return;
  }

  const python::Result& output = app.output();
  std::snprintf(line, sizeof line, "%s: %s%s", app.running(), output.ok ? "Tamam" : "Hata",
                output.truncated ? " (kesildi)" : "");
  text(view, 8, 5, line, font_sans12, output.ok ? t.text : t.accent);
  box(view, make_rect(8, 22, 304, 1), t.line, 0);
  if (!output.length) {
    text(view, 10, 35, "(çıktı yok)", font_sans12, t.muted);
    return;
  }
  const int scroll = app.scroll();
  int number = 0;
  const char* start = output.text;
  for (const char* p = output.text;; ++p) {
    if (*p == '\n' || *p == '\0') {
      if (number >= scroll && number < scroll + apps::PythonApp::kOutputLines) {
        // Uzun satır kendi kabında sağdan kırpılır.
        copy_line(line, sizeof line, std::string_view(start, p - start));
        text(clip_area(view, make_rect(8, 28 + (number - scroll) * 18, 304, 18)), 2, 0, line,
             font_sans12, t.text);
      }
      ++number;
      if (*p == '\0') break;
      start = p + 1;
    }
  }
}

void LvglScreen::files(const apps::FilesApp& app) {
  using Page = apps::FilesApp::Page;
  const Theme& t = theme();
  lv_obj_t* view = prepare(root(App::View::Files), t);
  char line[160];

  if (app.page() == Page::List) {
    if (app.count() < 0) {
      text(view, 12, 34, "microSD kart bağlı değil", font_sans16, t.text);
      text(view, 12, 67, "Kartı takıp F5 ile yenile", font_sans12, t.muted);
      return;
    }
    text(view, 8, 4, "microSD / (en çok 32 giriş)", font_sans12, t.muted);
    const int first = app.selected() >= 6 ? app.selected() - 6 : 0;
    for (int i = first; i <= app.count() && i < first + 7; ++i) {
      const int y = 23 + (i - first) * 24;
      const bool selected = i == app.selected();
      if (selected) box(view, make_rect(5, y, 310, 23), t.select, 4);
      if (i == app.count()) std::snprintf(line, sizeof line, "+ Yeni belge");
      else std::snprintf(line, sizeof line, "%s (%lu B)", app.entry(i).name,
                         static_cast<unsigned long>(app.entry(i).size));
      text(view, 12, y + 5, line, font_sans12, selected ? t.accent : t.text);
    }
    return;
  }

  if (app.page() == Page::Rename) {
    text(view, 8, 9, "BELGE ADI (en çok 31 karakter)", font_sans12, t.muted);
    auto* field = box(view, make_rect(8, 37, 304, 31), t.surface, 4, t.accent, 1);
    text(field, 7, 6, app.rename_text(), font_sans16, t.text);
    text(view, 8, 86, "EXE kaydet · F6 iptal", font_sans12, t.muted);
    return;
  }

  const files::Document& doc = app.document();
  std::snprintf(line, sizeof line, "%s%s · %u/4095", doc.name, app.dirty() ? "*" : "",
                static_cast<unsigned>(doc.length));
  text(view, 8, 3, line, font_sans12, t.text);
  box(view, make_rect(8, 22, 304, 1), t.line, 0);

  // Son dokuz satır görünür; imleç belgenin sonundadır.
  constexpr int kLines = 9;
  size_t starts[kLines]{};
  int count = 1;
  for (size_t i = 0; i < doc.length; ++i) {
    if (doc.text[i] != '\n') continue;
    if (count < kLines) {
      starts[count++] = i + 1;
    } else {
      for (int j = 1; j < kLines; ++j) starts[j - 1] = starts[j];
      starts[kLines - 1] = i + 1;
    }
  }
  for (int row = 0; row < count; ++row) {
    size_t end = starts[row];
    while (end < doc.length && doc.text[end] != '\n') ++end;
    const std::string_view full(doc.text + starts[row], end - starts[row]);
    // Satır sığmazsa yazılan son kısım sağa yaslı görünür.
    const bool long_line = Canvas::text_width(full, font_sans12) > 300;
    const std::string_view shown = long_line ? fitting_tail(full, 300, font_sans12) : full;
    copy_line(line, sizeof line, shown);
    const int x = long_line ? 300 - Canvas::text_width(line, font_sans12) : 2;
    text(clip_area(view, make_rect(8, 27 + row * 17, 304, 17)), x, 1, line, font_sans12,
         t.text);
  }
  text(view, 8, 183, app.symbol_layer() ? "FN: 1=: 3=' 0=boşluk" :
                     app.alpha() ? "Harf: 2=abc 3=def · Sağ: sabitle" :
                                   "Rakam · ALPHA: harf · FN: simge", font_sans12, t.muted);
}

void LvglScreen::gallery(const apps::GalleryApp& app) {
  using Page = apps::GalleryApp::Page;
  const Theme& t = theme();
  lv_obj_t* view = prepare(root(App::View::Gallery), t);
  switch (app.page()) {
    case Page::Menu:
      list_view(view, app.menu(), t);
      break;
    case Page::LongList:
      list_view(view, app.long_list(), t);
      break;
    case Page::Table:
      table_view(view, app.table(), t);
      break;
    case Page::Fonts: {
      int y = 8;
      text(view, 10, y, "24 px  Pijamalı hasta", font_sans24, t.text);
      y += 34;
      text(view, 10, y, "16 px  yağız şoföre çabucak güvendi", font_sans16, t.text);
      y += 26;
      text(view, 10, y, "12 px  ÇĞİÖŞÜ çğıöşü 0123456789 +−×÷=≠≈", font_sans12, t.text);
      y += 20;
      text(view, 10, y, "√2 · π ≈ 4,4429  ∫ f(x) dx  ∑ n²  x² + y² = r²", font_sans12, t.text);
      y += 20;
      text(view, 10, y, "α β γ δ θ λ μ σ φ ω  Δ Σ Ω  ← → ↑ ↓  ½ ¼ ¾", font_sans12, t.text);
      y += 26;
      text(view, 10, y, "sin²θ + cos²θ = 1", font_sans16, t.accent);
      y += 26;
      text(view, 10, y, "Geri: menüye dön", font_sans12, t.muted);
      break;
    }
  }
}

void LvglScreen::keytest(const apps::KeyTestApp& app) {
  const Theme& t = theme();
  lv_obj_t* view = prepare(root(App::View::KeyTest), t);
  char line[64];

  text(view, 8, 6, "Son tuş", font_sans12, t.muted);
  const Rect last_box = make_rect(8, 24, 152, 62);
  box(view, last_box, t.surface, 6, app.last_down() ? t.accent : t.line, 1);
  if (app.last() == Key::None) {
    text_in(view, last_box, "bir tuşa bas", font_sans12, t.muted);
    text(view, 8, 92, "Fare ya da klavye", font_sans12, t.muted);
  } else {
    const KeyInfo& info = key_info(app.last());
    text_in(view, last_box, info.label, font_sans24, t.text);
    std::snprintf(line, sizeof line, "%s · %s", info.name,
                  app.last_down() ? "basılı" : "bırakıldı");
    text(view, 8, 92, line, font_sans12, t.text);
    std::snprintf(line, sizeof line, "Matris: satır %d, sütun %d", key_row(app.last()),
                  key_col(app.last()));
    text(view, 8, 110, line, font_sans12, t.text);
  }

  std::snprintf(line, sizeof line, "Görülen: %d / %d", app.seen_count(), kKeyCount);
  text(view, 8, 128, line, font_sans12, t.text);
  std::snprintf(line, sizeof line, "Basış: %u", static_cast<unsigned>(app.presses()));
  text(view, 8, 146, line, font_sans12, t.text);
  std::snprintf(line, sizeof line, "Süre: %u:%02u", app.elapsed_s() / 60, app.elapsed_s() % 60);
  text(view, 8, 164, line, font_sans12, t.text);
  if (app.seen_count() == kKeyCount) text(view, 8, 182, "✓ Tüm tuşlar çalışıyor", font_sans12, t.accent);
  else text(view, 8, 182, "Çıkış: ⌂ tuşuna iki kez", font_sans12, t.muted);

  // Mini tuş haritası: tuş takımının 0,44 ölçekli kopyası.
  auto scale = [](int v) { return v * 11 / 25; };
  for (int i = 0; i < kKeyCount; ++i) {
    const KeyInfo& k = key_info(static_cast<Key>(i));
    const Rect r = make_rect(172 + scale(k.x), 8 + scale(k.y), scale(k.w), scale(k.h));
    if (app.down(i)) box(view, r, t.accent, 2);
    else if (app.seen(i)) box(view, r, t.select, 2);
    else box(view, r, t.bg, 2, t.line, 1);
  }
}

}  // namespace calctus::ui
