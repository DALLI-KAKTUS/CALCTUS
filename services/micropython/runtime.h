#pragma once

#include <cstddef>

namespace calctus::python {

struct Result {
  char text[2048]{};
  size_t length = 0;
  bool ok = false;
  bool truncated = false;
};

// Kaynak boyutu en çok 4095 bayt; yürütme başına temiz MicroPython VM.
// Sonuç ve hata mesajları text içinde tutulur. Tek görevden çağrılmalıdır.
void run(const char* source, Result& result);

}  // namespace calctus::python
