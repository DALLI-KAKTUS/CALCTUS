// Ayarlar uygulaması.
#pragma once

#include "kernel/app.h"
#include "ui/widgets/list.h"
#include "ui/widgets/modal.h"

namespace calctus::apps {

class SettingsApp final : public App, private ui::ListModel {
 public:
  const char* title() const override { return page_ == Page::About ? "Hakkında" : "Ayarlar"; }
  View view() const override { return View::Settings; }
  void on_open() override;
  bool on_key(const KeyEvent& ev) override;
  void draw(ui::Canvas& c, Rect area) override;
  void soft_keys(SoftKeys& keys) const override;
  bool about_page() const { return page_ == Page::About; }
  int selected_row() const { return list_.selected(); }
  void row(int index, ui::ListItem& out) const { item(index, out); }

 private:
  enum class Page : uint8_t { Main, About };
  enum Row { kTheme, kBrightness, kAngle, kFormat, kAbout, kRowCount };

  int count() const override { return kRowCount; }
  void item(int index, ui::ListItem& out) const override;

  void activate(int row);
  void set_page(Page page);

  Page page_ = Page::Main;
  ui::ListView list_;
  ui::Picker picker_;
  ui::SliderModal slider_;
  mutable char brightness_text_[8] = {};
};

}  // namespace calctus::apps
