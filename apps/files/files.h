#pragma once

#include "kernel/app.h"
#include "services/files.h"

namespace calctus::apps {

class FilesApp final : public App {
 public:
  const char* title() const override { return "Dosyalar"; }
  void on_open() override;
  bool on_key(const KeyEvent& event) override;
  void draw(ui::Canvas& canvas, Rect area) override;
  void soft_keys(SoftKeys& keys) const override;

 private:
  enum class Page { List, Edit, Rename };
  void refresh();
  void create();
  void open_selected();
  bool persist();
  void append(char c, const KeyEvent& event);

  Page page_ = Page::List;
  hal::FileInfo entries_[files::kMaxEntries]{};
  int count_ = 0;
  int selected_ = 0;
  files::Document doc_{};
  char original_[files::kNameCapacity]{};
  char rename_[files::kNameCapacity]{};
  size_t rename_length_ = 0;
  size_t text_length_ = 0;
  bool alpha_ = false;
  bool symbol_ = false;
  bool uppercase_ = false;
  bool dirty_ = false;
  bool confirm_ = false;
  Key last_key_ = Key::None;
  uint32_t last_at_ = 0;
};

}  // namespace calctus::apps
