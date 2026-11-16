#include "apps/files/files.h"

#include <cstdio>
#include <cstring>

#include "kernel/system.h"

namespace calctus::apps {
namespace {

const char* letters(Key key) {
  switch (key) {
    case Key::N2: return "abc"; case Key::N3: return "def";
    case Key::N4: return "ghi"; case Key::N5: return "jkl";
    case Key::N6: return "mno"; case Key::N7: return "pqrs";
    case Key::N8: return "tuv"; case Key::N9: return "wxyz";
    case Key::N0: return " "; case Key::N1: return "1";
    default: return nullptr;
  }
}

char literal(Key key) {
  switch (key) {
    case Key::N0: return '0'; case Key::N1: return '1';
    case Key::N2: return '2'; case Key::N3: return '3';
    case Key::N4: return '4'; case Key::N5: return '5';
    case Key::N6: return '6'; case Key::N7: return '7';
    case Key::N8: return '8'; case Key::N9: return '9';
    case Key::Dot: return '.'; case Key::Plus: return '+';
    case Key::Minus: return '-'; case Key::Mul: return '*';
    case Key::Div: return '/'; case Key::Pow: return '^';
    case Key::LParen: return '('; case Key::RParen: return ')';
    case Key::Comma: return ','; case Key::Var: return '=';
    case Key::Xnt: return 'x'; case Key::EE: return 'e';
    default: return 0;
  }
}

char symbol(Key key) {
  switch (key) {
    case Key::N0: return ' '; case Key::N1: return ':';
    case Key::N2: return '='; case Key::N3: return '\'';
    case Key::N4: return '"'; case Key::N5: return '[';
    case Key::N6: return ']'; case Key::N7: return '<';
    case Key::N8: return '>'; case Key::N9: return '_';
    default: return literal(key);
  }
}

}  // namespace

void FilesApp::refresh() {
  count_ = files::list(entries_, files::kMaxEntries);
  if (count_ >= 0 && selected_ > count_) selected_ = count_;
  System::get().invalidate_content();
}

void FilesApp::on_open() {
  page_ = Page::List;
  confirm_ = false;
  refresh();
}

void FilesApp::create() {
  if (count_ < 0) { System::get().toast("microSD bağlı değil"); return; }
  if (count_ >= files::kMaxEntries) { System::get().toast("Liste dolu (32)"); return; }
  doc_ = {};
  original_[0] = '\0';
  for (int i = 1; i < 1000; ++i) {
    std::snprintf(doc_.name, sizeof doc_.name, "not%d.txt", i);
    bool used = false;
    for (int j = 0; j < count_; ++j)
      if (std::strcmp(entries_[j].name, doc_.name) == 0) used = true;
    if (!used) break;
  }
  text_length_ = 0;
  dirty_ = true;
  page_ = Page::Edit;
  alpha_ = false;
  symbol_ = false;
  confirm_ = false;
  last_key_ = Key::None;
  System::get().invalidate_all();
}

void FilesApp::open_selected() {
  if (count_ < 0) { System::get().toast("microSD bağlı değil"); return; }
  if (selected_ == count_) { create(); return; }
  if (!files::load(entries_[selected_].name, doc_)) {
    System::get().toast("Metin dışı veya >4095 B"); return;
  }
  std::memcpy(original_, doc_.name, sizeof original_);
  text_length_ = doc_.length;
  dirty_ = false;
  page_ = Page::Edit;
  alpha_ = false;
  symbol_ = false;
  confirm_ = false;
  last_key_ = Key::None;
  System::get().invalidate_all();
}

bool FilesApp::persist() {
  if (original_[0] && std::strcmp(original_, doc_.name)) {
    if (!files::rename(original_, doc_.name)) {
      System::get().toast("Ad değiştirilemedi"); return false;
    }
    if (!files::save(doc_)) {
      files::rename(doc_.name, original_);
      System::get().toast("Yazma hatası"); return false;
    }
  } else if (!files::save(doc_)) {
    System::get().toast("Yazma hatası"); return false;
  }
  std::memcpy(original_, doc_.name, sizeof original_);
  dirty_ = false;
  refresh();
  System::get().toast("Kaydedildi");
  return true;
}

void FilesApp::append(char c, const KeyEvent& event) {
  if (page_ == Page::Rename) {
    if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
          c == '.' || c == '_' || c == '-')) return;
  }
  confirm_ = false;
  char* buffer = page_ == Page::Rename ? rename_ : doc_.text;
  const size_t capacity = page_ == Page::Rename ? sizeof rename_ : sizeof doc_.text;
  size_t& length = page_ == Page::Rename ? rename_length_ : text_length_;
  const char* group = alpha_ ? letters(event.key) : nullptr;
  if (group && std::strlen(group) > 1 && last_key_ == event.key &&
      event.time_ms - last_at_ < 650 && length) {
    char previous = buffer[length - 1];
    if (previous >= 'A' && previous <= 'Z') previous = static_cast<char>(previous - 'A' + 'a');
    const char* found = std::strchr(group, previous);
    if (found) {
      c = group[(found - group + 1) % std::strlen(group)];
      if (uppercase_ && page_ != Page::Rename) c = static_cast<char>(c - 'a' + 'A');
      buffer[length - 1] = c;
      dirty_ = true;
      last_at_ = event.time_ms;
      System::get().invalidate_content();
      return;
    }
  }
  if (length + 1 >= capacity) { System::get().toast("Editör dolu"); return; }
  buffer[length++] = c;
  buffer[length] = '\0';
  if (page_ == Page::Edit) doc_.length = length;
  dirty_ = true;
  last_key_ = group ? event.key : Key::None;
  last_at_ = event.time_ms;
  System::get().invalidate_content();
}

