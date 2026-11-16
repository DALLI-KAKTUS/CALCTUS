#pragma once

#include "kernel/app.h"
#include "services/files.h"

namespace calctus::apps {

class FilesApp final : public App {
 public:
  const char* title() const override { return "Dosyalar"; }
  View view() const override { return View::Files; }
  void on_open() override;
  bool on_key(const KeyEvent& event) override;
  void soft_keys(SoftKeys& keys) const override;

  enum class Page { List, Edit, Rename };
  Page page() const { return page_; }
  int count() const { return count_; }  // <0: microSD yok
  const hal::FileInfo& entry(int index) const { return entries_[index]; }
  int selected() const { return selected_; }
  const files::Document& document() const { return doc_; }
  bool dirty() const { return dirty_; }
  const char* rename_text() const { return rename_; }
  bool alpha() const { return alpha_; }
  bool symbol_layer() const { return symbol_; }

 private:
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
