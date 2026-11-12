#include "apps/gallery/gallery.h"

#include <cstdio>

#include "kernel/system.h"
#include "ui/theme.h"

namespace calctus::apps {

namespace {

const char* const kFruits[] = {"Elma", "Armut", "Kayısı", "Şeftali", "İncir", "Üzüm", "Nar"};
constexpr int kFruitCount = 7;

}  // namespace

void GalleryApp::LongModel::item(int index, ui::ListItem& out) const {
  std::snprintf(label, sizeof label, "Satır %d", index + 1);
  std::snprintf(value, sizeof value, "%d²=%d", index + 1, (index + 1) * (index + 1));
  out.label = label;
  out.value = value;
}

const char* GalleryApp::PowersModel::header(int col) const {
  static const char* const kHeaders[] = {"n", "n²", "n³", "2ⁿ"};
  return kHeaders[col];
}

void GalleryApp::PowersModel::cell(int row, int col, char* buf, int size) const {
  const long n = row + 1;
  long v = n;
  if (col == 1) v = n * n;
  if (col == 2) v = n * n * n;
  if (col == 3) v = 1L << n;
  std::snprintf(buf, static_cast<size_t>(size), "%ld", v);
}

const char* GalleryApp::title() const {
  switch (page_) {
    case Page::LongList:
      return "Uzun liste";
    case Page::Table:
      return "Tablo";
    case Page::Fonts:
      return "Yazı tipleri";
    case Page::Menu:
      break;
  }
  return "Bileşenler";
}

void GalleryApp::on_open() {
  page_ = Page::Menu;
  menu_.setup(this, System::kContent);
  menu_.select(0);
  long_list_.setup(&long_model_, System::kContent);
  long_list_.select(0);
  table_.setup(&powers_model_, System::kContent);
}

void GalleryApp::item(int index, ui::ListItem& out) const {
  using Mark = ui::ListItem::Mark;
  switch (index) {
    case kLongList:
      out.label = "Uzun liste";
      out.mark = Mark::Chevron;
      break;
    case kTable:
      out.label = "Tablo";
      out.mark = Mark::Chevron;
      break;
    case kFonts:
      out.label = "Yazı tipleri";
      out.mark = Mark::Chevron;
      break;
    case kDialog:
      out.label = "Onay penceresi";
      break;
    case kPicker:
      out.label = "Seçim listesi";
      out.value = kFruits[fruit_];
      break;
    case kSlider:
      std::snprintf(level_text_, sizeof level_text_, "%d", level_);
      out.label = "Kaydırıcı";
      out.value = level_text_;
      break;
    case kSwitch:
      out.label = "Anahtar";
      out.mark = switch_on_ ? Mark::On : Mark::Off;
      break;
    case kToast:
      out.label = "Bildirim";
      break;
    default:
      break;
  }
}

void GalleryApp::set_page(Page page) {
  page_ = page;
  System::get().invalidate_all();
}

void GalleryApp::activate(int row) {
  System& sys = System::get();
  switch (row) {
    case kLongList:
      set_page(Page::LongList);
      break;
    case kTable:
      set_page(Page::Table);
      break;
    case kFonts:
      set_page(Page::Fonts);
      break;
    case kDialog:
      dialog_.setup("Onay", "Bu yalnız bir deneme penceresi. Seçimin bildirim olarak gösterilecek.",
                    "Vazgeç", "Tamam",
                    [](void*, int button) {
                      System::get().toast(button == 1   ? "Tamam seçildi"
                                          : button == 0 ? "Vazgeç seçildi"
                                                        : "Geri tuşuyla kapatıldı");
                    },
                    nullptr);
      sys.show(dialog_);
      break;
    case kPicker:
      picker_.setup("Meyve seç", kFruits, kFruitCount, fruit_,
                    [](void* ctx, int index) {
                      static_cast<GalleryApp*>(ctx)->fruit_ = index;
                      System::get().invalidate_content();
                    },
                    this);
      sys.show(picker_);
      break;
    case kSlider:
      slider_.setup("Seviye", level_, 0, 100, 5, "",
                    [](void* ctx, int value) {
                      static_cast<GalleryApp*>(ctx)->level_ = value;
                      System::get().invalidate_content();
                    },
                    this);
      sys.show(slider_);
      break;
    case kSwitch:
      switch_on_ = !switch_on_;
      sys.invalidate(menu_.row_rect(row));
      break;
    case kToast:
      sys.toast("Bu bir bildirim");
      break;
    default:
      break;
  }
}

bool GalleryApp::on_key(const KeyEvent& ev) {
  if (!ev.down) return false;
  System& sys = System::get();

  if (page_ != Page::Menu && ev.key == Key::Back) {
    set_page(Page::Menu);
    return true;
  }

  switch (page_) {
    case Page::Menu:
      switch (menu_.on_key(ev)) {
        case ui::ListView::Result::Moved:
          sys.invalidate(menu_.damage());
          return true;
        case ui::ListView::Result::Activated:
          activate(menu_.selected());
          return true;
        case ui::ListView::Result::Ignored:
          break;
      }
      return ev.key == Key::Up || ev.key == Key::Down;
    case Page::LongList:
      if (long_list_.on_key(ev) == ui::ListView::Result::Moved) sys.invalidate(long_list_.damage());
      return true;
    case Page::Table:
      if (table_.on_key(ev)) sys.invalidate(table_.damage());
      return true;
    case Page::Fonts:
      return false;
  }
  return false;
}

void GalleryApp::draw(ui::Canvas& c, Rect area) {
  const ui::Theme& t = ui::theme();
  switch (page_) {
    case Page::Menu:
      menu_.draw(c);
      break;
    case Page::LongList:
      long_list_.draw(c);
      break;
    case Page::Table:
      table_.draw(c);
      break;
    case Page::Fonts: {
      c.fill_rect(area, t.bg);
      int y = area.y + 8;
      c.text(10, y, "24 px  Pijamalı hasta", ui::font_sans24, t.text);
      y += 34;
      c.text(10, y, "16 px  yağız şoföre çabucak güvendi", ui::font_sans16, t.text);
      y += 26;
      c.text(10, y, "12 px  ÇĞİÖŞÜ çğıöşü 0123456789 +−×÷=≠≈", ui::font_sans12, t.text);
      y += 20;
      c.text(10, y, "√2 · π ≈ 4,4429  ∫ f(x) dx  ∑ n²  x² + y² = r²", ui::font_sans12, t.text);
      y += 20;
      c.text(10, y, "α β γ δ θ λ μ σ φ ω  Δ Σ Ω  ← → ↑ ↓  ½ ¼ ¾", ui::font_sans12, t.text);
      y += 26;
      c.text(10, y, "sin²θ + cos²θ = 1", ui::font_sans16, t.accent);
      y += 26;
      c.text(10, y, "Geri: menüye dön", ui::font_sans12, t.muted);
      break;
    }
  }
}

void GalleryApp::soft_keys(SoftKeys&) const {}

}  // namespace calctus::apps
