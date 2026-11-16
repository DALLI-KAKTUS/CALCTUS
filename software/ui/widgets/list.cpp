#include "ui/widgets/list.h"

namespace calctus::ui {

void ListView::setup(const ListModel* model, Rect bounds, bool on_surface) {
  model_ = model;
  bounds_ = bounds;
  on_surface_ = on_surface;
  select(sel_);
}

void ListView::select(int index) {
  const int n = model_ != nullptr ? model_->count() : 0;
  if (index >= n) index = n - 1;
  if (index < 0) index = 0;
  sel_ = index;
  scroll_to_selection();
}

void ListView::scroll_to_selection() {
  const int rows = visible_rows();
  const int n = model_ != nullptr ? model_->count() : 0;
  if (sel_ < top_) top_ = sel_;
  if (sel_ >= top_ + rows) top_ = sel_ - rows + 1;
  if (top_ > n - rows) top_ = n - rows;
  if (top_ < 0) top_ = 0;
}

Rect ListView::row_rect(int index) const {
  return make_rect(bounds_.x, bounds_.y + (index - top_) * kRowH, bounds_.w, kRowH);
}

ListView::Result ListView::on_key(const KeyEvent& ev) {
  damage_ = Rect{};
  if (!ev.down || model_ == nullptr) return Result::Ignored;
  const int n = model_->count();

  if (ev.key == Key::Up || ev.key == Key::Down) {
    const int next = sel_ + (ev.key == Key::Down ? 1 : -1);
    if (next < 0 || next >= n) return Result::Ignored;
    const int old = sel_;
    const int old_top = top_;
    sel_ = next;
    scroll_to_selection();
    damage_ = top_ != old_top ? bounds_ : unite(row_rect(old), row_rect(sel_));
    return Result::Moved;
  }
  if ((ev.key == Key::Ok || ev.key == Key::Exe) && !ev.repeat && n > 0) {
    return Result::Activated;
  }
  return Result::Ignored;
}

}  // namespace calctus::ui
