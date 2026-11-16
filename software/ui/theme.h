// Renk teması. Bütün arayüz renkleri buradan okunur.
#pragma once

#include "calctus/types.h"

namespace calctus::ui {

struct Theme {
  Color bg;         // içerik zemini
  Color surface;    // kutu ve pencere zemini
  Color text;
  Color muted;      // ikincil yazı
  Color line;       // çizgi ve kenarlık
  Color accent;
  Color on_accent;  // vurgu rengi üstündeki yazı
  Color select;     // seçili satır zemini
  Color disabled;
};

const Theme& theme();
void set_dark_theme(bool dark);
bool dark_theme();

}  // namespace calctus::ui
