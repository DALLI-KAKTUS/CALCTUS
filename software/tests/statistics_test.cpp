#include <cstdio>
#include <cstring>

#include "math/statistics.h"

int main() {
  using calctus::math::Error;
  using calctus::math::Statistics;
  using calctus::math::summarize;
  Statistics result{};
  const char* numbers[] = {"2", "4", "6"};
  if (summarize(numbers, 3, result) != Error::None ||
      std::strcmp(result.sum, "12") || std::strcmp(result.mean, "4") ||
      std::strcmp(result.minimum, "2") || std::strcmp(result.maximum, "6")) return 1;
  // Yuvarlama açısından güvenilir, tam varyanslı küme.
  const char* symmetric[] = {"1", "3"};
  if (summarize(symmetric, 2, result) != Error::None ||
      std::strcmp(result.variance, "1") || std::strcmp(result.deviation, "1")) return 2;
  const char* decimals[] = {"0.1", "0.2", "0.3"};
  if (summarize(decimals, 3, result) != Error::None ||
      std::strcmp(result.sum, "0.6") || std::strcmp(result.mean, "0.2")) return 3;
  if (summarize(decimals, 0, result) != Error::Syntax) return 4;
  std::puts("istatistik: toplam, ortalama, varyans, min/max geçti");
  return 0;
}
