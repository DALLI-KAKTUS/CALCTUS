#pragma once

#include "math/eval.h"

namespace calctus::math {

enum class Roots { None, Any, Linear, Double, Two, Complex };

struct Solution {
  Roots kind = Roots::None;
  char first[128]{};   // reel kök veya karmaşık kökün reel parçası
  char second[128]{};  // ikinci reel kök veya pozitif sanal parça
};

// ax² + bx + c = 0. Girdiler ondalık sayı metinleri, çıktı yalnız başarıda değişir.
Error solve_quadratic(const char* a, const char* b, const char* c, Solution& out);

}  // namespace calctus::math