bool FilesApp::on_key(const KeyEvent& event) {
  if (!event.down) return false;
  System& sys = System::get();
  if (page_ == Page::List) {
    if (event.key == Key::Up || event.key == Key::Down) {
      if (count_ >= 0)
        selected_ = (selected_ + count_ + 1 + (event.key == Key::Up ? -1 : 1)) %
                    (count_ + 1);
      confirm_ = false;
      sys.invalidate_content();
      return true;
    }
    if (event.repeat) return true;
    switch (event.key) {
      case Key::F1: create(); return true;
      case Key::F2: case Key::Ok: case Key::Exe: open_selected(); return true;
      case Key::F3:
        if (count_ < 0 || selected_ == count_) return true;
        if (!confirm_) { confirm_ = true; sys.toast("Silmek için F3 tekrar"); }
        else if (files::erase(entries_[selected_].name)) {
          confirm_ = false; refresh(); sys.toast("Silindi");
        } else sys.toast("Silme hatası");
        return true;
      case Key::F5: hal::sd_refresh(); refresh(); return true;
      case Key::F6: sys.go_home(); return true;
      default: return false;
    }
  }
  if (event.repeat) return true;
  if (event.key == Key::Alpha || event.key == Key::F2) {
    alpha_ = !alpha_; symbol_ = false;
    last_key_ = Key::None; sys.invalidate_all(); return true;
  }
  if (event.key == Key::Fn) {
    symbol_ = !symbol_; alpha_ = false;
    last_key_ = Key::None; sys.invalidate_content(); return true;
  }
  if (event.key == Key::Shift) {
    uppercase_ = !uppercase_; last_key_ = Key::None; sys.invalidate_content(); return true;
  }
  if (page_ == Page::Rename) {
    if (event.key == Key::F6 || event.key == Key::Back) {
      page_ = Page::Edit; last_key_ = Key::None; sys.invalidate_all(); return true;
    }
    if (event.key == Key::Exe || event.key == Key::Ok || event.key == Key::F1) {
      if (!files::valid_name(rename_)) { sys.toast("Geçersiz ad"); return true; }
      for (int i = 0; i < count_; ++i)
        if (std::strcmp(rename_, entries_[i].name) == 0 &&
            std::strcmp(rename_, original_) != 0) {
          sys.toast("Ad kullanılıyor"); return true;
        }
      std::memcpy(doc_.name, rename_, sizeof rename_);
      dirty_ = true;
      page_ = Page::Edit;
      last_key_ = Key::None;
      sys.invalidate_all(); return true;
    }
  } else {
    if (event.key == Key::F1) { persist(); return true; }
    if (event.key == Key::F4) {
      std::memcpy(rename_, doc_.name, sizeof rename_);
      rename_length_ = std::strlen(rename_);
      page_ = Page::Rename; last_key_ = Key::None; sys.invalidate_all(); return true;
    }
    if (event.key == Key::F6 || event.key == Key::Back) {
      if (dirty_ && !persist()) return true;
      page_ = Page::List; sys.invalidate_all(); return true;
    }
    if (event.key == Key::F5) {
      if (!confirm_) { confirm_ = true; sys.toast("Temizlemek için F5 tekrar"); }
      else { doc_.text[0] = '\0'; doc_.length = 0; text_length_ = 0;
             dirty_ = true; confirm_ = false; last_key_ = Key::None;
             sys.invalidate_content(); }
      return true;
    }
    if (event.key == Key::Exe || event.key == Key::F3) {
      append('\n', event); last_key_ = Key::None; return true;
    }
  }
  if (event.key == Key::Backspace) {
    char* buffer = page_ == Page::Rename ? rename_ : doc_.text;
    size_t& length = page_ == Page::Rename ? rename_length_ : text_length_;
    if (length) { buffer[--length] = '\0'; if (page_ == Page::Edit) doc_.length = length;
                  dirty_ = true; sys.invalidate_content(); }
    last_key_ = Key::None;
    return true;
  }
  if (event.key == Key::Right) { last_key_ = Key::None; return true; }
  char c = 0;
  if (symbol_) c = symbol(event.key);
  else if (alpha_ && letters(event.key)) {
    c = letters(event.key)[0];
    if (uppercase_ && c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
  } else c = literal(event.key);
  if (c) { append(c, event); return true; }
  return false;
}

void FilesApp::soft_keys(SoftKeys& keys) const {
  if (page_ == Page::List) {
    keys.label[0] = "Yeni"; keys.label[1] = "Aç"; keys.label[2] = "Sil";
    keys.label[4] = "Yenile"; keys.label[5] = "Ana";
  } else if (page_ == Page::Rename) {
    keys.label[0] = "Tamam"; keys.label[1] = alpha_ ? "Rakam" : "Harf";
    keys.label[5] = "İptal";
  } else {
    keys.label[0] = "Kaydet"; keys.label[1] = alpha_ ? "Rakam" : "Harf";
    keys.label[2] = "Satır"; keys.label[3] = "Ad";
    keys.label[4] = "Temiz"; keys.label[5] = "Liste";
  }
}

}  // namespace calctus::apps
