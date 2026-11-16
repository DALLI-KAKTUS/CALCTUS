#pragma once

#include <cstddef>
#include <cstdint>

#include "math/eval.h"

namespace calctus::math {

enum class Combinatoric : uint8_t { Factorial, Permutation, Combination };

// n <= 60: tüm sonuçlar, 60! dahil, ondalık olarak tam saklanır.
// k faktöriyelde kullanılmaz. Başarıda out null sonlu tam sayıdır.
Error combinatoric(Combinatoric operation, unsigned n, unsigned k,
                   char* out, size_t capacity);

}  // namespace calctus::math
