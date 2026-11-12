#include "ui/widgets/list.h"

#include "ui/theme.h"

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

void ListView::draw(Canvas& c) const {
  const Theme& t = theme();
  const Color bg = on_surface_ ? t.surface : t.bg;
  c.fill_rect(bounds_, bg);
  if (model_ == nullptr) return;

  const int n = model_->count();
  const int rows = visible_rows();
  const bool scrollbar = n > rows;

  for (int i = top_; i < n && i < top_ + rows; ++i) {
    ListItem it;
    model_->item(i, it);
    const Rect r = row_rect(i);
    const bool selected = i == sel_;
    if (selected) {
      c.fill_rect(r, t.select);
      c.fill_rect(make_rect(r.x, r.y, 3, r.h), t.accent);
    }
    const Color fg = it.enabled ? t.text : t.disabled;
    int right = r.right() - 10 - (scrollbar ? 5 : 0);

    switch (it.mark) {
      case ListItem::Mark::Chevron:
        right -= 8;
        c.text_in(make_rect(right, r.y, 8, r.h), "›", font_sans12, t.muted);
        right -= 6;
        break;
      case ListItem::Mark::Checked:
        right -= 12;
        c.text_in(make_rect(right, r.y, 12, r.h), "✓", font_sans12, t.accent);
        right -= 6;
        break;
      case ListItem::Mark::On:
      case ListItem::Mark::Off: {
        const bool on = it.mark == ListItem::Mark::On;
        const Rect pill = make_rect(right - 26, r.y + (r.h - 14) / 2, 26, 14);
        c.fill_round_rect(pill, 7, on ? t.accent : t.line);
        c.fill_round_rect(make_rect(on ? pill.right() - 12 : pill.x + 2, pill.y + 2, 10, 10), 5,
                          t.surface);
        right -= 32;
        break;
      }
      case ListItem::Mark::None:
        break;
    }

    if (it.value != nullptr) {
      c.text_in(make_rect(r.x + 10, r.y, right - (r.x + 10), r.h), it.value, font_sans12, t.muted,
                Align::Right);
    }
    c.text_in(make_rect(r.x + 10, r.y, r.w - 20, r.h), it.label, font_sans12, fg, Align::Left);
  }

  if (scrollbar) {
    const int track_h = rows * kRowH;
    int thumb_h = track_h * rows / n;
    if (thumb_h < 8) thumb_h = 8;
    const int thumb_y = bounds_.y + (track_h - thumb_h) * top_ / (n - rows);
    c.fill_round_rect(make_rect(bounds_.right() - 5, thumb_y, 3, thumb_h), 1, t.muted);
  }
}

}  // namespace calctus::ui
