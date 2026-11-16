#include "ui/widgets/modal.h"

namespace calctus::ui {

namespace {

constexpr int kScreenW = 320;
constexpr int kScreenH = 240;

Rect centered(int w, int h) { return make_rect((kScreenW - w) / 2, (kScreenH - h) / 2, w, h); }

}  // namespace

// --- Dialog --------------------------------------------------------------------

void Dialog::setup(const char* title, const char* message, const char* button0,
                   const char* button1, Callback done, void* ctx) {
  title_ = title;
  message_ = message;
  buttons_[0] = button0;
  buttons_[1] = button1;
  sel_ = 1;
  done_ = done;
  ctx_ = ctx;
}

Rect Dialog::bounds() const { return centered(264, 118); }

Modal::Result Dialog::on_key(const KeyEvent& ev) {
  if (!ev.down) return Result::Stay;
  switch (ev.key) {
    case Key::Left:
    case Key::Right: {
      const int next = ev.key == Key::Left ? 0 : 1;
      if (next == sel_) return Result::Stay;
      sel_ = next;
      return Result::Redraw;
    }
    case Key::Ok:
    case Key::Exe:
      if (ev.repeat) return Result::Stay;
      if (done_ != nullptr) done_(ctx_, sel_);
      return Result::Close;
    case Key::Back:
      if (done_ != nullptr) done_(ctx_, -1);
      return Result::Close;
    default:
      return Result::Stay;
  }
}

// --- Picker --------------------------------------------------------------------

void Picker::setup(const char* title, const char* const* options, int count, int current,
                   Callback done, void* ctx) {
  title_ = title;
  options_ = options;
  count_ = count;
  current_ = current;
  done_ = done;
  ctx_ = ctx;

  const int rows = count < 6 ? count : 6;
  bounds_ = centered(210, 30 + rows * ListView::kRowH + 8);
  list_.setup(this, make_rect(bounds_.x + 4, bounds_.y + 30, bounds_.w - 8, rows * ListView::kRowH),
              true);
  list_.select(current);
}

void Picker::item(int index, ListItem& out) const {
  out.label = options_[index];
  out.mark = index == current_ ? ListItem::Mark::Checked : ListItem::Mark::None;
}

Modal::Result Picker::on_key(const KeyEvent& ev) {
  if (!ev.down) return Result::Stay;
  if (ev.key == Key::Back) return Result::Close;
  switch (list_.on_key(ev)) {
    case ListView::Result::Moved:
      return Result::Redraw;
    case ListView::Result::Activated:
      if (done_ != nullptr) done_(ctx_, list_.selected());
      return Result::Close;
    case ListView::Result::Ignored:
      break;
  }
  return Result::Stay;
}

// --- SliderModal ---------------------------------------------------------------

void SliderModal::setup(const char* title, int value, int min, int max, int step,
                        const char* unit, Callback changed, void* ctx) {
  title_ = title;
  value_ = value;
  min_ = min;
  max_ = max;
  step_ = step;
  unit_ = unit;
  changed_ = changed;
  ctx_ = ctx;
}

Rect SliderModal::bounds() const { return centered(244, 78); }

Modal::Result SliderModal::on_key(const KeyEvent& ev) {
  if (!ev.down) return Result::Stay;
  switch (ev.key) {
    case Key::Left:
    case Key::Right: {
      int next = value_ + (ev.key == Key::Right ? step_ : -step_);
      if (next < min_) next = min_;
      if (next > max_) next = max_;
      if (next == value_) return Result::Stay;
      value_ = next;
      if (changed_ != nullptr) changed_(ctx_, value_);
      return Result::Redraw;
    }
    case Key::Ok:
    case Key::Exe:
      return ev.repeat ? Result::Stay : Result::Close;
    case Key::Back:
      return Result::Close;
    default:
      return Result::Stay;
  }
}

}  // namespace calctus::ui
