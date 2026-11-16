#pragma once

#include <cstddef>

#include "kernel/settings.h"

namespace calctus::math {

// Başarılıysa out'a null sonlu sonuç yazılır. Hata durumunda out boş kalır.
enum class Error { None, Syntax, Variable, DivideZero, Domain, TooLarge, Memory };
Error evaluate(const char* expression, const char* ans, NumberFormat format,
               char* out, size_t capacity, AngleUnit angle = AngleUnit::Degree,
               const char* x = nullptr);
const char* error_text(Error error);

}  // namespace calctus::math
