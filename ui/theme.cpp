#include "ui/theme.h"

namespace calctus::ui {

namespace {

constexpr Theme kLight = {
    rgb(244, 241, 234), rgb(255, 255, 255), rgb(30, 36, 32),
    rgb(112, 120, 114), rgb(201, 196, 184), rgb(47, 143, 91),
    rgb(255, 255, 255), rgb(212, 234, 221), rgb(170, 174, 168),
};

constexpr Theme kDark = {
    rgb(18, 22, 20),    rgb(28, 34, 31), rgb(230, 237, 232),
    rgb(140, 150, 144), rgb(52, 62, 56), rgb(76, 195, 138),
    rgb(10, 20, 14),    rgb(36, 72, 54), rgb(84, 92, 87),
};

bool g_dark = false;

}  // namespace

const Theme& theme() { return g_dark ? kDark : kLight; }

void set_dark_theme(bool dark) { g_dark = dark; }

bool dark_theme() { return g_dark; }

}  // namespace calctus::ui
