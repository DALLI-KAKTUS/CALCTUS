#include "ui/theme.h"

namespace calctus::ui {

namespace {

constexpr Theme kLight = {
    rgb(246, 249, 247), rgb(255, 255, 255), rgb(29, 41, 34),
    rgb(95, 113, 102), rgb(214, 225, 216), rgb(47, 143, 91),
    rgb(255, 255, 255), rgb(217, 237, 224), rgb(155, 168, 159),
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
