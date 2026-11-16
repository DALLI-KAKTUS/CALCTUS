// Liste: tuşla gezilen, kaydırılabilen satırlar.
// Satırları sahibi bir ListModel üzerinden verir; liste veri kopyalamaz.
// Yalnız seçim ve kaydırma durumunu tutar; çizimi LVGL görünümü yapar.
#pragma once

#include "calctus/keys.h"
#include "calctus/types.h"

namespace calctus::ui {

struct ListItem {
  enum class Mark : uint8_t { None, Chevron, On, Off, Checked };

  const char* label = "";
  const char* value = nullptr;  // sağda, soluk yazı
  Mark mark = Mark::None;
  bool enabled = true;
};

class ListModel {
 public:
  virtual int count() const = 0;
  virtual void item(int index, ListItem& out) const = 0;

 protected:
  ~ListModel() = default;
};

class ListView {
 public:
  static constexpr int kRowH = 25;

  enum class Result : uint8_t { Ignored, Moved, Activated };

  // on_surface: liste bir pencerenin içindeyse zemini surface rengidir.
  void setup(const ListModel* model, Rect bounds, bool on_surface = false);
  Rect bounds() const { return bounds_; }

  const ListModel* model() const { return model_; }
  bool on_surface() const { return on_surface_; }
  int selected() const { return sel_; }
  int first_visible() const { return top_; }
  int visible_rows() const { return bounds_.h / kRowH; }
  void select(int index);

  // Yukarı/aşağı seçimi taşır; OK ya da EXE etkinleştirir.
  Result on_key(const KeyEvent& ev);
  // Son tuşun değiştirdiği bölge (yeniden çizilmesi gereken yer).
  Rect damage() const { return damage_; }
  Rect row_rect(int index) const;

 private:
  void scroll_to_selection();

  const ListModel* model_ = nullptr;
  Rect bounds_{};
  Rect damage_{};
  int sel_ = 0;
  int top_ = 0;
  bool on_surface_ = false;
};

}  // namespace calctus::ui
