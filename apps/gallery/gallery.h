// Bileşenler: arayüz kitinin parçalarını denemek için vitrin uygulaması.
#pragma once

#include "kernel/app.h"
#include "ui/widgets/list.h"
#include "ui/widgets/modal.h"
#include "ui/widgets/table.h"

namespace calctus::apps {

class GalleryApp final : public App, private ui::ListModel {
 public:
  const char* title() const override;
  View view() const override { return View::Gallery; }
  void on_open() override;
  bool on_key(const KeyEvent& ev) override;
  void soft_keys(SoftKeys& keys) const override;

  enum class Page : uint8_t { Menu, LongList, Table, Fonts };
  Page page() const { return page_; }
  const ui::ListView& menu() const { return menu_; }
  const ui::ListView& long_list() const { return long_list_; }
  const ui::TableView& table() const { return table_; }

 private:
  enum Row { kLongList, kTable, kFonts, kDialog, kPicker, kSlider, kSwitch, kToast, kRowCount };

  struct LongModel final : ui::ListModel {
    int count() const override { return 40; }
    void item(int index, ui::ListItem& out) const override;
    mutable char label[24] = {};
    mutable char value[32] = {};
  };

  struct PowersModel final : ui::TableModel {
    int rows() const override { return 30; }
    int cols() const override { return 4; }
    const char* header(int col) const override;
    void cell(int row, int col, char* buf, int size) const override;
  };

  int count() const override { return kRowCount; }
  void item(int index, ui::ListItem& out) const override;

  void activate(int row);
  void set_page(Page page);

  Page page_ = Page::Menu;
  ui::ListView menu_;
  ui::ListView long_list_;
  ui::TableView table_;
  LongModel long_model_;
  PowersModel powers_model_;

  ui::Dialog dialog_;
  ui::Picker picker_;
  ui::SliderModal slider_;

  int fruit_ = 0;
  int level_ = 40;
  bool switch_on_ = false;
  mutable char level_text_[8] = {};
};

}  // namespace calctus::apps
