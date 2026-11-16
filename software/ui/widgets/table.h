// Tablo: başlık satırı ve tuşla gezilen hücreler. Yalnız durum tutar;
// çizimi LVGL görünümü yapar.
#pragma once

#include "calctus/keys.h"
#include "calctus/types.h"

namespace calctus::ui {

class TableModel {
 public:
  virtual int rows() const = 0;
  virtual int cols() const = 0;
  virtual const char* header(int col) const = 0;
  // Hücre metnini buf içine yazar.
  virtual void cell(int row, int col, char* buf, int size) const = 0;

 protected:
  ~TableModel() = default;
};

class TableView {
 public:
  static constexpr int kRowH = 22;

  void setup(const TableModel* model, Rect bounds);

  const TableModel* model() const { return model_; }
  Rect bounds() const { return bounds_; }
  int row() const { return row_; }
  int col() const { return col_; }
  int first_visible() const { return top_; }
  int visible_rows() const { return bounds_.h / kRowH - 1; }
  Rect cell_rect(int row, int col) const;

  // Ok tuşları seçili hücreyi taşır. Tuş işlendiyse true.
  bool on_key(const KeyEvent& ev);
  Rect damage() const { return damage_; }

 private:

  const TableModel* model_ = nullptr;
  Rect bounds_{};
  Rect damage_{};
  int row_ = 0, col_ = 0, top_ = 0;
};

}  // namespace calctus::ui
