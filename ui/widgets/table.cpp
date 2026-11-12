#include "ui/widgets/table.h"

#include "ui/theme.h"

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

void TableView::draw(Canvas& c) const {
  const Theme& t = theme();
  c.fill_rect(bounds_, t.bg);
  if (model_ == nullptr) return;
  const int cols = model_->cols();
  const int rows = model_->rows();

  // Başlık satırı
  c.fill_rect(make_rect(bounds_.x, bounds_.y, bounds_.w, kRowH), t.select);
  for (int col = 0; col < cols; ++col) {
    Rect r = cell_rect(top_, col);
    r.y = bounds_.y;
    c.text_in(r, model_->header(col), font_sans12, t.text);
  }

  char buf[32];
  for (int row = top_; row < rows && row < top_ + visible_rows(); ++row) {
    c.fill_rect(make_rect(bounds_.x + 6, cell_rect(row, 0).bottom() - 1, bounds_.w - 12, 1),
                t.line);
    for (int col = 0; col < cols; ++col) {
      model_->cell(row, col, buf, sizeof buf);
      c.text_in(cell_rect(row, col), buf, font_sans12, t.text);
    }
  }

  // Seçili hücre en son çizilir ki kutusu çizgilerin üstünde kalsın.
  if (row_ >= top_ && row_ < top_ + visible_rows()) {
    const Rect r = cell_rect(row_, col_);
    model_->cell(row_, col_, buf, sizeof buf);
    c.box(make_rect(r.x, r.y - 1, r.w, r.h + 1), 3, t.accent, t.surface);
    c.text_in(r, buf, font_sans12, t.text);
  }
}

}  // namespace calctus::ui
