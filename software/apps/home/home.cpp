#include "apps/home/home.h"

#include <cstdio>

#include "kernel/system.h"

namespace calctus::apps {

Rect HomeApp::tile_rect(int index) const {
  const Rect area = System::kContent;
  const int row = index / kCols - top_row_;
  const int col = index % kCols;
  return make_rect(area.x + col * kTileW, area.y + 1 + row * kTileH, kTileW, kTileH);
}

void HomeApp::open_selected() {
  System& sys = System::get();
  const AppEntry& entry = sys.apps()[sel_];
  if (entry.app != nullptr) {
    sys.open(*entry.app);
  } else {
    char buf[64];
    std::snprintf(buf, sizeof buf, "%s: yakında", entry.name);
    sys.toast(buf);
  }
}

bool HomeApp::on_key(const KeyEvent& ev) {
  if (!ev.down) return false;
  System& sys = System::get();
  const int n = sys.app_count();

  int next = sel_;
  switch (ev.key) {
    case Key::Left:
      if (sel_ % kCols > 0) next = sel_ - 1;
      break;
    case Key::Right:
      if (sel_ % kCols < kCols - 1 && sel_ + 1 < n) next = sel_ + 1;
      break;
    case Key::Up:
      if (sel_ >= kCols) next = sel_ - kCols;
      break;
    case Key::Down:
      if (sel_ + kCols < n) {
        next = sel_ + kCols;
      } else if (sel_ / kCols < (n - 1) / kCols) {
        next = n - 1;  // son satır eksikse son uygulamaya in
      }
      break;
    case Key::Ok:
    case Key::Exe:
    case Key::F1:
      if (!ev.repeat) open_selected();
      return true;
    default:
      return false;
  }

  if (next != sel_) {
    const Rect old = tile_rect(sel_);
    const int old_top = top_row_;
    sel_ = next;
    const int row = sel_ / kCols;
    if (row < top_row_) top_row_ = row;
    if (row >= top_row_ + kRows) top_row_ = row - kRows + 1;
    if (top_row_ != old_top) {
      sys.invalidate_content();
    } else {
      sys.invalidate(old);
      sys.invalidate(tile_rect(sel_));
    }
  }
  return true;
}

void HomeApp::soft_keys(SoftKeys& keys) const { keys.label[0] = "Aç"; }

}  // namespace calctus::apps
