#pragma once

#include "math/eval.h"

namespace calctus::math {

// Tek değişkenli, frekansı 1 olan örneklerin tanımlayıcı istatistikleri.
// Bellek sahipliği çağırandadır; en çok 32 değer, sayı metinleri <=127 bayt.
struct Statistics {
  char sum[128]{};
  char mean[128]{};
  char variance[128]{};  // popülasyon varyansı: n böleni
  char deviation[128]{};
  char minimum[128]{};
  char maximum[128]{};
};

Error summarize(const char* const* values, int count, Statistics& result);

}  // namespace calctus::math
