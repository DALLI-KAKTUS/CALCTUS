#include "ui/widgets/table.h"

namespace calctus::ui {

void TableView::setup(const TableModel* model, Rect bounds) {
  model_ = model;
  bounds_ = bounds;
  row_ = col_ = top_ = 0;
}

Rect TableView::cell_rect(int row, int col) const {
  const int cols = model_->cols();
  const int x0 = bounds_.x + bounds_.w * col / cols;
  const int x1 = bounds_.x + bounds_.w * (col + 1) / cols;
  return make_rect(x0, bounds_.y + (row - top_ + 1) * kRowH, x1 - x0, kRowH);
}

bool TableView::on_key(const KeyEvent& ev) {
  damage_ = Rect{};
  if (!ev.down || model_ == nullptr) return false;
  int r = row_, c = col_;
  switch (ev.key) {
    case Key::Up:
      --r;
      break;
    case Key::Down:
      ++r;
      break;
    case Key::Left:
      --c;
      break;
    case Key::Right:
      ++c;
      break;
    default:
      return false;
  }
  if (r < 0 || r >= model_->rows() || c < 0 || c >= model_->cols()) return true;

  // Seçim kutusu hücrenin bir piksel üstüne taşar; hasar bölgesi onu da kapsar.
  auto boxed = [](Rect cell) { return make_rect(cell.x, cell.y - 1, cell.w, cell.h + 1); };
  const Rect old = boxed(cell_rect(row_, col_));
  const int old_top = top_;
  row_ = r;
  col_ = c;
  if (row_ < top_) top_ = row_;
  if (row_ >= top_ + visible_rows()) top_ = row_ - visible_rows() + 1;
  damage_ = top_ != old_top ? bounds_ : unite(old, boxed(cell_rect(row_, col_)));
  return true;
}

}  // namespace calctus::ui
